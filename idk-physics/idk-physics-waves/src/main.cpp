#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

bool g_telex_mode = false;
String g_status = "Ready";


constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

int g_menu_idx = 0;
int g_menu_scroll = 0;

void actionWaveSpeed() {
  double f = 0, lambda = 0;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  if (!inputNumber(lambda, "lambda (m)", g_telex_mode)) return;
  double v = f * lambda;
  showResult("Wave", "v = " + formatFloat(v, 6) + " m/s");
}

void actionPeriod() {
  double f = 0;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  if (fabs(f) < 1e-9) { showResult("Period", "f cannot be 0"); return; }
  double T = 1.0 / f;
  showResult("Period", "T = " + formatFloat(T, 6) + " s");
}

void actionOmega() {
  double f = 0;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  double w = 2.0 * 3.141592653589793 * f;
  showResult("Omega", "w = " + formatFloat(w, 6) + " rad/s");
}

void actionPhotonE() {
  double f = 0;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  const double h = 6.626e-34;
  double e = h * f;
  showResult("Photon", "E = " + formatFloat(e, 12) + " J");
}

void actionPhotonLambda() {
  double f = 0;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  if (fabs(f) < 1e-9) { showResult("Photon", "f cannot be 0"); return; }
  const double c = 3.0e8;
  double lambda = c / f;
  showResult("Photon", "lambda = " + formatFloat(lambda, 6) + " m");
}

MenuItem kMenu[] = {
    {"v = f*lambda", actionWaveSpeed},
    {"T = 1/f", actionPeriod},
    {"omega = 2pi f", actionOmega},
    {"Photon E=hf", actionPhotonE},
    {"Photon lambda=c/f", actionPhotonLambda},
};
constexpr int kMenuCount = sizeof(kMenu) / sizeof(kMenu[0]);

void ensureScroll() {
  if (g_menu_idx < g_menu_scroll) g_menu_scroll = g_menu_idx;
  if (g_menu_idx >= g_menu_scroll + kMenuLines) g_menu_scroll = g_menu_idx - kMenuLines + 1;
}

void drawMenu() {
  ensureScroll();
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-physics-waves");

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 14);
  M5.Display.print(g_status);

  int y = 28;
  for (int i = 0; i < kMenuLines; ++i) {
    int idx = g_menu_scroll + i;
    if (idx >= kMenuCount) break;
    bool sel = (idx == g_menu_idx);
    M5.Display.setTextColor(sel ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(4, y);
    M5.Display.print(sel ? "> " : "  ");
    M5.Display.print(kMenu[idx].title);
    y += 16;
  }

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 124);
  M5.Display.print("A:Select  B:Next  PWR:Prev");
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  autoConnectKnownWifi(kKnownWifis, sizeof(kKnownWifis) / sizeof(kKnownWifis[0]), g_status);
  drawMenu();
}

void loop() {
  M5.update();
  if (M5.BtnB.wasPressed()) {
    g_menu_idx = (g_menu_idx + 1) % kMenuCount;
    drawMenu();
  }
  if (M5.BtnPWR.wasPressed()) {
    g_menu_idx = (g_menu_idx - 1 + kMenuCount) % kMenuCount;
    drawMenu();
  }
  if (M5.BtnA.wasPressed()) {
    kMenu[g_menu_idx].action();
    drawMenu();
  }
  delay(10);
}

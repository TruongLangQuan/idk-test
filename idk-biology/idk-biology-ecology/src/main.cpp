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

void actionExponential() {
  double N0 = 0, r = 0, t = 0;
  if (!inputNumber(N0, "N0", g_telex_mode)) return;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  if (!inputNumber(t, "t", g_telex_mode)) return;
  double Nt = N0 * exp(r * t);
  showResult("Exponential", "Nt = " + formatFloat(Nt, 6));
}

void actionLogistic() {
  double N0 = 0, r = 0, t = 0, K = 0;
  if (!inputNumber(N0, "N0", g_telex_mode)) return;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  if (!inputNumber(t, "t", g_telex_mode)) return;
  if (!inputNumber(K, "K", g_telex_mode)) return;
  if (K <= 0) {
    showResult("Logistic", "K invalid");
    return;
  }
  double Nt = K / (1.0 + ((K - N0) / N0) * exp(-r * t));
  showResult("Logistic", "Nt = " + formatFloat(Nt, 6));
}

void actionDensity() {
  double N = 0, A = 0;
  if (!inputNumber(N, "N", g_telex_mode)) return;
  if (!inputNumber(A, "Area", g_telex_mode)) return;
  if (fabs(A) < 1e-12) {
    showResult("Density", "Area invalid");
    return;
  }
  double D = N / A;
  showResult("Density", "D = " + formatFloat(D, 6));
}

void actionEnergyTransfer() {
  double E0 = 0;
  if (!inputNumber(E0, "E0", g_telex_mode)) return;
  double levels = 0;
  if (!inputNumber(levels, "levels", g_telex_mode)) return;
  int n = static_cast<int>(round(levels));
  double E = E0;
  for (int i = 0; i < n; ++i) E *= 0.1;
  showResult("Energy", "E = " + formatFloat(E, 6));
}

MenuItem kMenu[] = {
    {"Exponential growth", actionExponential},
    {"Logistic growth", actionLogistic},
    {"Population density", actionDensity},
    {"Energy 10% rule", actionEnergyTransfer},
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
  M5.Display.print("idk-biology-ecology");

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

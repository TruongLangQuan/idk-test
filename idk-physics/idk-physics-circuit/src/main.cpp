#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;
constexpr double kPi = 3.141592653589793;

struct MenuItem { const char* title; void (*action)(); };

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

void actionResonance() {
  double L = 0, C = 0;
  if (!inputNumber(L, "L (H)", g_telex_mode)) return;
  if (!inputNumber(C, "C (F)", g_telex_mode)) return;
  if (L <= 0 || C <= 0) {
    showResult("Resonance", "L,C >0");
    return;
  }
  double f0 = 1.0 / (2.0 * kPi * sqrt(L * C));
  showResult("Resonance", "f0 = " + formatFloat(f0, 6) + " Hz");
}

void actionReactance() {
  double f = 0, L = 0, C = 0;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  if (!inputNumber(L, "L (H)", g_telex_mode)) return;
  if (!inputNumber(C, "C (F)", g_telex_mode)) return;
  double xl = 2.0 * kPi * f * L;
  double xc = (C > 0) ? 1.0 / (2.0 * kPi * f * C) : 0.0;
  String out = "XL = " + formatFloat(xl, 6) + "\n";
  out += "XC = " + formatFloat(xc, 6);
  showResult("Reactance", out);
}

void actionImpedance() {
  double R = 0, f = 0, L = 0, C = 0;
  if (!inputNumber(R, "R", g_telex_mode)) return;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  if (!inputNumber(L, "L (H)", g_telex_mode)) return;
  if (!inputNumber(C, "C (F)", g_telex_mode)) return;
  double xl = 2.0 * kPi * f * L;
  double xc = (C > 0) ? 1.0 / (2.0 * kPi * f * C) : 0.0;
  double z = sqrt(R * R + (xl - xc) * (xl - xc));
  showResult("Impedance", "|Z| = " + formatFloat(z, 6));
}

MenuItem kMenu[] = {
    {"Resonance f0", actionResonance},
    {"Reactance XL/XC", actionReactance},
    {"Series impedance", actionImpedance},
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
  M5.Display.print("idk-physics-circuit");

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

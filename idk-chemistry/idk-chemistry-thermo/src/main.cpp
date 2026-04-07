#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

bool g_telex_mode = false;
String g_status = "Ready";

MemoryFontWrapper g_font_wrap(vi12_font, vi12_font_len);

constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

int g_menu_idx = 0;
int g_menu_scroll = 0;

void actionGibbs() {
  double h = 0, s = 0, t = 0;
  if (!inputNumber(h, "DeltaH (kJ)", g_telex_mode)) return;
  if (!inputNumber(s, "DeltaS (kJ/K)", g_telex_mode)) return;
  if (!inputNumber(t, "T (K)", g_telex_mode)) return;
  double g = h - t * s;
  showResult("Gibbs", "DeltaG = " + formatFloat(g, 6) + " kJ");
}

void actionEquilibriumK() {
  double g = 0, t = 0;
  if (!inputNumber(g, "DeltaG (kJ)", g_telex_mode)) return;
  if (!inputNumber(t, "T (K)", g_telex_mode)) return;
  if (t <= 0) {
    showResult("K", "T > 0");
    return;
  }
  const double R = 8.314462618;
  double k = exp(-(g * 1000.0) / (R * t));
  showResult("K", "K = " + formatFloat(k, 6));
}

void actionEquilibriumT() {
  double h = 0, s = 0;
  if (!inputNumber(h, "DeltaH (kJ)", g_telex_mode)) return;
  if (!inputNumber(s, "DeltaS (kJ/K)", g_telex_mode)) return;
  if (s == 0) {
    showResult("Teq", "DeltaS != 0");
    return;
  }
  double t = h / s;
  showResult("Teq", "T(eq) = " + formatFloat(t, 6) + " K");
}

void actionHeat() {
  double m = 0, c = 0, dT = 0;
  if (!inputNumber(m, "m (g)", g_telex_mode)) return;
  if (!inputNumber(c, "c (J/gK)", g_telex_mode)) return;
  if (!inputNumber(dT, "DeltaT (K)", g_telex_mode)) return;
  double q = m * c * dT;
  showResult("Heat", "q = " + formatFloat(q, 6) + " J");
}

void actionEntropy() {
  double qrev = 0, t = 0;
  if (!inputNumber(qrev, "qrev (kJ)", g_telex_mode)) return;
  if (!inputNumber(t, "T (K)", g_telex_mode)) return;
  if (t <= 0) {
    showResult("Entropy", "T > 0");
    return;
  }
  double s = (qrev * 1000.0) / t;
  showResult("Entropy", "DeltaS = " + formatFloat(s, 6) + " J/K");
}

MenuItem kMenu[] = {
    {"DeltaG = H - T*S", actionGibbs},
    {"K from DeltaG", actionEquilibriumK},
    {"T(eq) = H/S", actionEquilibriumT},
    {"Heat q = m c dT", actionHeat},
    {"Entropy qrev/T", actionEntropy},
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
  M5.Display.print("idk-chemistry-thermo");

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

  (void)loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);
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

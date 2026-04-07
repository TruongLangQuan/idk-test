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

void actionMoles() {
  double m = 0, mm = 0;
  if (!inputNumber(m, "Mass (g)", g_telex_mode)) return;
  if (!inputNumber(mm, "Molar mass", g_telex_mode)) return;
  if (mm <= 0) {
    showResult("Moles", "Molar mass > 0");
    return;
  }
  double n = m / mm;
  showResult("Moles", "n = " + formatFloat(n, 6) + " mol");
}

void actionMass() {
  double n = 0, mm = 0;
  if (!inputNumber(n, "n (mol)", g_telex_mode)) return;
  if (!inputNumber(mm, "Molar mass", g_telex_mode)) return;
  double m = n * mm;
  showResult("Mass", "m = " + formatFloat(m, 6) + " g");
}

void actionMolarity() {
  double n = 0, v = 0;
  if (!inputNumber(n, "n (mol)", g_telex_mode)) return;
  if (!inputNumber(v, "V (L)", g_telex_mode)) return;
  if (v <= 0) {
    showResult("Molarity", "V > 0");
    return;
  }
  double c = n / v;
  showResult("Molarity", "C = " + formatFloat(c, 6) + " M");
}

void actionDilution() {
  double c1 = 0, v1 = 0, c2 = 0;
  if (!inputNumber(c1, "C1", g_telex_mode)) return;
  if (!inputNumber(v1, "V1", g_telex_mode)) return;
  if (!inputNumber(c2, "C2", g_telex_mode)) return;
  if (c2 == 0) {
    showResult("Dilution", "C2 != 0");
    return;
  }
  double v2 = c1 * v1 / c2;
  showResult("Dilution", "V2 = " + formatFloat(v2, 6));
}

void actionGasLaw() {
  double p = 0, v = 0, t = 0;
  if (!inputNumber(p, "P (atm)", g_telex_mode)) return;
  if (!inputNumber(v, "V (L)", g_telex_mode)) return;
  if (!inputNumber(t, "T (K)", g_telex_mode)) return;
  if (t <= 0) {
    showResult("Gas", "T > 0");
    return;
  }
  const double R = 0.082057366;
  double n = p * v / (R * t);
  showResult("Gas", "n = " + formatFloat(n, 6) + " mol");
}

MenuItem kMenu[] = {
    {"Moles (m/mm)", actionMoles},
    {"Mass (n*mm)", actionMass},
    {"Molarity n/V", actionMolarity},
    {"Dilution C1V1=C2V2", actionDilution},
    {"Gas law PV=nRT", actionGasLaw},
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
  M5.Display.print("idk-lab-tools");

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

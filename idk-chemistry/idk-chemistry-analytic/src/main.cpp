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

void actionPH() {
  double h = 0;
  if (!inputNumber(h, "[H+] (M)", g_telex_mode)) return;
  if (h <= 0) {
    showResult("pH", "[H+] > 0");
    return;
  }
  double ph = -log10(h);
  showResult("pH", "pH = " + formatFloat(ph, 4));
}

void actionPOH() {
  double oh = 0;
  if (!inputNumber(oh, "[OH-] (M)", g_telex_mode)) return;
  if (oh <= 0) {
    showResult("pOH", "[OH-] > 0");
    return;
  }
  double poh = -log10(oh);
  showResult("pOH", "pOH = " + formatFloat(poh, 4));
}

void actionDilution() {
  double c1 = 0, v1 = 0, v2 = 0;
  if (!inputNumber(c1, "C1 (M)", g_telex_mode)) return;
  if (!inputNumber(v1, "V1 (L)", g_telex_mode)) return;
  if (!inputNumber(v2, "V2 (L)", g_telex_mode)) return;
  if (v2 <= 0) {
    showResult("Dilution", "V2 > 0");
    return;
  }
  double c2 = c1 * v1 / v2;
  showResult("Dilution", "C2 = " + formatFloat(c2, 6) + " M");
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

void actionYield() {
  double actual = 0, theoretical = 0;
  if (!inputNumber(actual, "Actual (g)", g_telex_mode)) return;
  if (!inputNumber(theoretical, "Theo (g)", g_telex_mode)) return;
  if (theoretical <= 0) {
    showResult("Yield", "Theo > 0");
    return;
  }
  double pct = actual / theoretical * 100.0;
  showResult("Yield", "Yield = " + formatFloat(pct, 2) + "%");
}

MenuItem kMenu[] = {
    {"pH from [H+]", actionPH},
    {"pOH from [OH-]", actionPOH},
    {"Dilution C1V1=C2V2", actionDilution},
    {"Molarity n/V", actionMolarity},
    {"Percent yield", actionYield},
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
  M5.Display.print("idk-chemistry-analytic");

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

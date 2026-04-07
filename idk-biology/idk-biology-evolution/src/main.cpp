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

void showHW(double p, double q) {
  double p2 = p * p;
  double q2 = q * q;
  double pq2 = 2 * p * q;
  String out = "p = " + formatFloat(p, 4) + ", q = " + formatFloat(q, 4) + "\n" +
               "p^2 = " + formatFloat(p2, 4) + "\n" +
               "2pq = " + formatFloat(pq2, 4) + "\n" +
               "q^2 = " + formatFloat(q2, 4);
  showResult("Hardy-Weinberg", out);
}

void actionFromP() {
  double p = 0;
  if (!inputNumber(p, "p (0-1)", g_telex_mode)) return;
  if (p < 0 || p > 1) {
    showResult("HW", "p in [0,1]");
    return;
  }
  double q = 1.0 - p;
  showHW(p, q);
}

void actionFromQ2() {
  double q2 = 0;
  if (!inputNumber(q2, "q^2", g_telex_mode)) return;
  if (q2 < 0 || q2 > 1) {
    showResult("HW", "q^2 in [0,1]");
    return;
  }
  double q = sqrt(q2);
  double p = 1.0 - q;
  showHW(p, q);
}

void actionFromCounts() {
  double aa = 0, Aa = 0, AA = 0;
  if (!inputNumber(AA, "AA count", g_telex_mode)) return;
  if (!inputNumber(Aa, "Aa count", g_telex_mode)) return;
  if (!inputNumber(aa, "aa count", g_telex_mode)) return;
  double n = AA + Aa + aa;
  if (n <= 0) {
    showResult("HW", "Total > 0");
    return;
  }
  double p = (2 * AA + Aa) / (2 * n);
  double q = 1.0 - p;
  showHW(p, q);
}

void actionSelection() {
  double wAA = 0, wAa = 0, waa = 0;
  if (!inputNumber(wAA, "wAA", g_telex_mode)) return;
  if (!inputNumber(wAa, "wAa", g_telex_mode)) return;
  if (!inputNumber(waa, "waa", g_telex_mode)) return;
  String out = "Relative fitness:\n" +
               String("wAA=") + formatFloat(wAA, 3) + "\n" +
               String("wAa=") + formatFloat(wAa, 3) + "\n" +
               String("waa=") + formatFloat(waa, 3);
  showResult("Selection", out);
}

MenuItem kMenu[] = {
    {"HW from p", actionFromP},
    {"HW from q^2", actionFromQ2},
    {"HW from counts", actionFromCounts},
    {"Fitness (w)", actionSelection},
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
  M5.Display.print("idk-biology-evolution");

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

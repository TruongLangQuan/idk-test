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

void actionGrowth() {
  double n0 = 0, gen = 0;
  if (!inputNumber(n0, "N0", g_telex_mode)) return;
  if (!inputNumber(gen, "Generations", g_telex_mode)) return;
  double n = n0 * pow(2.0, gen);
  showResult("Growth", "N = " + formatFloat(n, 3));
}

void actionDoubling() {
  double n0 = 0, n = 0, t = 0;
  if (!inputNumber(n0, "N0", g_telex_mode)) return;
  if (!inputNumber(n, "N", g_telex_mode)) return;
  if (!inputNumber(t, "t", g_telex_mode)) return;
  if (n0 <= 0 || n <= 0 || t <= 0) {
    showResult("Doubling", "Values > 0");
    return;
  }
  double gen = log(n / n0) / log(2.0);
  double td = t / gen;
  showResult("Doubling", "td = " + formatFloat(td, 4));
}

void actionMicroscope() {
  double obj = 0, eye = 0;
  if (!inputNumber(obj, "Objective", g_telex_mode)) return;
  if (!inputNumber(eye, "Ocular", g_telex_mode)) return;
  double mag = obj * eye;
  showResult("Microscope", "Total = " + formatFloat(mag, 2) + "x");
}

void actionDilution() {
  double c1 = 0, df = 0;
  if (!inputNumber(c1, "C1", g_telex_mode)) return;
  if (!inputNumber(df, "Dilution factor", g_telex_mode)) return;
  if (df == 0) {
    showResult("Dilution", "factor != 0");
    return;
  }
  double c2 = c1 / df;
  showResult("Dilution", "C2 = " + formatFloat(c2, 6));
}

void actionCFU() {
  double colonies = 0, dil = 0, vol = 0;
  if (!inputNumber(colonies, "Colonies", g_telex_mode)) return;
  if (!inputNumber(dil, "Dilution", g_telex_mode)) return;
  if (!inputNumber(vol, "Volume (mL)", g_telex_mode)) return;
  if (dil <= 0 || vol <= 0) {
    showResult("CFU", "dil, vol > 0");
    return;
  }
  double cfu = colonies / (dil * vol);
  showResult("CFU", "CFU/mL = " + formatFloat(cfu, 3));
}

MenuItem kMenu[] = {
    {"Growth N0*2^n", actionGrowth},
    {"Doubling time", actionDoubling},
    {"Microscope mag", actionMicroscope},
    {"Dilution C1/f", actionDilution},
    {"CFU count", actionCFU},
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
  M5.Display.print("idk-biology-micro");

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

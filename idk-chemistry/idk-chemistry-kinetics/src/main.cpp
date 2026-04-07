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

void actionFirstOrderK() {
  double c0 = 0, ct = 0, t = 0;
  if (!inputNumber(c0, "C0", g_telex_mode)) return;
  if (!inputNumber(ct, "Ct", g_telex_mode)) return;
  if (!inputNumber(t, "t (s)", g_telex_mode)) return;
  if (c0 <= 0 || ct <= 0 || t <= 0) {
    showResult("1st order", "Values > 0");
    return;
  }
  double k = log(c0 / ct) / t;
  showResult("1st order", "k = " + formatFloat(k, 6) + " 1/s");
}

void actionSecondOrderK() {
  double c0 = 0, ct = 0, t = 0;
  if (!inputNumber(c0, "C0", g_telex_mode)) return;
  if (!inputNumber(ct, "Ct", g_telex_mode)) return;
  if (!inputNumber(t, "t (s)", g_telex_mode)) return;
  if (c0 <= 0 || ct <= 0 || t <= 0) {
    showResult("2nd order", "Values > 0");
    return;
  }
  double k = (1.0 / ct - 1.0 / c0) / t;
  showResult("2nd order", "k = " + formatFloat(k, 6) + " L/mol*s");
}

void actionHalfLife() {
  double k = 0;
  if (!inputNumber(k, "k (1/s)", g_telex_mode)) return;
  if (k <= 0) {
    showResult("Half-life", "k > 0");
    return;
  }
  double t12 = 0.69314718056 / k;
  showResult("Half-life", "t1/2 = " + formatFloat(t12, 6) + " s");
}

void actionArrhenius() {
  double A = 0, Ea = 0, T = 0;
  if (!inputNumber(A, "A", g_telex_mode)) return;
  if (!inputNumber(Ea, "Ea (kJ/mol)", g_telex_mode)) return;
  if (!inputNumber(T, "T (K)", g_telex_mode)) return;
  if (T <= 0) {
    showResult("Arrhenius", "T > 0");
    return;
  }
  const double R = 8.314462618;
  double k = A * exp(-(Ea * 1000.0) / (R * T));
  showResult("Arrhenius", "k = " + formatFloat(k, 6));
}

void actionRate() {
  double dC = 0, dt = 0;
  if (!inputNumber(dC, "Delta C", g_telex_mode)) return;
  if (!inputNumber(dt, "Delta t", g_telex_mode)) return;
  if (dt == 0) {
    showResult("Rate", "dt != 0");
    return;
  }
  double rate = dC / dt;
  showResult("Rate", "rate = " + formatFloat(rate, 6));
}

MenuItem kMenu[] = {
    {"k (1st order)", actionFirstOrderK},
    {"k (2nd order)", actionSecondOrderK},
    {"t1/2 (1st)", actionHalfLife},
    {"Arrhenius", actionArrhenius},
    {"Rate dC/dt", actionRate},
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
  M5.Display.print("idk-chemistry-kinetics");

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

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

void actionOhm() {
  double v = 0, r = 0;
  if (!inputNumber(v, "V (volt)", g_telex_mode)) return;
  if (!inputNumber(r, "R (ohm)", g_telex_mode)) return;
  if (fabs(r) < 1e-9) { showResult("Ohm", "R cannot be 0"); return; }
  double i = v / r;
  double p = v * i;
  String out = "I = " + formatFloat(i, 6) + " A\n";
  out += "P = " + formatFloat(p, 6) + " W";
  showResult("Ohm", out);
}

void actionSeries() {
  double r1 = 0, r2 = 0, r3 = 0;
  if (!inputNumber(r1, "R1", g_telex_mode)) return;
  if (!inputNumber(r2, "R2", g_telex_mode)) return;
  if (!inputNumber(r3, "R3", g_telex_mode)) return;
  double req = r1 + r2 + r3;
  showResult("Series", "Req = " + formatFloat(req, 6) + " ohm");
}

void actionParallel() {
  double r1 = 0, r2 = 0, r3 = 0;
  if (!inputNumber(r1, "R1", g_telex_mode)) return;
  if (!inputNumber(r2, "R2", g_telex_mode)) return;
  if (!inputNumber(r3, "R3", g_telex_mode)) return;
  if (fabs(r1) < 1e-9 || fabs(r2) < 1e-9 || fabs(r3) < 1e-9) {
    showResult("Parallel", "R cannot be 0");
    return;
  }
  double req = 1.0 / (1.0 / r1 + 1.0 / r2 + 1.0 / r3);
  showResult("Parallel", "Req = " + formatFloat(req, 6) + " ohm");
}

void actionCapacitance() {
  double q = 0, v = 0;
  if (!inputNumber(q, "Q (C)", g_telex_mode)) return;
  if (!inputNumber(v, "V (V)", g_telex_mode)) return;
  if (fabs(v) < 1e-9) { showResult("Capacitance", "V cannot be 0"); return; }
  double c = q / v;
  showResult("Capacitance", "C = " + formatFloat(c, 6) + " F");
}

void actionCapEnergy() {
  double c = 0, v = 0;
  if (!inputNumber(c, "C (F)", g_telex_mode)) return;
  if (!inputNumber(v, "V (V)", g_telex_mode)) return;
  double e = 0.5 * c * v * v;
  showResult("Cap Energy", "E = " + formatFloat(e, 6) + " J");
}

void actionRCTau() {
  double r = 0, c = 0;
  if (!inputNumber(r, "R (ohm)", g_telex_mode)) return;
  if (!inputNumber(c, "C (F)", g_telex_mode)) return;
  double tau = r * c;
  showResult("RC", "tau = " + formatFloat(tau, 6) + " s");
}

void actionInductor() {
  double l = 0, i = 0;
  if (!inputNumber(l, "L (H)", g_telex_mode)) return;
  if (!inputNumber(i, "I (A)", g_telex_mode)) return;
  double e = 0.5 * l * i * i;
  showResult("Inductor", "E = " + formatFloat(e, 6) + " J");
}

MenuItem kMenu[] = {
    {"Ohm V=IR", actionOhm},
    {"R series (3)", actionSeries},
    {"R parallel (3)", actionParallel},
    {"Cap C=Q/V", actionCapacitance},
    {"Cap Energy", actionCapEnergy},
    {"RC tau", actionRCTau},
    {"Inductor energy", actionInductor},
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
  M5.Display.print("idk-physics-electric");

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

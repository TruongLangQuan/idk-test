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

void actionSpeed() {
  double s = 0, t = 0;
  if (!inputNumber(s, "s (m)", g_telex_mode)) return;
  if (!inputNumber(t, "t (s)", g_telex_mode)) return;
  if (fabs(t) < 1e-9) { showResult("Speed", "t cannot be 0"); return; }
  double v = s / t;
  showResult("Speed", "v = " + formatFloat(v, 6) + " m/s");
}

void actionAccel() {
  double v0 = 0, v1 = 0, t = 0;
  if (!inputNumber(v0, "v0", g_telex_mode)) return;
  if (!inputNumber(v1, "v", g_telex_mode)) return;
  if (!inputNumber(t, "t", g_telex_mode)) return;
  if (fabs(t) < 1e-9) { showResult("Accel", "t cannot be 0"); return; }
  double a = (v1 - v0) / t;
  showResult("Accel", "a = " + formatFloat(a, 6) + " m/s2");
}

void actionDistance() {
  double v0 = 0, a = 0, t = 0;
  if (!inputNumber(v0, "v0", g_telex_mode)) return;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(t, "t", g_telex_mode)) return;
  double s = v0 * t + 0.5 * a * t * t;
  showResult("Distance", "s = " + formatFloat(s, 6) + " m");
}

void actionForce() {
  double m = 0, a = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  double f = m * a;
  showResult("Force", "F = " + formatFloat(f, 6) + " N");
}

void actionMomentum() {
  double m = 0, v = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(v, "v (m/s)", g_telex_mode)) return;
  double p = m * v;
  showResult("Momentum", "p = " + formatFloat(p, 6) + " kg.m/s");
}

void actionKinetic() {
  double m = 0, v = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(v, "v (m/s)", g_telex_mode)) return;
  double e = 0.5 * m * v * v;
  showResult("Kinetic", "Ek = " + formatFloat(e, 6) + " J");
}

void actionPotential() {
  double m = 0, h = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(h, "h (m)", g_telex_mode)) return;
  double g = 9.8;
  double e = m * g * h;
  showResult("Potential", "Ep = " + formatFloat(e, 6) + " J");
}

void actionWork() {
  double f = 0, s = 0;
  if (!inputNumber(f, "F (N)", g_telex_mode)) return;
  if (!inputNumber(s, "s (m)", g_telex_mode)) return;
  double w = f * s;
  showResult("Work", "W = " + formatFloat(w, 6) + " J");
}

void actionPower() {
  double w = 0, t = 0;
  if (!inputNumber(w, "W (J)", g_telex_mode)) return;
  if (!inputNumber(t, "t (s)", g_telex_mode)) return;
  if (fabs(t) < 1e-9) { showResult("Power", "t cannot be 0"); return; }
  double p = w / t;
  showResult("Power", "P = " + formatFloat(p, 6) + " W");
}

void actionGravity() {
  double m1 = 0, m2 = 0, r = 0;
  if (!inputNumber(m1, "m1", g_telex_mode)) return;
  if (!inputNumber(m2, "m2", g_telex_mode)) return;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  if (fabs(r) < 1e-9) { showResult("Gravity", "r cannot be 0"); return; }
  const double G = 6.674e-11;
  double f = G * m1 * m2 / (r * r);
  showResult("Gravity", "F = " + formatFloat(f, 9) + " N");
}

MenuItem kMenu[] = {
    {"v = s/t", actionSpeed},
    {"a = (v-v0)/t", actionAccel},
    {"s = v0t + 1/2at2", actionDistance},
    {"F = m a", actionForce},
    {"p = m v", actionMomentum},
    {"Ek = 1/2 m v2", actionKinetic},
    {"Ep = m g h", actionPotential},
    {"Work W = F s", actionWork},
    {"Power P = W/t", actionPower},
    {"Gravity", actionGravity},
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
  M5.Display.print("idk-physics-mechanics");

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

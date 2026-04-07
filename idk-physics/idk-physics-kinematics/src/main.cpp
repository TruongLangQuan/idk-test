#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;
constexpr double kPi = 3.141592653589793;
constexpr double kG = 9.81;

struct MenuItem { const char* title; void (*action)(); };

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

double deg2rad(double d) { return d * kPi / 180.0; }

void actionProjectile() {
  double v0 = 0, ang = 0;
  if (!inputNumber(v0, "v0", g_telex_mode)) return;
  if (!inputNumber(ang, "angle", g_telex_mode)) return;
  double rad = deg2rad(ang);
  double t = 2.0 * v0 * sin(rad) / kG;
  double R = v0 * v0 * sin(2.0 * rad) / kG;
  double h = v0 * v0 * sin(rad) * sin(rad) / (2.0 * kG);
  String out = "T = " + formatFloat(t, 6) + "\n";
  out += "Range = " + formatFloat(R, 6) + "\n";
  out += "Hmax = " + formatFloat(h, 6);
  showResult("Projectile", out);
}

void actionCircular() {
  double v = 0, r = 0;
  if (!inputNumber(v, "v", g_telex_mode)) return;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  if (r <= 0) { showResult("Circular", "r>0"); return; }
  double a = v * v / r;
  showResult("Circular", "a = " + formatFloat(a, 6));
}

void actionUniform() {
  double s = 0, t = 0;
  if (!inputNumber(s, "s", g_telex_mode)) return;
  if (!inputNumber(t, "t", g_telex_mode)) return;
  if (t == 0) { showResult("v", "t=0"); return; }
  double v = s / t;
  showResult("Uniform", "v = " + formatFloat(v, 6));
}

MenuItem kMenu[] = {
    {"Projectile motion", actionProjectile},
    {"Circular accel", actionCircular},
    {"Uniform v=s/t", actionUniform},
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
  M5.Display.print("idk-physics-kinematics");

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

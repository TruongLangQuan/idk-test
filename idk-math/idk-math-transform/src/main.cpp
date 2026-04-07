#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;
constexpr double kPi = 3.141592653589793;

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

void actionRotate() {
  double x = 0, y = 0, deg = 0;
  if (!inputNumber(x, "x", g_telex_mode)) return;
  if (!inputNumber(y, "y", g_telex_mode)) return;
  if (!inputNumber(deg, "deg", g_telex_mode)) return;
  double rad = deg * kPi / 180.0;
  double xr = x * cos(rad) - y * sin(rad);
  double yr = x * sin(rad) + y * cos(rad);
  String out = "x'=" + formatFloat(xr, 6) + "\n";
  out += "y'=" + formatFloat(yr, 6);
  showResult("Rotate", out);
}

void actionScale() {
  double x = 0, y = 0, sx = 1, sy = 1;
  if (!inputNumber(x, "x", g_telex_mode)) return;
  if (!inputNumber(y, "y", g_telex_mode)) return;
  if (!inputNumber(sx, "sx", g_telex_mode)) return;
  if (!inputNumber(sy, "sy", g_telex_mode)) return;
  String out = "x'=" + formatFloat(x * sx, 6) + "\n";
  out += "y'=" + formatFloat(y * sy, 6);
  showResult("Scale", out);
}

void actionTranslate() {
  double x = 0, y = 0, dx = 0, dy = 0;
  if (!inputNumber(x, "x", g_telex_mode)) return;
  if (!inputNumber(y, "y", g_telex_mode)) return;
  if (!inputNumber(dx, "dx", g_telex_mode)) return;
  if (!inputNumber(dy, "dy", g_telex_mode)) return;
  String out = "x'=" + formatFloat(x + dx, 6) + "\n";
  out += "y'=" + formatFloat(y + dy, 6);
  showResult("Translate", out);
}

void actionMatrix() {
  double a = 1, b = 0, c = 0, d = 1, x = 0, y = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  if (!inputNumber(c, "c", g_telex_mode)) return;
  if (!inputNumber(d, "d", g_telex_mode)) return;
  if (!inputNumber(x, "x", g_telex_mode)) return;
  if (!inputNumber(y, "y", g_telex_mode)) return;
  double xr = a * x + b * y;
  double yr = c * x + d * y;
  String out = "x'=" + formatFloat(xr, 6) + "\n";
  out += "y'=" + formatFloat(yr, 6);
  showResult("Matrix", out);
}

MenuItem kMenu[] = {
    {"Rotate point", actionRotate},
    {"Scale point", actionScale},
    {"Translate point", actionTranslate},
    {"Matrix 2x2", actionMatrix},
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
  M5.Display.print("idk-math-transform");

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

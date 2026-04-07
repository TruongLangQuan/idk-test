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

void actionCircle() {
  double r = 0;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  double area = kPi * r * r;
  double peri = 2.0 * kPi * r;
  String out = "Area = " + formatFloat(area, 6) + "\n";
  out += "Perimeter = " + formatFloat(peri, 6);
  showResult("Circle", out);
}

void actionRectangle() {
  double a = 0, b = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  double area = a * b;
  double peri = 2.0 * (a + b);
  String out = "Area = " + formatFloat(area, 6) + "\n";
  out += "Perimeter = " + formatFloat(peri, 6);
  showResult("Rectangle", out);
}

void actionTriangle() {
  double a = 0, b = 0, c = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  if (!inputNumber(c, "c", g_telex_mode)) return;
  double s = 0.5 * (a + b + c);
  double area = sqrt(fmax(0.0, s * (s - a) * (s - b) * (s - c)));
  showResult("Triangle", "Area = " + formatFloat(area, 6));
}

void actionSphere() {
  double r = 0;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  double vol = 4.0 / 3.0 * kPi * r * r * r;
  double area = 4.0 * kPi * r * r;
  String out = "Surface = " + formatFloat(area, 6) + "\n";
  out += "Volume = " + formatFloat(vol, 6);
  showResult("Sphere", out);
}

void actionCylinder() {
  double r = 0, h = 0;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  if (!inputNumber(h, "h", g_telex_mode)) return;
  double vol = kPi * r * r * h;
  double area = 2.0 * kPi * r * (r + h);
  String out = "Surface = " + formatFloat(area, 6) + "\n";
  out += "Volume = " + formatFloat(vol, 6);
  showResult("Cylinder", out);
}

MenuItem kMenu[] = {
    {"Circle", actionCircle},
    {"Rectangle", actionRectangle},
    {"Triangle (Heron)", actionTriangle},
    {"Sphere", actionSphere},
    {"Cylinder", actionCylinder},
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
  M5.Display.print("idk-math-geom");

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

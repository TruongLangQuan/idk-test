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

struct Vec3 { double x; double y; double z; };
struct Vec2 { int x; int y; };

Vec3 rotateX(const Vec3& v, double a) {
  double c = cos(a), s = sin(a);
  return {v.x, v.y * c - v.z * s, v.y * s + v.z * c};
}

Vec3 rotateY(const Vec3& v, double a) {
  double c = cos(a), s = sin(a);
  return {v.x * c + v.z * s, v.y, -v.x * s + v.z * c};
}

Vec2 project(const Vec3& v, int cx, int cy, double scale) {
  double px = v.x * scale + v.y * scale * 0.5;
  double py = -v.y * scale * 0.25 - v.z * scale;
  return {static_cast<int>(cx + px), static_cast<int>(cy + py)};
}

void drawSurface(double t, double scale) {
  int w = M5.Display.width();
  int h = M5.Display.height();
  int cx = w / 2;
  int cy = h / 2 + 10;

  M5.Display.fillScreen(TFT_BLACK);

  double angX = 0.6 + 0.2 * sin(t * 0.7);
  double angY = t * 0.4;

  for (int yi = -4; yi <= 4; ++yi) {
    Vec2 prev{}; bool have = false;
    for (int xi = -4; xi <= 4; ++xi) {
      double x = xi * 0.5;
      double y = yi * 0.5;
      double z = 0.6 * sin(x + t * 0.3) * cos(y + t * 0.2);
      Vec3 v{x, y, z};
      v = rotateX(v, angX);
      v = rotateY(v, angY);
      Vec2 p = project(v, cx, cy, scale);
      if (have) {
        M5.Display.drawLine(prev.x, prev.y, p.x, p.y, TFT_CYAN);
      }
      prev = p;
      have = true;
    }
  }

  for (int xi = -4; xi <= 4; ++xi) {
    Vec2 prev{}; bool have = false;
    for (int yi = -4; yi <= 4; ++yi) {
      double x = xi * 0.5;
      double y = yi * 0.5;
      double z = 0.6 * sin(x + t * 0.3) * cos(y + t * 0.2);
      Vec3 v{x, y, z};
      v = rotateX(v, angX);
      v = rotateY(v, angY);
      Vec2 p = project(v, cx, cy, scale);
      if (have) {
        M5.Display.drawLine(prev.x, prev.y, p.x, p.y, TFT_BLUE);
      }
      prev = p;
      have = true;
    }
  }

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("3D surface z=sin(x)cos(y)");
  M5.Display.setCursor(2, 12);
  M5.Display.print("A:pause  B:+scale  PWR:-scale  hold A:exit");
}

void graphView() {
  double scale = 18.0;
  bool pause = false;
  double t = 0.0;

  while (true) {
    M5.update();
    if (M5.BtnA.pressedFor(700)) break;
    if (M5.BtnA.wasPressed()) pause = !pause;
    if (M5.BtnB.wasPressed()) scale += 1.5;
    if (M5.BtnPWR.wasPressed()) scale = fmax(6.0, scale - 1.5);

    if (!pause) t += 0.05;
    drawSurface(t, scale);
    delay(20);
  }
}

MenuItem kMenu[] = {
    {"Start 3D graph", graphView},
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
  M5.Display.print("idk-graph-2d3d");

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

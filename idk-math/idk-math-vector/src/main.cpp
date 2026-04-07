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

struct Vec3 {
  double x;
  double y;
  double z;
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

bool inputVec(const char* label, Vec3& v) {
  if (!inputNumber(v.x, String(label) + " x", g_telex_mode)) return false;
  if (!inputNumber(v.y, String(label) + " y", g_telex_mode)) return false;
  if (!inputNumber(v.z, String(label) + " z", g_telex_mode)) return false;
  return true;
}

void actionDot() {
  Vec3 a, b;
  if (!inputVec("A", a)) return;
  if (!inputVec("B", b)) return;
  double dot = a.x * b.x + a.y * b.y + a.z * b.z;
  showResult("Dot", "A·B = " + formatFloat(dot, 6));
}

void actionCross() {
  Vec3 a, b;
  if (!inputVec("A", a)) return;
  if (!inputVec("B", b)) return;
  Vec3 c{a.y * b.z - a.z * b.y,
         a.z * b.x - a.x * b.z,
         a.x * b.y - a.y * b.x};
  String out = "AxB = (" + formatFloat(c.x, 4) + ", " + formatFloat(c.y, 4) + ", " + formatFloat(c.z, 4) + ")";
  showResult("Cross", out);
}

void actionMagnitude() {
  Vec3 a;
  if (!inputVec("A", a)) return;
  double mag = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  showResult("Magnitude", "|A| = " + formatFloat(mag, 6));
}

void actionAngle() {
  Vec3 a, b;
  if (!inputVec("A", a)) return;
  if (!inputVec("B", b)) return;
  double dot = a.x * b.x + a.y * b.y + a.z * b.z;
  double ma = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  double mb = sqrt(b.x * b.x + b.y * b.y + b.z * b.z);
  if (ma == 0 || mb == 0) {
    showResult("Angle", "Zero vector");
    return;
  }
  double cosv = dot / (ma * mb);
  if (cosv > 1) cosv = 1;
  if (cosv < -1) cosv = -1;
  double ang = acos(cosv) * 180.0 / kPi;
  showResult("Angle", "theta = " + formatFloat(ang, 3) + " deg");
}

MenuItem kMenu[] = {
    {"Dot product", actionDot},
    {"Cross product", actionCross},
    {"Magnitude", actionMagnitude},
    {"Angle", actionAngle},
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
  M5.Display.print("idk-math-vector");

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

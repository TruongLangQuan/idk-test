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

bool g_font_loaded = false;
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

bool inputInt(int& out, const String& title, int minValue, int maxValue) {
  double v = 0;
  if (!inputNumber(v, title, g_telex_mode, false)) return false;
  int n = static_cast<int>(llround(v));
  if (n < minValue || n > maxValue) {
    showResult("Input", "Out of range");
    return false;
  }
  out = n;
  return true;
}

void actionOhm() {
  double v = 0, r = 0;
  if (!inputNumber(v, "V (volt)", g_telex_mode)) return;
  if (!inputNumber(r, "R (ohm)", g_telex_mode)) return;
  if (fabs(r) < 1e-9) {
    showResult("Ohm", "R cannot be 0");
    return;
  }
  double i = v / r;
  double p = v * i;
  String out = "I = " + formatFloat(i, 6) + " A\n";
  out += "P = " + formatFloat(p, 6) + " W";
  showResult("Ohm", out);
}

void actionRSeries() {
  int n = 0;
  if (!inputInt(n, "Count", 1, 6)) return;
  double sum = 0;
  for (int i = 0; i < n; ++i) {
    double r = 0;
    if (!inputNumber(r, "R" + String(i + 1), g_telex_mode)) return;
    sum += r;
  }
  String out = "Req = " + formatFloat(sum, 6) + " ohm";
  showResult("R series", out);
}

void actionRParallel() {
  int n = 0;
  if (!inputInt(n, "Count", 1, 6)) return;
  double inv = 0;
  for (int i = 0; i < n; ++i) {
    double r = 0;
    if (!inputNumber(r, "R" + String(i + 1), g_telex_mode)) return;
    if (fabs(r) < 1e-9) {
      showResult("R parallel", "R cannot be 0");
      return;
    }
    inv += 1.0 / r;
  }
  if (inv <= 0) {
    showResult("R parallel", "Invalid");
    return;
  }
  double req = 1.0 / inv;
  String out = "Req = " + formatFloat(req, 6) + " ohm";
  showResult("R parallel", out);
}

void actionForce() {
  double m = 0, a = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(a, "a (m/s2)", g_telex_mode)) return;
  double f = m * a;
  String out = "F = " + formatFloat(f, 6) + " N";
  showResult("Force", out);
}

void actionMomentum() {
  double m = 0, v = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(v, "v (m/s)", g_telex_mode)) return;
  double p = m * v;
  String out = "p = " + formatFloat(p, 6) + " kg.m/s";
  showResult("Momentum", out);
}

void actionKinetic() {
  double m = 0, v = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(v, "v (m/s)", g_telex_mode)) return;
  double e = 0.5 * m * v * v;
  String out = "Ek = " + formatFloat(e, 6) + " J";
  showResult("Kinetic", out);
}

void actionPotential() {
  double m = 0, h = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(h, "h (m)", g_telex_mode)) return;
  double g = 9.8;
  double e = m * g * h;
  String out = "Ep = " + formatFloat(e, 6) + " J\n";
  out += "g = 9.8";
  showResult("Potential", out);
}

void actionSpeed() {
  double s = 0, t = 0;
  if (!inputNumber(s, "s (m)", g_telex_mode)) return;
  if (!inputNumber(t, "t (s)", g_telex_mode)) return;
  if (fabs(t) < 1e-9) {
    showResult("Speed", "t cannot be 0");
    return;
  }
  double v = s / t;
  String out = "v = " + formatFloat(v, 6) + " m/s";
  showResult("Speed", out);
}

void actionAcceleration() {
  double v0 = 0, v1 = 0, t = 0;
  if (!inputNumber(v0, "v0 (m/s)", g_telex_mode)) return;
  if (!inputNumber(v1, "v (m/s)", g_telex_mode)) return;
  if (!inputNumber(t, "t (s)", g_telex_mode)) return;
  if (fabs(t) < 1e-9) {
    showResult("Accel", "t cannot be 0");
    return;
  }
  double a = (v1 - v0) / t;
  String out = "a = " + formatFloat(a, 6) + " m/s2";
  showResult("Accel", out);
}

void actionSnell() {
  double n1 = 0, n2 = 0, t1 = 0;
  if (!inputNumber(n1, "n1", g_telex_mode)) return;
  if (!inputNumber(n2, "n2", g_telex_mode)) return;
  if (!inputNumber(t1, "theta1", g_telex_mode)) return;
  if (fabs(n2) < 1e-9) {
    showResult("Snell", "n2 cannot be 0");
    return;
  }
  double rad = t1 * 3.141592653589793 / 180.0;
  double ratio = (n1 / n2) * sin(rad);
  if (ratio > 1.0) {
    showResult("Snell", "Total reflection");
    return;
  }
  double t2 = asin(ratio) * 180.0 / 3.141592653589793;
  String out = "theta2 = " + formatFloat(t2, 4) + " deg";
  showResult("Snell", out);
}

MenuItem kMenu[] = {
    {"Ohm law", actionOhm},
    {"R series", actionRSeries},
    {"R parallel", actionRParallel},
    {"Force (F=ma)", actionForce},
    {"Momentum", actionMomentum},
    {"Kinetic energy", actionKinetic},
    {"Potential energy", actionPotential},
    {"Speed (v=s/t)", actionSpeed},
    {"Acceleration", actionAcceleration},
    {"Snell law", actionSnell},
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
  M5.Display.print("idk-physics");

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

  g_font_loaded = loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);

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

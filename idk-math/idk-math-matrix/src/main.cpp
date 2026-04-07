#include <M5Unified.h>
#include <cmath>
#include <vector>

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

bool inputMatrix2(double m[2][2], const char* label) {
  for (int r = 0; r < 2; ++r) {
    for (int c = 0; c < 2; ++c) {
      String name = String(label) + "[" + String(r + 1) + "," + String(c + 1) + "]";
      if (!inputNumber(m[r][c], name, g_telex_mode)) return false;
    }
  }
  return true;
}

bool inputMatrix3(double m[3][3], const char* label) {
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) {
      String name = String(label) + "[" + String(r + 1) + "," + String(c + 1) + "]";
      if (!inputNumber(m[r][c], name, g_telex_mode)) return false;
    }
  }
  return true;
}

void actionAdd2() {
  double a[2][2], b[2][2];
  if (!inputMatrix2(a, "A")) return;
  if (!inputMatrix2(b, "B")) return;
  String out = "A+B=\n";
  for (int r = 0; r < 2; ++r) {
    out += formatFloat(a[r][0] + b[r][0], 4) + " ";
    out += formatFloat(a[r][1] + b[r][1], 4) + "\n";
  }
  showResult("Add 2x2", out);
}

void actionMul2() {
  double a[2][2], b[2][2];
  if (!inputMatrix2(a, "A")) return;
  if (!inputMatrix2(b, "B")) return;
  double c[2][2];
  for (int r = 0; r < 2; ++r) {
    for (int ccol = 0; ccol < 2; ++ccol) {
      c[r][ccol] = a[r][0] * b[0][ccol] + a[r][1] * b[1][ccol];
    }
  }
  String out = "A*B=\n";
  for (int r = 0; r < 2; ++r) {
    out += formatFloat(c[r][0], 4) + " ";
    out += formatFloat(c[r][1], 4) + "\n";
  }
  showResult("Mul 2x2", out);
}

void actionDet2() {
  double a[2][2];
  if (!inputMatrix2(a, "A")) return;
  double det = a[0][0] * a[1][1] - a[0][1] * a[1][0];
  showResult("Det 2x2", "det = " + formatFloat(det, 6));
}

void actionInv2() {
  double a[2][2];
  if (!inputMatrix2(a, "A")) return;
  double det = a[0][0] * a[1][1] - a[0][1] * a[1][0];
  if (fabs(det) < 1e-12) {
    showResult("Inv 2x2", "Not invertible");
    return;
  }
  double inv[2][2];
  inv[0][0] = a[1][1] / det;
  inv[0][1] = -a[0][1] / det;
  inv[1][0] = -a[1][0] / det;
  inv[1][1] = a[0][0] / det;
  String out = "A^-1=\n";
  for (int r = 0; r < 2; ++r) {
    out += formatFloat(inv[r][0], 6) + " ";
    out += formatFloat(inv[r][1], 6) + "\n";
  }
  showResult("Inv 2x2", out);
}

void actionDet3() {
  double a[3][3];
  if (!inputMatrix3(a, "A")) return;
  double det =
      a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
      a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
      a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
  showResult("Det 3x3", "det = " + formatFloat(det, 6));
}

void actionSolve3() {
  double a[3][3];
  if (!inputMatrix3(a, "A")) return;
  double b[3];
  for (int i = 0; i < 3; ++i) {
    if (!inputNumber(b[i], String("b") + String(i + 1), g_telex_mode)) return;
  }
  double det =
      a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
      a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
      a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
  if (fabs(det) < 1e-12) {
    showResult("Solve 3x3", "No unique solution");
    return;
  }

  double detx =
      b[0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
      a[0][1] * (b[1] * a[2][2] - a[1][2] * b[2]) +
      a[0][2] * (b[1] * a[2][1] - a[1][1] * b[2]);
  double dety =
      a[0][0] * (b[1] * a[2][2] - a[1][2] * b[2]) -
      b[0] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
      a[0][2] * (a[1][0] * b[2] - b[1] * a[2][0]);
  double detz =
      a[0][0] * (a[1][1] * b[2] - b[1] * a[2][1]) -
      a[0][1] * (a[1][0] * b[2] - b[1] * a[2][0]) +
      b[0] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);

  double x = detx / det;
  double y = dety / det;
  double z = detz / det;
  String out = "x=" + formatFloat(x, 6) + "\n";
  out += "y=" + formatFloat(y, 6) + "\n";
  out += "z=" + formatFloat(z, 6);
  showResult("Solve 3x3", out);
}

MenuItem kMenu[] = {
    {"Add 2x2", actionAdd2},
    {"Mul 2x2", actionMul2},
    {"Det 2x2", actionDet2},
    {"Inv 2x2", actionInv2},
    {"Det 3x3", actionDet3},
    {"Solve 3x3", actionSolve3},
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
  M5.Display.print("idk-math-matrix");

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

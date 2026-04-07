#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

bool g_telex_mode = false;
String g_status = "Ready";


constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

int g_menu_idx = 0;
int g_menu_scroll = 0;

void actionSnell() {
  double n1 = 0, n2 = 0, t1 = 0;
  if (!inputNumber(n1, "n1", g_telex_mode)) return;
  if (!inputNumber(n2, "n2", g_telex_mode)) return;
  if (!inputNumber(t1, "theta1", g_telex_mode)) return;
  if (fabs(n2) < 1e-9) { showResult("Snell", "n2 cannot be 0"); return; }
  double rad = t1 * 3.141592653589793 / 180.0;
  double ratio = (n1 / n2) * sin(rad);
  if (ratio > 1.0) {
    showResult("Snell", "Total reflection");
    return;
  }
  double t2 = asin(ratio) * 180.0 / 3.141592653589793;
  showResult("Snell", "theta2 = " + formatFloat(t2, 4) + " deg");
}

void actionLens() {
  double f = 0, do_ = 0;
  if (!inputNumber(f, "f", g_telex_mode)) return;
  if (!inputNumber(do_, "do", g_telex_mode)) return;
  if (fabs(f) < 1e-9 || fabs(do_) < 1e-9) {
    showResult("Lens", "f/do cannot be 0");
    return;
  }
  double di = 1.0 / (1.0 / f - 1.0 / do_);
  String out = "di = " + formatFloat(di, 6);
  showResult("Lens", out);
}

void actionMagnification() {
  double di = 0, do_ = 0;
  if (!inputNumber(di, "di", g_telex_mode)) return;
  if (!inputNumber(do_, "do", g_telex_mode)) return;
  if (fabs(do_) < 1e-9) { showResult("Mag", "do cannot be 0"); return; }
  double m = -di / do_;
  showResult("Mag", "m = " + formatFloat(m, 6));
}

void actionMirror() {
  double f = 0, do_ = 0;
  if (!inputNumber(f, "f", g_telex_mode)) return;
  if (!inputNumber(do_, "do", g_telex_mode)) return;
  if (fabs(f) < 1e-9 || fabs(do_) < 1e-9) {
    showResult("Mirror", "f/do cannot be 0");
    return;
  }
  double di = 1.0 / (1.0 / f - 1.0 / do_);
  showResult("Mirror", "di = " + formatFloat(di, 6));
}

void actionCritical() {
  double n1 = 0, n2 = 0;
  if (!inputNumber(n1, "n1", g_telex_mode)) return;
  if (!inputNumber(n2, "n2", g_telex_mode)) return;
  if (n1 <= n2) { showResult("Critical", "Need n1 > n2"); return; }
  double ratio = n2 / n1;
  double tc = asin(ratio) * 180.0 / 3.141592653589793;
  showResult("Critical", "theta_c = " + formatFloat(tc, 4) + " deg");
}

void actionRefrIndex() {
  double v = 0;
  if (!inputNumber(v, "v (m/s)", g_telex_mode)) return;
  if (fabs(v) < 1e-9) { showResult("Index", "v cannot be 0"); return; }
  const double c = 3.0e8;
  double n = c / v;
  showResult("Index", "n = " + formatFloat(n, 6));
}

MenuItem kMenu[] = {
    {"Snell", actionSnell},
    {"Lens 1/f", actionLens},
    {"Magnification", actionMagnification},
    {"Mirror 1/f", actionMirror},
    {"Critical angle", actionCritical},
    {"n = c/v", actionRefrIndex},
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
  M5.Display.print("idk-physics-optics");

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

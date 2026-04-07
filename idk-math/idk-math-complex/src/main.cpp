#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct Complex {
  double re;
  double im;
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

bool inputComplex(Complex& out, const char* label) {
  if (!inputNumber(out.re, String(label) + " Re", g_telex_mode)) return false;
  if (!inputNumber(out.im, String(label) + " Im", g_telex_mode)) return false;
  return true;
}

String formatComplex(const Complex& c) {
  String out = formatFloat(c.re, 6);
  out += (c.im >= 0) ? " + " : " - ";
  out += formatFloat(fabs(c.im), 6);
  out += "i";
  return out;
}

void actionAdd() {
  Complex a, b;
  if (!inputComplex(a, "A")) return;
  if (!inputComplex(b, "B")) return;
  Complex c{a.re + b.re, a.im + b.im};
  showResult("Add", formatComplex(c));
}

void actionSub() {
  Complex a, b;
  if (!inputComplex(a, "A")) return;
  if (!inputComplex(b, "B")) return;
  Complex c{a.re - b.re, a.im - b.im};
  showResult("Sub", formatComplex(c));
}

void actionMul() {
  Complex a, b;
  if (!inputComplex(a, "A")) return;
  if (!inputComplex(b, "B")) return;
  Complex c{a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
  showResult("Mul", formatComplex(c));
}

void actionDiv() {
  Complex a, b;
  if (!inputComplex(a, "A")) return;
  if (!inputComplex(b, "B")) return;
  double denom = b.re * b.re + b.im * b.im;
  if (fabs(denom) < 1e-12) {
    showResult("Div", "Divide by zero");
    return;
  }
  Complex c{(a.re * b.re + a.im * b.im) / denom, (a.im * b.re - a.re * b.im) / denom};
  showResult("Div", formatComplex(c));
}

void actionMagArg() {
  Complex a;
  if (!inputComplex(a, "A")) return;
  double r = sqrt(a.re * a.re + a.im * a.im);
  double ang = atan2(a.im, a.re) * 180.0 / 3.141592653589793;
  String out = "|A| = " + formatFloat(r, 6) + "\n";
  out += "arg = " + formatFloat(ang, 3) + " deg";
  showResult("Mag/Arg", out);
}

void actionPolar() {
  double r = 0, ang = 0;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  if (!inputNumber(ang, "theta deg", g_telex_mode)) return;
  double rad = ang * 3.141592653589793 / 180.0;
  Complex c{r * cos(rad), r * sin(rad)};
  showResult("Polar->Rect", formatComplex(c));
}

MenuItem kMenu[] = {
    {"Add", actionAdd},
    {"Subtract", actionSub},
    {"Multiply", actionMul},
    {"Divide", actionDiv},
    {"Magnitude/Angle", actionMagArg},
    {"Polar -> Rect", actionPolar},
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
  M5.Display.print("idk-math-complex");

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

#include <M5Unified.h>
#include <cmath>
#include <cctype>

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

class ExprParser {
 public:
  ExprParser(const String& s, double x_value) : s_(s), pos_(0), x_(x_value) {}

  double parse(bool& ok) {
    ok = true;
    double v = parseExpr(ok);
    skipSpace();
    if (pos_ < s_.length()) ok = false;
    return v;
  }

 private:
  const String s_;
  size_t pos_;
  double x_;

  void skipSpace() {
    while (pos_ < s_.length() && isspace(static_cast<unsigned char>(s_[pos_]))) pos_++;
  }

  bool match(char c) {
    skipSpace();
    if (pos_ < s_.length() && s_[pos_] == c) {
      pos_++;
      return true;
    }
    return false;
  }

  double parseExpr(bool& ok) {
    double v = parseTerm(ok);
    while (ok) {
      if (match('+')) {
        v += parseTerm(ok);
      } else if (match('-')) {
        v -= parseTerm(ok);
      } else {
        break;
      }
    }
    return v;
  }

  double parseTerm(bool& ok) {
    double v = parsePower(ok);
    while (ok) {
      if (match('*')) {
        v *= parsePower(ok);
      } else if (match('/')) {
        double d = parsePower(ok);
        if (d == 0.0) {
          ok = false;
          return 0.0;
        }
        v /= d;
      } else {
        break;
      }
    }
    return v;
  }

  double parsePower(bool& ok) {
    double v = parseUnary(ok);
    if (match('^')) {
      double e = parsePower(ok);
      v = pow(v, e);
    }
    return v;
  }

  double parseUnary(bool& ok) {
    skipSpace();
    if (match('+')) return parseUnary(ok);
    if (match('-')) return -parseUnary(ok);
    return parsePrimary(ok);
  }

  bool parseIdentifier(String& out) {
    skipSpace();
    if (pos_ >= s_.length() || !isalpha(static_cast<unsigned char>(s_[pos_]))) return false;
    String id = "";
    while (pos_ < s_.length() && isalpha(static_cast<unsigned char>(s_[pos_]))) {
      id += s_[pos_++];
    }
    out = id;
    return true;
  }

  double parseNumber(bool& ok) {
    skipSpace();
    const char* start = s_.c_str() + pos_;
    char* endptr = nullptr;
    double v = strtod(start, &endptr);
    if (endptr == start) {
      ok = false;
      return 0.0;
    }
    pos_ += static_cast<size_t>(endptr - start);
    return v;
  }

  double deg2rad(double d) { return d * kPi / 180.0; }

  double parsePrimary(bool& ok) {
    skipSpace();
    if (match('(')) {
      double v = parseExpr(ok);
      if (!match(')')) ok = false;
      return v;
    }

    String ident;
    if (parseIdentifier(ident)) {
      ident.toLowerCase();
      if (ident == "x") return x_;
      if (ident == "pi") return kPi;
      if (ident == "e") return 2.718281828459045;

      if (match('(')) {
        double arg = parseExpr(ok);
        if (!match(')')) ok = false;
        if (!ok) return 0.0;
        if (ident == "sin") return sin(deg2rad(arg));
        if (ident == "cos") return cos(deg2rad(arg));
        if (ident == "tan") return tan(deg2rad(arg));
        if (ident == "sqrt") return sqrt(arg);
        if (ident == "abs") return fabs(arg);
        if (ident == "log") return log10(arg);
        if (ident == "ln") return log(arg);
        if (ident == "exp") return exp(arg);
      }
      ok = false;
      return 0.0;
    }

    return parseNumber(ok);
  }
};

bool evalExpr(const String& expr, double x, double& out) {
  bool ok = false;
  ExprParser parser(expr, x);
  double v = parser.parse(ok);
  if (!ok || isnan(v) || isinf(v)) return false;
  out = v;
  return true;
}

void actionDerivative() {
  String expr = "";
  if (!keyboardInput(expr, "f(x)", false, false, g_telex_mode, 120, true)) return;
  double x = 0;
  if (!inputNumber(x, "x", g_telex_mode)) return;
  double h = 1e-4;
  double f1 = 0, f2 = 0;
  if (!evalExpr(expr, x + h, f1) || !evalExpr(expr, x - h, f2)) {
    showResult("Derivative", "Bad expression");
    return;
  }
  double d = (f1 - f2) / (2.0 * h);
  showResult("Derivative", "f'(x) = " + formatFloat(d, 6));
}

void actionIntegral() {
  String expr = "";
  if (!keyboardInput(expr, "f(x)", false, false, g_telex_mode, 120, true)) return;
  double a = 0, b = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  int n = 200;
  double h = (b - a) / n;
  double sum = 0.0;
  for (int i = 0; i <= n; ++i) {
    double x = a + h * i;
    double fx = 0;
    if (!evalExpr(expr, x, fx)) {
      showResult("Integral", "Bad expression");
      return;
    }
    if (i == 0 || i == n) sum += fx;
    else sum += 2.0 * fx;
  }
  double area = 0.5 * h * sum;
  showResult("Integral", "Area = " + formatFloat(area, 6));
}

void actionTangent() {
  String expr = "";
  if (!keyboardInput(expr, "f(x)", false, false, g_telex_mode, 120, true)) return;
  double x = 0;
  if (!inputNumber(x, "x", g_telex_mode)) return;
  double y = 0;
  if (!evalExpr(expr, x, y)) {
    showResult("Tangent", "Bad expression");
    return;
  }
  double h = 1e-4;
  double f1 = 0, f2 = 0;
  if (!evalExpr(expr, x + h, f1) || !evalExpr(expr, x - h, f2)) {
    showResult("Tangent", "Bad expression");
    return;
  }
  double m = (f1 - f2) / (2.0 * h);
  String out = "y = " + formatFloat(m, 6) + "(x - " + formatFloat(x, 4) + ") + " + formatFloat(y, 6);
  showResult("Tangent", out);
}

MenuItem kMenu[] = {
    {"Derivative f'(x)", actionDerivative},
    {"Integral \xE2\x88\xAB f(x)dx", actionIntegral},
    {"Tangent line", actionTangent},
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
  M5.Display.print("idk-math-calc");

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

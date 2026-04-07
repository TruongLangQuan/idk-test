#include <M5Unified.h>
#include <cmath>
#include <cctype>
#include <vector>

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

  double deg2rad(double d) { return d * 3.141592653589793 / 180.0; }
  double rad2deg(double r) { return r * 180.0 / 3.141592653589793; }

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
      if (ident == "pi") return 3.141592653589793;
      if (ident == "e") return 2.718281828459045;

      if (match('(')) {
        double arg = parseExpr(ok);
        if (!match(')')) ok = false;
        if (!ok) return 0.0;
        if (ident == "sin") return sin(deg2rad(arg));
        if (ident == "cos") return cos(deg2rad(arg));
        if (ident == "tan") return tan(deg2rad(arg));
        if (ident == "asin") return rad2deg(asin(arg));
        if (ident == "acos") return rad2deg(acos(arg));
        if (ident == "atan") return rad2deg(atan(arg));
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

void actionLinear() {
  double a = 0, b = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  if (fabs(a) < 1e-12) {
    showResult("Linear", "No solution (a=0)");
    return;
  }
  double x = -b / a;
  showResult("Linear", "x = " + formatFloat(x, 6));
}

void actionQuadratic() {
  double a = 0, b = 0, c = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  if (!inputNumber(c, "c", g_telex_mode)) return;

  if (fabs(a) < 1e-12) {
    if (fabs(b) < 1e-12) {
      showResult("Quadratic", "No solution");
      return;
    }
    double x = -c / b;
    showResult("Quadratic", "Linear x = " + formatFloat(x, 6));
    return;
  }

  double d = b * b - 4.0 * a * c;
  if (d < 0) {
    showResult("Quadratic", "No real roots");
    return;
  }
  double r1 = (-b + sqrt(d)) / (2.0 * a);
  double r2 = (-b - sqrt(d)) / (2.0 * a);
  String out = "x1 = " + formatFloat(r1, 6) + "\nx2 = " + formatFloat(r2, 6);
  showResult("Quadratic", out);
}

void actionSystem2() {
  double a1 = 0, b1 = 0, c1 = 0;
  double a2 = 0, b2 = 0, c2 = 0;
  if (!inputNumber(a1, "a1", g_telex_mode)) return;
  if (!inputNumber(b1, "b1", g_telex_mode)) return;
  if (!inputNumber(c1, "c1", g_telex_mode)) return;
  if (!inputNumber(a2, "a2", g_telex_mode)) return;
  if (!inputNumber(b2, "b2", g_telex_mode)) return;
  if (!inputNumber(c2, "c2", g_telex_mode)) return;

  double det = a1 * b2 - a2 * b1;
  if (fabs(det) < 1e-12) {
    showResult("System", "No unique solution");
    return;
  }
  double x = (c1 * b2 - c2 * b1) / det;
  double y = (a1 * c2 - a2 * c1) / det;
  String out = "x = " + formatFloat(x, 6) + "\ny = " + formatFloat(y, 6);
  showResult("System", out);
}

void actionRoot() {
  String expr = "";
  if (!keyboardInput(expr, "f(x)", false, false, g_telex_mode, 120, true)) return;
  double a = 0, b = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;

  double fa = 0, fb = 0;
  if (!evalExpr(expr, a, fa) || !evalExpr(expr, b, fb)) {
    showResult("Root", "Bad expression");
    return;
  }
  if (fa * fb > 0) {
    showResult("Root", "No sign change");
    return;
  }

  double lo = a, hi = b;
  for (int i = 0; i < 60; ++i) {
    double mid = 0.5 * (lo + hi);
    double fm = 0;
    if (!evalExpr(expr, mid, fm)) break;
    if (fa * fm <= 0) {
      hi = mid;
      fb = fm;
    } else {
      lo = mid;
      fa = fm;
    }
  }
  double x = 0.5 * (lo + hi);
  showResult("Root", "x = " + formatFloat(x, 6));
}

void actionEval() {
  String expr = "";
  if (!keyboardInput(expr, "f(x)", false, false, g_telex_mode, 120, true)) return;
  double x = 0;
  if (!inputNumber(x, "x", g_telex_mode)) return;
  double v = 0;
  if (!evalExpr(expr, x, v)) {
    showResult("Evaluate", "Bad expression");
    return;
  }
  showResult("Evaluate", "f(x) = " + formatFloat(v, 6));
}

MenuItem kMenu[] = {
    {"Solve linear", actionLinear},
    {"Quadratic", actionQuadratic},
    {"Solve system 2x2", actionSystem2},
    {"Root f(x)=0", actionRoot},
    {"Evaluate f(x)", actionEval},
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
  M5.Display.print("idk-math-algebra");

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

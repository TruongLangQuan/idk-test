#include <M5Unified.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cctype>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;
constexpr uint32_t kLongPressMs = 650;

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

class ExprParser {
 public:
  explicit ExprParser(const String& s) : s_(s), pos_(0) {}

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
      }
      ok = false;
      return 0.0;
    }

    return parseNumber(ok);
  }
};

void actionExpr() {
  String expr = "";
  if (!keyboardInput(expr, "Expression", false, false, g_telex_mode, 80)) return;
  bool ok = false;
  ExprParser parser(expr);
  double v = parser.parse(ok);
  if (!ok || isnan(v) || isinf(v)) {
    showResult("Expr", "Invalid expression");
    return;
  }
  String out = "Result: " + formatFloat(v, 6);
  showResult("Expr", out);
}

void actionQuadratic() {
  double a = 0, b = 0, c = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  if (!inputNumber(c, "c", g_telex_mode)) return;
  if (fabs(a) < 1e-9) {
    if (fabs(b) < 1e-9) {
      showResult("Quadratic", "No solution");
      return;
    }
    double x = -c / b;
    showResult("Quadratic", "Linear x = " + formatFloat(x, 6));
    return;
  }
  double d = b * b - 4 * a * c;
  if (d < 0) {
    showResult("Quadratic", "No real roots");
    return;
  }
  double r = sqrt(d);
  double x1 = (-b + r) / (2 * a);
  double x2 = (-b - r) / (2 * a);
  String out = "x1 = " + formatFloat(x1, 6) + "\n" + "x2 = " + formatFloat(x2, 6);
  showResult("Quadratic", out);
}

long long gcdll(long long a, long long b) {
  a = llabs(a); b = llabs(b);
  while (b != 0) { long long t = a % b; a = b; b = t; }
  return a == 0 ? 1 : a;
}

void actionGcdLcm() {
  double a = 0, b = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  long long ia = static_cast<long long>(llround(a));
  long long ib = static_cast<long long>(llround(b));
  long long g = gcdll(ia, ib);
  long long l = llabs(ia / g * ib);
  String out = "GCD = " + String(g) + "\nLCM = " + String(l);
  showResult("GCD/LCM", out);
}

bool isPrime(long long n) {
  if (n < 2) return false;
  if (n % 2 == 0) return n == 2;
  for (long long i = 3; i * i <= n; i += 2) {
    if (n % i == 0) return false;
  }
  return true;
}

void actionPrime() {
  double a = 0;
  if (!inputNumber(a, "n", g_telex_mode, false)) return;
  long long n = static_cast<long long>(llround(a));
  String out = String(n) + (isPrime(n) ? " is prime" : " is not prime");
  showResult("Prime", out);
}

void actionFactor() {
  double a = 0;
  if (!inputNumber(a, "n", g_telex_mode, false)) return;
  long long n = static_cast<long long>(llround(a));
  if (n < 2) {
    showResult("Factor", "n < 2");
    return;
  }
  String out = String(n) + " = ";
  long long tmp = n;
  bool first = true;
  for (long long p = 2; p * p <= tmp; ++p) {
    while (tmp % p == 0) {
      if (!first) out += " * ";
      out += String(p);
      tmp /= p;
      first = false;
    }
  }
  if (tmp > 1) {
    if (!first) out += " * ";
    out += String(tmp);
  }
  showResult("Factor", out);
}

void actionSolve2x2() {
  double a = 0, b = 0, c = 0;
  double d = 0, e = 0, f = 0;
  if (!inputNumber(a, "a", g_telex_mode)) return;
  if (!inputNumber(b, "b", g_telex_mode)) return;
  if (!inputNumber(c, "c", g_telex_mode)) return;
  if (!inputNumber(d, "d", g_telex_mode)) return;
  if (!inputNumber(e, "e", g_telex_mode)) return;
  if (!inputNumber(f, "f", g_telex_mode)) return;

  double det = a * e - b * d;
  if (fabs(det) < 1e-9) {
    showResult("2x2", "No unique solution");
    return;
  }
  double x = (c * e - b * f) / det;
  double y = (a * f - c * d) / det;
  String out = "x = " + formatFloat(x, 6) + "\n" + "y = " + formatFloat(y, 6);
  showResult("2x2", out);
}

double nCr(int n, int r) {
  if (r < 0 || r > n) return 0.0;
  r = std::min(r, n - r);
  long double res = 1.0;
  for (int i = 1; i <= r; ++i) {
    res = res * (n - r + i) / i;
  }
  return static_cast<double>(res);
}

double nPr(int n, int r) {
  if (r < 0 || r > n) return 0.0;
  long double res = 1.0;
  for (int i = 0; i < r; ++i) {
    res *= (n - i);
  }
  return static_cast<double>(res);
}

void actionCombPerm() {
  double dn = 0, dr = 0;
  if (!inputNumber(dn, "n", g_telex_mode, false)) return;
  if (!inputNumber(dr, "r", g_telex_mode, false)) return;
  int n = static_cast<int>(llround(dn));
  int r = static_cast<int>(llround(dr));
  if (n < 0 || r < 0 || r > n) {
    showResult("nCr/nPr", "Invalid n or r");
    return;
  }
  double comb = nCr(n, r);
  double perm = nPr(n, r);
  String out = "nCr = " + formatFloat(comb, 0) + "\n" + "nPr = " + formatFloat(perm, 0);
  showResult("nCr/nPr", out);
}

bool parseNumberList(const String& s, std::vector<double>& out) {
  out.clear();
  const char* p = s.c_str();
  while (*p) {
    while (*p && (isspace(static_cast<unsigned char>(*p)) || *p == ',' || *p == ';')) {
      ++p;
    }
    if (!*p) break;
    char* endptr = nullptr;
    double v = strtod(p, &endptr);
    if (endptr == p) return false;
    out.push_back(v);
    p = endptr;
  }
  return !out.empty();
}

void actionStats() {
  String text = "";
  if (!keyboardInput(text, "List (space)", false, false, g_telex_mode, 160)) return;
  std::vector<double> values;
  if (!parseNumberList(text, values)) {
    showResult("Stats", "Invalid list");
    return;
  }
  double sum = 0.0;
  for (double v : values) sum += v;
  double mean = sum / values.size();
  double var = 0.0;
  for (double v : values) {
    double d = v - mean;
    var += d * d;
  }
  var /= values.size();
  double stddev = sqrt(var);
  String out = "n = " + String(values.size()) + "\n";
  out += "mean = " + formatFloat(mean, 6) + "\n";
  out += "std = " + formatFloat(stddev, 6);
  showResult("Stats", out);
}

MenuItem kMenu[] = {
    {"Expression", actionExpr},
    {"Quadratic", actionQuadratic},
    {"GCD / LCM", actionGcdLcm},
    {"Prime?", actionPrime},
    {"Factorize", actionFactor},
    {"Solve 2x2", actionSolve2x2},
    {"nCr / nPr", actionCombPerm},
    {"Statistics", actionStats},
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
  M5.Display.print("idk-math-num");

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

#include <M5Unified.h>
#include <cmath>
#include <cctype>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct GraphFunc {
  String expr;
  bool enabled;
  uint16_t color;
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

GraphFunc g_funcs[3] = {
    {"sin(x)", true, TFT_CYAN},
    {"cos(x)", true, TFT_YELLOW},
    {"x", false, TFT_MAGENTA},
};

double g_x_center = 0.0;
double g_y_center = 0.0;
double g_x_span = 10.0;
double g_y_span = 8.0;
double g_cursor_x = 0.0;

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

void drawGraphFrame(const String& modeLabel) {
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print(modeLabel);
  M5.Display.setCursor(2, 14);
  M5.Display.printf("x:[%g,%g]", g_x_center - g_x_span / 2.0, g_x_center + g_x_span / 2.0);
  M5.Display.setCursor(2, 24);
  M5.Display.printf("y:[%g,%g]", g_y_center - g_y_span / 2.0, g_y_center + g_y_span / 2.0);
}

double niceStep(double span) {
  double rough = span / 6.0;
  double pow10 = pow(10.0, floor(log10(fabs(rough))));
  double n = rough / pow10;
  if (n < 1.5) return 1.0 * pow10;
  if (n < 3.0) return 2.0 * pow10;
  if (n < 7.0) return 5.0 * pow10;
  return 10.0 * pow10;
}

void drawGraph() {
  int w = M5.Display.width();
  int h = M5.Display.height();
  double x_min = g_x_center - g_x_span / 2.0;
  double x_max = g_x_center + g_x_span / 2.0;
  double y_min = g_y_center - g_y_span / 2.0;
  double y_max = g_y_center + g_y_span / 2.0;

  M5.Display.fillScreen(TFT_BLACK);

  double stepX = niceStep(g_x_span);
  double stepY = niceStep(g_y_span);

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  for (double x = floor(x_min / stepX) * stepX; x <= x_max; x += stepX) {
    int sx = static_cast<int>((x - x_min) / (x_max - x_min) * (w - 1));
    if (sx >= 0 && sx < w) M5.Display.drawFastVLine(sx, 0, h, TFT_DARKGREY);
  }
  for (double y = floor(y_min / stepY) * stepY; y <= y_max; y += stepY) {
    int sy = static_cast<int>((y_max - y) / (y_max - y_min) * (h - 1));
    if (sy >= 0 && sy < h) M5.Display.drawFastHLine(0, sy, w, TFT_DARKGREY);
  }

  if (0 >= x_min && 0 <= x_max) {
    int sx = static_cast<int>((0 - x_min) / (x_max - x_min) * (w - 1));
    M5.Display.drawFastVLine(sx, 0, h, TFT_WHITE);
  }
  if (0 >= y_min && 0 <= y_max) {
    int sy = static_cast<int>((y_max - 0) / (y_max - y_min) * (h - 1));
    M5.Display.drawFastHLine(0, sy, w, TFT_WHITE);
  }

  for (int f = 0; f < 3; ++f) {
    if (!g_funcs[f].enabled || g_funcs[f].expr.length() == 0) continue;
    bool have_prev = false;
    int prev_x = 0, prev_y = 0;
    for (int sx = 0; sx < w; ++sx) {
      double x = x_min + (x_max - x_min) * (static_cast<double>(sx) / (w - 1));
      double y = 0.0;
      if (!evalExpr(g_funcs[f].expr, x, y)) {
        have_prev = false;
        continue;
      }
      if (y < y_min || y > y_max) {
        have_prev = false;
        continue;
      }
      int sy = static_cast<int>((y_max - y) / (y_max - y_min) * (h - 1));
      if (have_prev) {
        M5.Display.drawLine(prev_x, prev_y, sx, sy, g_funcs[f].color);
      }
      prev_x = sx;
      prev_y = sy;
      have_prev = true;
    }
  }
}

void graphView() {
  int mode = 0;
  g_cursor_x = g_x_center;

  while (true) {
    M5.update();
    if (M5.BtnA.pressedFor(700)) {
      break;
    }
    if (M5.BtnA.wasPressed()) {
      mode = (mode + 1) % 4;
    }
    if (M5.BtnB.wasPressed()) {
      if (mode == 0) g_x_center += g_x_span * 0.1;
      if (mode == 1) g_y_center += g_y_span * 0.1;
      if (mode == 2) {
        g_x_span *= 0.85;
        g_y_span *= 0.85;
      }
      if (mode == 3) g_cursor_x += g_x_span * 0.05;
    }
    if (M5.BtnPWR.wasPressed()) {
      if (mode == 0) g_x_center -= g_x_span * 0.1;
      if (mode == 1) g_y_center -= g_y_span * 0.1;
      if (mode == 2) {
        g_x_span *= 1.15;
        g_y_span *= 1.15;
      }
      if (mode == 3) g_cursor_x -= g_x_span * 0.05;
    }

    drawGraph();
    const char* labels[] = {"PanX", "PanY", "Zoom", "Cursor"};
    String modeLabel = String("Mode:") + labels[mode] + "  A:mode B:+ PWR:- (hold A exit)";
    drawGraphFrame(modeLabel);

    if (mode == 3) {
      int w = M5.Display.width();
      int h = M5.Display.height();
      double x_min = g_x_center - g_x_span / 2.0;
      double x_max = g_x_center + g_x_span / 2.0;
      double y_min = g_y_center - g_y_span / 2.0;
      double y_max = g_y_center + g_y_span / 2.0;
      int sx = static_cast<int>((g_cursor_x - x_min) / (x_max - x_min) * (w - 1));
      if (sx >= 0 && sx < w) M5.Display.drawFastVLine(sx, 0, h, TFT_GREEN);
      String info = "x=" + formatFloat(g_cursor_x, 3);
      for (int f = 0; f < 3; ++f) {
        if (!g_funcs[f].enabled || g_funcs[f].expr.length() == 0) continue;
        double y = 0.0;
        if (evalExpr(g_funcs[f].expr, g_cursor_x, y)) {
          info += " y" + String(f + 1) + "=" + formatFloat(y, 3);
        }
      }
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(2, 116);
      M5.Display.print(info);
    }

    delay(30);
  }
}

void actionEdit1() {
  String text = g_funcs[0].expr;
  if (keyboardInput(text, "y1(x)", false, false, g_telex_mode, 120, true)) {
    g_funcs[0].expr = text;
  }
}

void actionEdit2() {
  String text = g_funcs[1].expr;
  if (keyboardInput(text, "y2(x)", false, false, g_telex_mode, 120, true)) {
    g_funcs[1].expr = text;
  }
}

void actionEdit3() {
  String text = g_funcs[2].expr;
  if (keyboardInput(text, "y3(x)", false, false, g_telex_mode, 120, true)) {
    g_funcs[2].expr = text;
  }
}

void actionToggle1() { g_funcs[0].enabled = !g_funcs[0].enabled; }
void actionToggle2() { g_funcs[1].enabled = !g_funcs[1].enabled; }
void actionToggle3() { g_funcs[2].enabled = !g_funcs[2].enabled; }

void actionSetRange() {
  double xmin = g_x_center - g_x_span / 2.0;
  double xmax = g_x_center + g_x_span / 2.0;
  double ymin = g_y_center - g_y_span / 2.0;
  double ymax = g_y_center + g_y_span / 2.0;
  if (!inputNumber(xmin, "x min", g_telex_mode)) return;
  if (!inputNumber(xmax, "x max", g_telex_mode)) return;
  if (!inputNumber(ymin, "y min", g_telex_mode)) return;
  if (!inputNumber(ymax, "y max", g_telex_mode)) return;
  if (xmax <= xmin || ymax <= ymin) {
    showResult("Range", "Invalid range");
    return;
  }
  g_x_center = 0.5 * (xmin + xmax);
  g_y_center = 0.5 * (ymin + ymax);
  g_x_span = xmax - xmin;
  g_y_span = ymax - ymin;
}

void actionReset() {
  g_x_center = 0.0;
  g_y_center = 0.0;
  g_x_span = 10.0;
  g_y_span = 8.0;
}

MenuItem kMenu[] = {
    {"Edit y1", actionEdit1},
    {"Toggle y1", actionToggle1},
    {"Edit y2", actionEdit2},
    {"Toggle y2", actionToggle2},
    {"Edit y3", actionEdit3},
    {"Toggle y3", actionToggle3},
    {"Set range", actionSetRange},
    {"Reset view", actionReset},
    {"Graph view", graphView},
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
  M5.Display.print("idk-math-graph");

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

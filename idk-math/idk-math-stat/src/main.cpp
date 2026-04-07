#include <M5Unified.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cctype>

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

bool parseList(const String& text, std::vector<double>& out) {
  out.clear();
  String token = "";
  for (int i = 0; i <= text.length(); ++i) {
    char c = (i == text.length()) ? ',' : text[i];
    if (c == ',' || c == ';' || c == ' ' || c == '\n' || c == '\t') {
      if (token.length() > 0) {
        char* endptr = nullptr;
        double v = strtod(token.c_str(), &endptr);
        if (endptr == token.c_str()) return false;
        out.push_back(v);
        token = "";
      }
    } else {
      token += c;
    }
  }
  return !out.empty();
}

long long nCr(int n, int r) {
  if (r < 0 || r > n) return 0;
  if (r > n - r) r = n - r;
  long long res = 1;
  for (int i = 1; i <= r; ++i) {
    res = res * (n - r + i) / i;
  }
  return res;
}

long long nPr(int n, int r) {
  if (r < 0 || r > n) return 0;
  long long res = 1;
  for (int i = 0; i < r; ++i) res *= (n - i);
  return res;
}

void actionSummary() {
  String text = "";
  if (!keyboardInput(text, "Data list", false, false, g_telex_mode, 180, true)) return;
  std::vector<double> values;
  if (!parseList(text, values)) {
    showResult("Summary", "Bad list");
    return;
  }
  std::sort(values.begin(), values.end());
  double sum = 0.0;
  for (double v : values) sum += v;
  double mean = sum / values.size();
  double var = 0.0;
  for (double v : values) {
    double d = v - mean;
    var += d * d;
  }
  var /= values.size();
  double std = sqrt(var);
  double med = values.size() % 2 == 0
                   ? 0.5 * (values[values.size() / 2 - 1] + values[values.size() / 2])
                   : values[values.size() / 2];
  String out = "n = " + String(values.size()) + "\n";
  out += "mean = " + formatFloat(mean, 6) + "\n";
  out += "median = " + formatFloat(med, 6) + "\n";
  out += "var = " + formatFloat(var, 6) + "\n";
  out += "std = " + formatFloat(std, 6) + "\n";
  out += "min = " + formatFloat(values.front(), 6) + "\n";
  out += "max = " + formatFloat(values.back(), 6);
  showResult("Summary", out);
}

void actionNCr() {
  double n = 0, r = 0;
  if (!inputNumber(n, "n", g_telex_mode)) return;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  long long res = nCr(static_cast<int>(round(n)), static_cast<int>(round(r)));
  showResult("nCr", "C = " + String(res));
}

void actionNPr() {
  double n = 0, r = 0;
  if (!inputNumber(n, "n", g_telex_mode)) return;
  if (!inputNumber(r, "r", g_telex_mode)) return;
  long long res = nPr(static_cast<int>(round(n)), static_cast<int>(round(r)));
  showResult("nPr", "P = " + String(res));
}

MenuItem kMenu[] = {
    {"Summary stats", actionSummary},
    {"nCr", actionNCr},
    {"nPr", actionNPr},
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
  M5.Display.print("idk-math-stat");

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

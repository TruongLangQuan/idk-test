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

long long nCr(int n, int r) {
  if (r < 0 || r > n) return 0;
  if (r > n - r) r = n - r;
  long long res = 1;
  for (int i = 1; i <= r; ++i) res = res * (n - r + i) / i;
  return res;
}

long long nPr(int n, int r) {
  if (r < 0 || r > n) return 0;
  long long res = 1;
  for (int i = 0; i < r; ++i) res *= (n - i);
  return res;
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

void actionBinomial() {
  double n = 0, k = 0, p = 0;
  if (!inputNumber(n, "n", g_telex_mode)) return;
  if (!inputNumber(k, "k", g_telex_mode)) return;
  if (!inputNumber(p, "p", g_telex_mode)) return;
  int ni = static_cast<int>(round(n));
  int ki = static_cast<int>(round(k));
  if (p < 0 || p > 1) {
    showResult("Binomial", "p in [0,1]");
    return;
  }
  long long comb = nCr(ni, ki);
  double prob = comb * pow(p, ki) * pow(1.0 - p, ni - ki);
  showResult("Binomial", "P(X=k) = " + formatFloat(prob, 6));
}

void actionExpectation() {
  double n = 0, p = 0;
  if (!inputNumber(n, "n", g_telex_mode)) return;
  if (!inputNumber(p, "p", g_telex_mode)) return;
  if (p < 0 || p > 1) {
    showResult("Expect", "p in [0,1]");
    return;
  }
  double mean = n * p;
  double var = n * p * (1.0 - p);
  String out = "E = " + formatFloat(mean, 6) + "\n";
  out += "Var = " + formatFloat(var, 6);
  showResult("Expect", out);
}

MenuItem kMenu[] = {
    {"nCr", actionNCr},
    {"nPr", actionNPr},
    {"Binomial P(X=k)", actionBinomial},
    {"Binomial E,Var", actionExpectation},
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
  M5.Display.print("idk-math-prob");

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

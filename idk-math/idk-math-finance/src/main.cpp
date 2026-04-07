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

void actionCompound() {
  double P = 0, r = 0, n = 0, t = 0;
  if (!inputNumber(P, "P", g_telex_mode)) return;
  if (!inputNumber(r, "r (0.1)", g_telex_mode)) return;
  if (!inputNumber(n, "n/yr", g_telex_mode)) return;
  if (!inputNumber(t, "years", g_telex_mode)) return;
  if (n <= 0) {
    showResult("Compound", "n must >0");
    return;
  }
  double A = P * pow(1.0 + r / n, n * t);
  showResult("Compound", "A = " + formatFloat(A, 6));
}

void actionSimple() {
  double P = 0, r = 0, t = 0;
  if (!inputNumber(P, "P", g_telex_mode)) return;
  if (!inputNumber(r, "r (0.1)", g_telex_mode)) return;
  if (!inputNumber(t, "years", g_telex_mode)) return;
  double A = P * (1.0 + r * t);
  showResult("Simple", "A = " + formatFloat(A, 6));
}

void actionLoan() {
  double P = 0, r = 0, years = 0;
  if (!inputNumber(P, "Loan", g_telex_mode)) return;
  if (!inputNumber(r, "rate (0.1)", g_telex_mode)) return;
  if (!inputNumber(years, "years", g_telex_mode)) return;
  int n = static_cast<int>(round(years * 12));
  if (n <= 0) {
    showResult("Loan", "years invalid");
    return;
  }
  double rm = r / 12.0;
  double pmt = 0;
  if (fabs(rm) < 1e-12) {
    pmt = P / n;
  } else {
    pmt = P * rm / (1.0 - pow(1.0 + rm, -n));
  }
  showResult("Loan", "PMT = " + formatFloat(pmt, 6));
}

MenuItem kMenu[] = {
    {"Compound interest", actionCompound},
    {"Simple interest", actionSimple},
    {"Loan payment", actionLoan},
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
  M5.Display.print("idk-math-finance");

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

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

void actionMolarity() {
  double n = 0, V = 0;
  if (!inputNumber(n, "n (mol)", g_telex_mode)) return;
  if (!inputNumber(V, "V (L)", g_telex_mode)) return;
  if (fabs(V) < 1e-12) {
    showResult("Molarity", "V invalid");
    return;
  }
  double M = n / V;
  showResult("Molarity", "M = " + formatFloat(M, 6) + " M");
}

void actionMolesFromM() {
  double M = 0, V = 0;
  if (!inputNumber(M, "M", g_telex_mode)) return;
  if (!inputNumber(V, "V (L)", g_telex_mode)) return;
  double n = M * V;
  showResult("Moles", "n = " + formatFloat(n, 6) + " mol");
}

void actionDilution() {
  double C1 = 0, V1 = 0, C2 = 0, V2 = 0;
  if (!inputNumber(C1, "C1", g_telex_mode)) return;
  if (!inputNumber(V1, "V1", g_telex_mode)) return;
  if (!inputNumber(C2, "C2", g_telex_mode)) return;
  if (!inputNumber(V2, "V2", g_telex_mode)) return;
  double left = C1 * V1;
  double right = C2 * V2;
  String out = "C1V1 = " + formatFloat(left, 6) + "\n";
  out += "C2V2 = " + formatFloat(right, 6);
  showResult("Dilution", out);
}

void actionMassPercent() {
  double solute = 0, solution = 0;
  if (!inputNumber(solute, "solute (g)", g_telex_mode)) return;
  if (!inputNumber(solution, "solution (g)", g_telex_mode)) return;
  if (fabs(solution) < 1e-12) {
    showResult("Percent", "solution invalid");
    return;
  }
  double percent = solute / solution * 100.0;
  showResult("Percent", "%m/m = " + formatFloat(percent, 3));
}

void actionPH() {
  double h = 0;
  if (!inputNumber(h, "[H+]", g_telex_mode)) return;
  if (h <= 0) {
    showResult("pH", "[H+] must be >0");
    return;
  }
  double pH = -log10(h);
  showResult("pH", "pH = " + formatFloat(pH, 3));
}

void actionHFromPH() {
  double pH = 0;
  if (!inputNumber(pH, "pH", g_telex_mode)) return;
  double h = pow(10.0, -pH);
  showResult("[H+]", "[H+] = " + formatFloat(h, 6));
}

MenuItem kMenu[] = {
    {"Molarity (M)", actionMolarity},
    {"Moles from M", actionMolesFromM},
    {"Dilution C1V1=C2V2", actionDilution},
    {"Mass percent", actionMassPercent},
    {"pH from [H+]", actionPH},
    {"[H+] from pH", actionHFromPH},
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
  M5.Display.print("idk-chemistry-solution");

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

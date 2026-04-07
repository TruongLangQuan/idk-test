#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;
constexpr double kAvogadro = 6.02214076e23;
constexpr double kGasR = 0.082057; // L·atm·K−1·mol−1

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

void actionMolesFromMass() {
  double mass = 0, molar = 0;
  if (!inputNumber(mass, "mass (g)", g_telex_mode)) return;
  if (!inputNumber(molar, "M (g/mol)", g_telex_mode)) return;
  if (fabs(molar) < 1e-12) {
    showResult("Moles", "Invalid molar mass");
    return;
  }
  double n = mass / molar;
  showResult("Moles", "n = " + formatFloat(n, 6) + " mol");
}

void actionMassFromMoles() {
  double n = 0, molar = 0;
  if (!inputNumber(n, "n (mol)", g_telex_mode)) return;
  if (!inputNumber(molar, "M (g/mol)", g_telex_mode)) return;
  double mass = n * molar;
  showResult("Mass", "m = " + formatFloat(mass, 6) + " g");
}

void actionParticles() {
  double n = 0;
  if (!inputNumber(n, "n (mol)", g_telex_mode)) return;
  double count = n * kAvogadro;
  showResult("Particles", "N = " + formatFloat(count, 3));
}

void actionIdealGas() {
  double P = 0, V = 0, T = 0;
  if (!inputNumber(P, "P (atm)", g_telex_mode)) return;
  if (!inputNumber(V, "V (L)", g_telex_mode)) return;
  if (!inputNumber(T, "T (K)", g_telex_mode)) return;
  if (T <= 0) {
    showResult("Ideal gas", "T must be > 0");
    return;
  }
  double n = P * V / (kGasR * T);
  showResult("Ideal gas", "n = " + formatFloat(n, 6) + " mol");
}

void actionPercentYield() {
  double actual = 0, theoretical = 0;
  if (!inputNumber(actual, "actual", g_telex_mode)) return;
  if (!inputNumber(theoretical, "theoretical", g_telex_mode)) return;
  if (fabs(theoretical) < 1e-12) {
    showResult("Yield", "Invalid theoretical");
    return;
  }
  double percent = actual / theoretical * 100.0;
  showResult("Yield", "% = " + formatFloat(percent, 3));
}

MenuItem kMenu[] = {
    {"Moles from mass", actionMolesFromMass},
    {"Mass from moles", actionMassFromMoles},
    {"Particles from n", actionParticles},
    {"Ideal gas (PV=nRT)", actionIdealGas},
    {"Percent yield", actionPercentYield},
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
  M5.Display.print("idk-chemistry-stoich");

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

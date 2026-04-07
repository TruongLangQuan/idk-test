#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;
constexpr double kGasR = 0.082057; // L·atm·K−1·mol−1

struct MenuItem { const char* title; void (*action)(); };

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

void actionHeat() {
  double m = 0, c = 0, dt = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (!inputNumber(c, "c", g_telex_mode)) return;
  if (!inputNumber(dt, "dT", g_telex_mode)) return;
  double Q = m * c * dt;
  showResult("Heat", "Q = " + formatFloat(Q, 6));
}

void actionLatent() {
  double m = 0, L = 0;
  if (!inputNumber(m, "m", g_telex_mode)) return;
  if (!inputNumber(L, "L", g_telex_mode)) return;
  double Q = m * L;
  showResult("Latent", "Q = " + formatFloat(Q, 6));
}

void actionIdealGas() {
  double P = 0, V = 0, T = 0;
  if (!inputNumber(P, "P (atm)", g_telex_mode)) return;
  if (!inputNumber(V, "V (L)", g_telex_mode)) return;
  if (!inputNumber(T, "T (K)", g_telex_mode)) return;
  if (T <= 0) {
    showResult("Ideal gas", "T must >0");
    return;
  }
  double n = P * V / (kGasR * T);
  showResult("Ideal gas", "n = " + formatFloat(n, 6));
}

void actionEfficiency() {
  double W = 0, Qh = 0;
  if (!inputNumber(W, "W", g_telex_mode)) return;
  if (!inputNumber(Qh, "Qh", g_telex_mode)) return;
  if (fabs(Qh) < 1e-12) {
    showResult("Efficiency", "Qh invalid");
    return;
  }
  double e = W / Qh * 100.0;
  showResult("Efficiency", "eta = " + formatFloat(e, 3) + "%");
}

MenuItem kMenu[] = {
    {"Q=mc dT", actionHeat},
    {"Q=mL", actionLatent},
    {"PV=nRT", actionIdealGas},
    {"Efficiency", actionEfficiency},
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
  M5.Display.print("idk-physics-thermo");

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

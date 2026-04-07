#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;

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

void actionPhotonEnergy() {
  double lambda_nm = 0;
  if (!inputNumber(lambda_nm, "Lambda (nm)", g_telex_mode)) return;
  if (lambda_nm <= 0) {
    showResult("Photon", "Lambda must be > 0");
    return;
  }
  double e_ev = 1240.0 / lambda_nm;
  double e_j = e_ev * 1.602176634e-19;
  String out = "E = " + formatFloat(e_ev, 6) + " eV\n" +
               "E = " + formatFloat(e_j, 9) + " J";
  showResult("Photon", out);
}

void actionPhotoelectric() {
  double f = 0;
  double phi = 0;
  if (!inputNumber(f, "f (Hz)", g_telex_mode)) return;
  if (!inputNumber(phi, "Work fn (eV)", g_telex_mode)) return;
  if (f <= 0) {
    showResult("Photoelectric", "f must be > 0");
    return;
  }
  const double h_ev = 4.135667696e-15;  // eV*s
  double e = h_ev * f;
  double k = e - phi;
  if (k <= 0) {
    showResult("Photoelectric", "No emission (E < phi)");
    return;
  }
  String out = "E = " + formatFloat(e, 6) + " eV\n" +
               "Kmax = " + formatFloat(k, 6) + " eV";
  showResult("Photoelectric", out);
}

void actionDeBroglie() {
  double p = 0;
  if (!inputNumber(p, "p (kg m/s)", g_telex_mode)) return;
  if (p <= 0) {
    showResult("De Broglie", "p must be > 0");
    return;
  }
  const double h = 6.62607015e-34;
  double lambda = h / p;
  showResult("De Broglie", "lambda = " + formatFloat(lambda, 12) + " m");
}

void actionRelativity() {
  double v = 0, t = 0;
  if (!inputNumber(v, "v (m/s)", g_telex_mode)) return;
  if (!inputNumber(t, "t (s)", g_telex_mode)) return;
  const double c = 299792458.0;
  if (v <= 0 || v >= c) {
    showResult("Relativity", "0 < v < c");
    return;
  }
  double beta2 = (v * v) / (c * c);
  double gamma = 1.0 / sqrt(1.0 - beta2);
  double tp = t * gamma;
  String out = "gamma = " + formatFloat(gamma, 6) + "\n" +
               "t' = " + formatFloat(tp, 6) + " s";
  showResult("Relativity", out);
}

void actionMassEnergy() {
  double m = 0;
  if (!inputNumber(m, "m (kg)", g_telex_mode)) return;
  if (m < 0) {
    showResult("E=mc^2", "m must be >= 0");
    return;
  }
  const double c = 299792458.0;
  double e = m * c * c;
  showResult("E=mc^2", "E = " + formatFloat(e, 6) + " J");
}

MenuItem kMenu[] = {
    {"Photon energy", actionPhotonEnergy},
    {"Photoelectric", actionPhotoelectric},
    {"De Broglie", actionDeBroglie},
    {"Relativity", actionRelativity},
    {"E = mc^2", actionMassEnergy},
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
  M5.Display.print("idk-physics-modern");

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

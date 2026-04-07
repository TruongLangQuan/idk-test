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

void actionEcell() {
  double ecath = 0, ean = 0;
  if (!inputNumber(ecath, "E cathode", g_telex_mode)) return;
  if (!inputNumber(ean, "E anode", g_telex_mode)) return;
  double e = ecath - ean;
  showResult("Ecell", "Ecell = " + formatFloat(e, 6) + " V");
}

void actionDeltaG() {
  double n = 0, e = 0;
  if (!inputNumber(n, "n e-", g_telex_mode)) return;
  if (!inputNumber(e, "Ecell (V)", g_telex_mode)) return;
  const double F = 96485.33212;
  double g = -n * F * e / 1000.0;
  showResult("DeltaG", "DeltaG = " + formatFloat(g, 6) + " kJ");
}

void actionNernst() {
  double e0 = 0, n = 0, q = 0;
  if (!inputNumber(e0, "E0 (V)", g_telex_mode)) return;
  if (!inputNumber(n, "n e-", g_telex_mode)) return;
  if (!inputNumber(q, "Q", g_telex_mode)) return;
  if (n <= 0 || q <= 0) {
    showResult("Nernst", "n>0, Q>0");
    return;
  }
  double e = e0 - (0.05916 / n) * log10(q);
  showResult("Nernst", "E = " + formatFloat(e, 6) + " V");
}

void actionOxRules() {
  String text =
      "Oxidation number rules:\n"
      "- Free element: 0\n"
      "- Monoatomic ion: charge\n"
      "- Group 1: +1, Group 2: +2\n"
      "- F: -1; O: -2 (peroxide -1)\n"
      "- H: +1 (metal hydride -1)\n"
      "- Sum in neutral = 0";
  showResult("Rules", text);
}

MenuItem kMenu[] = {
    {"Ecell = Ecath - Ean", actionEcell},
    {"DeltaG = -nFE", actionDeltaG},
    {"Nernst equation", actionNernst},
    {"Oxidation rules", actionOxRules},
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
  M5.Display.print("idk-chemistry-redox");

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

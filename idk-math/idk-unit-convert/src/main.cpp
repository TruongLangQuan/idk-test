#include <M5Unified.h>
#include <cmath>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;
constexpr double kAtmToPa = 101325.0;

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

void actionMetersToKm() {
  double v = 0;
  if (!inputNumber(v, "m", g_telex_mode)) return;
  showResult("m->km", "km = " + formatFloat(v / 1000.0, 6));
}

void actionKmToM() {
  double v = 0;
  if (!inputNumber(v, "km", g_telex_mode)) return;
  showResult("km->m", "m = " + formatFloat(v * 1000.0, 6));
}

void actionKgToG() {
  double v = 0;
  if (!inputNumber(v, "kg", g_telex_mode)) return;
  showResult("kg->g", "g = " + formatFloat(v * 1000.0, 6));
}

void actionGToKg() {
  double v = 0;
  if (!inputNumber(v, "g", g_telex_mode)) return;
  showResult("g->kg", "kg = " + formatFloat(v / 1000.0, 6));
}

void actionCToF() {
  double v = 0;
  if (!inputNumber(v, "C", g_telex_mode)) return;
  showResult("C->F", "F = " + formatFloat(v * 9.0 / 5.0 + 32.0, 3));
}

void actionFToC() {
  double v = 0;
  if (!inputNumber(v, "F", g_telex_mode)) return;
  showResult("F->C", "C = " + formatFloat((v - 32.0) * 5.0 / 9.0, 3));
}

void actionAtmToPa() {
  double v = 0;
  if (!inputNumber(v, "atm", g_telex_mode)) return;
  showResult("atm->Pa", "Pa = " + formatFloat(v * kAtmToPa, 2));
}

void actionPaToAtm() {
  double v = 0;
  if (!inputNumber(v, "Pa", g_telex_mode)) return;
  showResult("Pa->atm", "atm = " + formatFloat(v / kAtmToPa, 6));
}

void actionSpeedToMs() {
  double v = 0;
  if (!inputNumber(v, "km/h", g_telex_mode)) return;
  showResult("km/h->m/s", "m/s = " + formatFloat(v * 1000.0 / 3600.0, 6));
}

void actionSpeedToKmh() {
  double v = 0;
  if (!inputNumber(v, "m/s", g_telex_mode)) return;
  showResult("m/s->km/h", "km/h = " + formatFloat(v * 3.6, 6));
}

MenuItem kMenu[] = {
    {"m -> km", actionMetersToKm},
    {"km -> m", actionKmToM},
    {"kg -> g", actionKgToG},
    {"g -> kg", actionGToKg},
    {"C -> F", actionCToF},
    {"F -> C", actionFToC},
    {"atm -> Pa", actionAtmToPa},
    {"Pa -> atm", actionPaToAtm},
    {"km/h -> m/s", actionSpeedToMs},
    {"m/s -> km/h", actionSpeedToKmh},
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
  M5.Display.print("idk-unit-convert");

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

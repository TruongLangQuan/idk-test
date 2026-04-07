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

void actionCardiacOutput() {
  double hr = 0, sv = 0;
  if (!inputNumber(hr, "HR (bpm)", g_telex_mode)) return;
  if (!inputNumber(sv, "SV (mL)", g_telex_mode)) return;
  double co = hr * sv / 1000.0;
  showResult("Cardiac output", "CO = " + formatFloat(co, 3) + " L/min");
}

void actionMAP() {
  double sbp = 0, dbp = 0;
  if (!inputNumber(sbp, "SBP", g_telex_mode)) return;
  if (!inputNumber(dbp, "DBP", g_telex_mode)) return;
  double map = (sbp + 2 * dbp) / 3.0;
  showResult("MAP", "MAP = " + formatFloat(map, 2));
}

void actionMinuteVent() {
  double tv = 0, rr = 0;
  if (!inputNumber(tv, "Tidal V (mL)", g_telex_mode)) return;
  if (!inputNumber(rr, "RR (breaths)", g_telex_mode)) return;
  double ve = tv * rr / 1000.0;
  showResult("Ventilation", "VE = " + formatFloat(ve, 3) + " L/min");
}

void actionGlucose() {
  double mg = 0;
  if (!inputNumber(mg, "mg/dL", g_telex_mode)) return;
  double mmol = mg / 18.0;
  showResult("Glucose", "mmol/L = " + formatFloat(mmol, 3));
}

void actionOsmolarity() {
  double na = 0, glu = 0, bun = 0;
  if (!inputNumber(na, "Na (mmol)", g_telex_mode)) return;
  if (!inputNumber(glu, "Glu (mg/dL)", g_telex_mode)) return;
  if (!inputNumber(bun, "BUN (mg/dL)", g_telex_mode)) return;
  double osm = 2 * na + glu / 18.0 + bun / 2.8;
  showResult("Osm", "Osm = " + formatFloat(osm, 2) + " mOsm/kg");
}

MenuItem kMenu[] = {
    {"Cardiac output", actionCardiacOutput},
    {"MAP (BP)", actionMAP},
    {"Minute ventilation", actionMinuteVent},
    {"Glucose mg/dL->mmol", actionGlucose},
    {"Plasma osm", actionOsmolarity},
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
  M5.Display.print("idk-biology-physiology");

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

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

void actionBMI() {
  double w = 0, h = 0;
  if (!inputNumber(w, "Weight (kg)", g_telex_mode)) return;
  if (!inputNumber(h, "Height (m)", g_telex_mode)) return;
  if (h <= 0) {
    showResult("BMI", "Height > 0");
    return;
  }
  double bmi = w / (h * h);
  showResult("BMI", "BMI = " + formatFloat(bmi, 2));
}

void actionHeartRate() {
  double age = 0;
  if (!inputNumber(age, "Age", g_telex_mode)) return;
  double maxhr = 220.0 - age;
  double low = maxhr * 0.6;
  double high = maxhr * 0.8;
  String out = "Max HR = " + formatFloat(maxhr, 1) + "\n" +
               "Target 60-80%: " + formatFloat(low, 1) + "-" + formatFloat(high, 1);
  showResult("Heart rate", out);
}

void actionOrgans() {
  String text =
      "Major organs:\n"
      "- Brain: control\n"
      "- Heart: blood pump\n"
      "- Lungs: gas exchange\n"
      "- Liver: metabolism\n"
      "- Kidneys: filtration\n"
      "- Stomach: digestion\n"
      "- Intestine: absorption";
  showResult("Organs", text);
}

void actionSkeleton() {
  String text =
      "Skeletal basics:\n"
      "- Human bones: ~206\n"
      "- Axial + appendicular\n"
      "- Functions: support,\n"
      "  protection, movement";
  showResult("Skeleton", text);
}

MenuItem kMenu[] = {
    {"BMI", actionBMI},
    {"Heart rate", actionHeartRate},
    {"Organ functions", actionOrgans},
    {"Skeleton notes", actionSkeleton},
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
  M5.Display.print("idk-biology-anatomy");

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

#include <M5Unified.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include <idk_ui.h>

namespace {

constexpr int kMenuLines = 6;
static const int kSdCsPin = 14;
static const int kSdSckPin = 0;
static const int kSdMisoPin = 36;
static const int kSdMosiPin = 26;
static const char* kNotesPath = "/notes.txt";

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

bool g_sd_ready = false;

bool ensureSdReady() {
  if (g_sd_ready) return true;
  SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
  g_sd_ready = SD.begin(kSdCsPin, SPI);
  return g_sd_ready;
}

void actionAddNote() {
  String text = "";
  if (!keyboardInput(text, "New note", false, true, g_telex_mode, 200, true)) return;
  if (!ensureSdReady()) {
    showResult("Notes", "SD not ready");
    return;
  }
  File f = SD.open(kNotesPath, FILE_APPEND);
  if (!f) {
    showResult("Notes", "Open failed");
    return;
  }
  f.println(text);
  f.close();
  showResult("Notes", "Saved");
}

void actionViewNotes() {
  if (!ensureSdReady()) {
    showResult("Notes", "SD not ready");
    return;
  }
  File f = SD.open(kNotesPath, FILE_READ);
  if (!f) {
    showResult("Notes", "No notes");
    return;
  }
  String out = "";
  while (f.available() && out.length() < 500) {
    out += static_cast<char>(f.read());
  }
  f.close();
  if (out.isEmpty()) out = "(empty)";
  showResult("Notes", out);
}

void actionClearNotes() {
  if (!ensureSdReady()) {
    showResult("Notes", "SD not ready");
    return;
  }
  SD.remove(kNotesPath);
  showResult("Notes", "Cleared");
}

MenuItem kMenu[] = {
    {"Add note", actionAddNote},
    {"View notes", actionViewNotes},
    {"Clear notes", actionClearNotes},
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
  M5.Display.print("idk-notes");

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

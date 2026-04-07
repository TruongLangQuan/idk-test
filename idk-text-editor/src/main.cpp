#include <M5Unified.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;
constexpr int kMaxFiles = 32;

// SD pins aligned with idk-firmware-idk m5stickc plus2
static const int kSdCsPin = 14;
static const int kSdSckPin = 0;
static const int kSdMisoPin = 36;
static const int kSdMosiPin = 26;

struct MenuItem {
  const char* title;
  void (*action)();
};

struct FileEntry {
  String name;
  String path;
};

enum class AppState {
  FILE_LIST,
  FILE_VIEW,
};

bool g_telex_mode = false;
String g_status = "Ready";
AppState g_state = AppState::FILE_LIST;

MemoryFontWrapper g_font_wrap(vi12_font, vi12_font_len);

constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

int g_menu_idx = 0;
int g_menu_scroll = 0;
int g_view_idx = 0;
int g_view_scroll = 0;

FileEntry g_files[kMaxFiles];
int g_file_count = 0;
int g_file_idx = 0; // includes pseudo "new file" at index 0

String g_current_path = "";
String g_content = "";

bool g_sd_ready = false;

bool ensureSdReady() {
  if (g_sd_ready) return true;
  SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
  g_sd_ready = SD.begin(kSdCsPin, SPI);
  return g_sd_ready;
}

bool isTxtFile(const String& name) {
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".txt");
}

String ensureTxtExtension(const String& name) {
  if (isTxtFile(name)) return name;
  return name + ".txt";
}

void scanFiles() {
  g_file_count = 0;
  g_file_idx = 0;
  if (!ensureSdReady()) {
    g_status = "SD mount failed";
    return;
  }
  File root = SD.open("/");
  if (!root) {
    g_status = "Open root failed";
    return;
  }
  while (true) {
    File f = root.openNextFile();
    if (!f) break;
    if (!f.isDirectory()) {
      String name = f.name();
      if (isTxtFile(name) && g_file_count < kMaxFiles) {
        g_files[g_file_count++] = {name, String("/") + name};
      }
    }
    f.close();
  }
  root.close();
}

void loadContent() {
  g_content = "";
  if (!ensureSdReady()) return;
  if (g_current_path.isEmpty()) return;
  File f = SD.open(g_current_path.c_str(), FILE_READ);
  if (!f) {
    g_content = "(empty)";
    return;
  }
  while (f.available() && g_content.length() < 1024) {
    g_content += static_cast<char>(f.read());
  }
  f.close();
  if (g_content.isEmpty()) g_content = "(empty)";
}

void drawFileList() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-text-editor");
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 14);
  M5.Display.print("A:open B:next PWR:prev");

  int total = g_file_count + 1; // include new file
  int start = g_file_idx - 3;
  if (start < 0) start = 0;
  int end = start + 7;
  if (end > total) end = total;

  int y = 30;
  for (int i = start; i < end; ++i) {
    bool sel = (i == g_file_idx);
    M5.Display.setTextColor(sel ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(4, y);
    M5.Display.print(sel ? "> " : "  ");
    if (i == 0) {
      M5.Display.print("[+] New file");
    } else {
      M5.Display.print(g_files[i - 1].name);
    }
    y += 12;
  }

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 124);
  M5.Display.print(g_status);
}

void drawViewMenu() {
  const char* items[] = {"Append line", "Overwrite all", "Save copy", "Delete file", "Back"};
  int count = 5;
  if (g_view_idx < g_view_scroll) g_view_scroll = g_view_idx;
  if (g_view_idx >= g_view_scroll + kMenuLines) g_view_scroll = g_view_idx - kMenuLines + 1;

  int y = 78;
  for (int i = 0; i < kMenuLines; ++i) {
    int idx = g_view_scroll + i;
    if (idx >= count) break;
    bool sel = (idx == g_view_idx);
    M5.Display.setTextColor(sel ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(4, y);
    M5.Display.print(sel ? "> " : "  ");
    M5.Display.print(items[idx]);
    y += 12;
  }
}

void drawFileView() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print(g_current_path);

  String preview = g_content;
  if (preview.length() > 280) preview = preview.substring(0, 280) + "...";
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  drawWrapped(preview, 2, 14, 236, 4);

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 64);
  M5.Display.print("A:select B:next PWR:prev");

  drawViewMenu();
}

void enterView() {
  g_view_idx = 0;
  g_view_scroll = 0;
  g_state = AppState::FILE_VIEW;
  drawFileView();
}

void actionAppend() {
  String line = "";
  if (!keyboardInput(line, "Append", false, true, g_telex_mode, 200, true)) return;
  if (!ensureSdReady()) return;
  File f = SD.open(g_current_path.c_str(), FILE_APPEND);
  if (!f) {
    g_status = "Append failed";
    return;
  }
  f.print(line);
  f.print("\n");
  f.close();
  loadContent();
}

void actionOverwrite() {
  String text = "";
  if (!keyboardInput(text, "Overwrite", false, true, g_telex_mode, 200, true)) return;
  if (!ensureSdReady()) return;
  File f = SD.open(g_current_path.c_str(), FILE_WRITE);
  if (!f) {
    g_status = "Write failed";
    return;
  }
  f.print(text);
  f.print("\n");
  f.close();
  loadContent();
}

void actionSaveCopy() {
  String name = "";
  if (!keyboardInput(name, "Copy name", false, true, g_telex_mode, 50, true)) return;
  name.trim();
  if (name.isEmpty()) return;
  name = ensureTxtExtension(name);
  String path = String("/") + name;
  if (!ensureSdReady()) return;
  File f = SD.open(path.c_str(), FILE_WRITE);
  if (!f) {
    g_status = "Save failed";
    return;
  }
  f.print(g_content);
  f.close();
  g_status = "Saved copy";
  scanFiles();
}

void actionDelete() {
  if (!ensureSdReady()) return;
  if (g_current_path.isEmpty()) return;
  SD.remove(g_current_path.c_str());
  g_status = "Deleted";
  scanFiles();
  g_state = AppState::FILE_LIST;
}

void openSelection() {
  if (g_file_idx == 0) {
    String name = "";
    if (!keyboardInput(name, "Filename", false, true, g_telex_mode, 50, true)) return;
    name.trim();
    if (name.isEmpty()) return;
    name = ensureTxtExtension(name);
    g_current_path = String("/") + name;
    if (ensureSdReady()) {
      File f = SD.open(g_current_path.c_str(), FILE_WRITE);
      if (f) f.close();
    }
  } else {
    g_current_path = g_files[g_file_idx - 1].path;
  }
  loadContent();
  enterView();
}

void handleViewAction() {
  if (g_view_idx == 0) actionAppend();
  if (g_view_idx == 1) actionOverwrite();
  if (g_view_idx == 2) actionSaveCopy();
  if (g_view_idx == 3) actionDelete();
  if (g_view_idx == 4) g_state = AppState::FILE_LIST;
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  (void)loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);
  autoConnectKnownWifi(kKnownWifis, sizeof(kKnownWifis) / sizeof(kKnownWifis[0]), g_status);
  scanFiles();
  drawFileList();
}

void loop() {
  M5.update();

  if (g_state == AppState::FILE_LIST) {
    int total = g_file_count + 1;
    if (M5.BtnB.wasPressed()) {
      g_file_idx = (g_file_idx + 1) % total;
      drawFileList();
    }
    if (M5.BtnPWR.wasPressed()) {
      g_file_idx = (g_file_idx - 1 + total) % total;
      drawFileList();
    }
    if (M5.BtnA.wasPressed()) {
      openSelection();
    }
  } else {
    if (M5.BtnB.wasPressed()) {
      g_view_idx = (g_view_idx + 1) % 5;
      drawFileView();
    }
    if (M5.BtnPWR.wasPressed()) {
      g_view_idx = (g_view_idx - 1 + 5) % 5;
      drawFileView();
    }
    if (M5.BtnA.wasPressed()) {
      handleViewAction();
      if (g_state == AppState::FILE_LIST) {
        drawFileList();
      } else {
        drawFileView();
      }
    }
  }
  delay(10);
}

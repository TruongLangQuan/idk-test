#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>

#ifndef TFT_I2C_POWER
#define TFT_I2C_POWER 21
#endif

namespace {

enum class UiMode {
  Browser,
  Viewer,
};

struct ButtonState {
  const char* name;
  int pin;
  bool pressed;
};

struct FsEntry {
  String name;
  bool isDir;
  bool isParent;
};

// microSD module wiring on free Tenstar GPIOs
static constexpr int kSdCs = 13;
static constexpr int kSdMosi = 11;
static constexpr int kSdSck = 12;
static constexpr int kSdMiso = 10;

// 5-way tactile switch wiring on free GPIOs
static constexpr int kBtnRight = 18;
static constexpr int kBtnDown = 17;
static constexpr int kBtnLeft = 16;
static constexpr int kBtnCenter = 15;
static constexpr int kBtnUp = 14;

static constexpr int kNeoPixelData = 33;
static constexpr int kNeoPixelPower = 34;
static constexpr int kBatteryAdcPin = 3;

static constexpr uint32_t kSdRetryMs = 4000;
static constexpr uint32_t kBatteryRefreshMs = 2000;
static constexpr uint16_t kBg = TFT_BLACK;
static constexpr uint16_t kFg = TFT_WHITE;
static constexpr uint16_t kAccent = TFT_CYAN;
static constexpr uint16_t kOk = TFT_GREEN;
static constexpr uint16_t kWarn = TFT_YELLOW;
static constexpr uint16_t kErr = TFT_RED;
static constexpr size_t kVisibleRows = 8;
static constexpr size_t kViewerLinesPerPage = 8;
static constexpr size_t kMaxBrowserEntries = 64;

ButtonState g_buttons[] = {
    {"RIGHT", kBtnRight, false},
    {"DOWN", kBtnDown, false},
    {"LEFT", kBtnLeft, false},
    {"CENTER", kBtnCenter, false},
    {"UP", kBtnUp, false},
};

TFT_eSPI tft = TFT_eSPI();
SPIClass sdSpi(FSPI);

bool g_sdReady = false;
String g_sdStatus = "Mounting microSD...";
String g_lastButton = "None";
uint64_t g_sdSizeMb = 0;
uint32_t g_lastSdCheckMs = 0;
uint32_t g_lastUiMs = 0;
uint32_t g_lastBatteryMs = 0;
float g_batteryVoltage = 0.0f;
int g_batteryPercent = 0;
String g_batteryStatus = "BAT ?";

UiMode g_uiMode = UiMode::Browser;
FsEntry g_browserEntries[kMaxBrowserEntries];
size_t g_browserEntryCount = 0;
size_t g_browserCursor = 0;
size_t g_browserScroll = 0;
String g_browserPath = "/";

String g_viewerPath = "/";
String g_viewerTitle = "";
String g_viewerStatus = "Select file";
String g_viewerLines[kViewerLinesPerPage];
size_t g_viewerLineCount = 0;
size_t g_viewerPage = 0;
bool g_viewerHasNext = false;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void enableDisplayPower() {
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
}

void enableBacklight() {
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
#ifdef TFT_BACKLIGHT_ON
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#else
  digitalWrite(TFT_BL, HIGH);
#endif
#endif
}

void disableOnboardLed() {
  pinMode(kNeoPixelPower, OUTPUT);
  digitalWrite(kNeoPixelPower, LOW);
  pinMode(kNeoPixelData, OUTPUT);
  digitalWrite(kNeoPixelData, LOW);
}

void setupBatteryAdc() {
  pinMode(kBatteryAdcPin, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void updateBatteryStatus(bool force) {
  const uint32_t now = millis();
  if (!force && now - g_lastBatteryMs < kBatteryRefreshMs) return;
  g_lastBatteryMs = now;

  // ASSUMPTION copied from idk-bat for Tenstar TS-ESP32-S3:
  // ADC pin 3, ESP32-S3 Vref ~= 4.3V, battery divider ~= 2:1.
  const uint16_t rawValue = analogRead(kBatteryAdcPin);
  constexpr float kVref = 4.3f;
  constexpr float kDivider = 2.0f;
  g_batteryVoltage = (static_cast<float>(rawValue) * kVref / 4095.0f) * kDivider;

  float percent = ((g_batteryVoltage - 3.0f) / (4.2f - 3.0f)) * 100.0f;
  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;
  g_batteryPercent = static_cast<int>(percent + 0.5f);

  if (g_batteryVoltage < 3.0f) {
    g_batteryStatus = "BAT CRIT";
  } else if (g_batteryVoltage < 3.3f) {
    g_batteryStatus = "BAT LOW";
  } else if (g_batteryVoltage > 4.2f) {
    g_batteryStatus = "BAT FULL";
  } else {
    g_batteryStatus = "BAT OK";
  }
}

String parentPathOf(const String& path) {
  if (path.length() <= 1) return "/";
  const int slash = path.lastIndexOf('/');
  if (slash <= 0) return "/";
  return path.substring(0, slash);
}

String joinPath(const String& base, const String& name) {
  if (base == "/") return "/" + name;
  return base + "/" + name;
}

bool hasReadableExtension(const String& path) {
  String lower = path;
  lower.toLowerCase();
  const char* exts[] = {
      ".txt", ".log", ".md",  ".ini", ".cfg", ".json", ".csv",
      ".h",   ".hpp", ".c",   ".cpp", ".py",  ".js",   ".css",
      ".htm", ".html",".xml", ".yaml",".yml", ".srt",
  };
  for (const char* ext : exts) {
    if (lower.endsWith(ext)) return true;
  }
  return false;
}

bool fileLooksText(File& file) {
  uint8_t probe[96];
  const size_t n = file.read(probe, sizeof(probe));
  file.seek(0);
  if (n == 0) return true;

  size_t bad = 0;
  for (size_t i = 0; i < n; ++i) {
    const uint8_t ch = probe[i];
    const bool ok = ch == '\n' || ch == '\r' || ch == '\t' || (ch >= 32 && ch < 127) || ch >= 160;
    if (!ok) ++bad;
  }
  return bad <= 4;
}

void setupButtons() {
  for (auto& button : g_buttons) {
    pinMode(button.pin, INPUT_PULLUP);
    button.pressed = readPressed(button.pin);
  }
}

void loadBrowserDirectory(const String& path) {
  g_browserPath = path.length() ? path : "/";
  g_browserEntryCount = 0;
  g_browserCursor = 0;
  g_browserScroll = 0;

  if (g_browserPath != "/" && g_browserEntryCount < kMaxBrowserEntries) {
    g_browserEntries[g_browserEntryCount++] = {"..", true, true};
  }

  File dir = SD.open(g_browserPath.c_str());
  if (!dir || !dir.isDirectory()) {
    g_sdStatus = "Cannot open dir";
    if (dir) dir.close();
    return;
  }

  while (g_browserEntryCount < kMaxBrowserEntries) {
    File entry = dir.openNextFile();
    if (!entry) break;
    String name = entry.name();
    const int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.substring(slash + 1);
    if (name.length() == 0) {
      entry.close();
      continue;
    }
    g_browserEntries[g_browserEntryCount++] = {name, entry.isDirectory(), false};
    entry.close();
  }
  dir.close();

  g_uiMode = UiMode::Browser;
  g_sdStatus = (g_browserEntryCount == 0) ? "Mounted, dir empty" : (String("Path: ") + g_browserPath);
}

bool loadViewerPage(const String& path, size_t page) {
  for (auto& line : g_viewerLines) line = "";
  g_viewerLineCount = 0;
  g_viewerHasNext = false;

  File file = SD.open(path.c_str(), FILE_READ);
  if (!file || file.isDirectory()) {
    g_viewerStatus = "Cannot open file";
    if (file) file.close();
    return false;
  }

  const size_t startLine = page * kViewerLinesPerPage;
  size_t lineIndex = 0;
  String current = "";

  while (file.available()) {
    const char ch = static_cast<char>(file.read());
    if (ch == '\r') continue;
    if (ch == '\n') {
      if (lineIndex >= startLine && g_viewerLineCount < kViewerLinesPerPage) {
        g_viewerLines[g_viewerLineCount++] = current;
      }
      ++lineIndex;
      current = "";
      if (g_viewerLineCount >= kViewerLinesPerPage) break;
      continue;
    }
    if (lineIndex >= startLine && current.length() < 44) {
      current += ch;
    }
  }

  if ((current.length() > 0 || lineIndex == 0) && g_viewerLineCount < kViewerLinesPerPage && lineIndex >= startLine) {
    g_viewerLines[g_viewerLineCount++] = current;
  }

  if (file.available()) g_viewerHasNext = true;
  file.close();

  if (g_viewerLineCount == 0 && page > 0) {
    g_viewerStatus = "End of file";
    return false;
  }

  g_uiMode = UiMode::Viewer;
  g_viewerPage = page;
  g_viewerStatus = String("Page ") + (g_viewerPage + 1);
  return true;
}

void openReadableFile(const String& path) {
  File file = SD.open(path.c_str(), FILE_READ);
  if (!file || file.isDirectory()) {
    g_sdStatus = "Cannot open file";
    if (file) file.close();
    return;
  }

  const bool extReadable = hasReadableExtension(path);
  const bool contentReadable = fileLooksText(file);
  const uint32_t fileSize = file.size();
  file.close();

  if (!extReadable && !contentReadable) {
    g_sdStatus = "Unsupported binary file";
    return;
  }

  g_viewerPath = path;
  g_viewerTitle = path;
  const int slash = g_viewerTitle.lastIndexOf('/');
  if (slash >= 0) g_viewerTitle = g_viewerTitle.substring(slash + 1);
  g_sdStatus = String("Open: ") + g_viewerTitle + " (" + fileSize + "B)";
  loadViewerPage(g_viewerPath, 0);
}

void mountSd(bool force) {
  const uint32_t now = millis();
  if (!force && g_sdReady) return;
  if (!force && now - g_lastSdCheckMs < kSdRetryMs) return;
  g_lastSdCheckMs = now;

  g_sdReady = false;
  SD.end();
  sdSpi.end();
  sdSpi.begin(kSdSck, kSdMiso, kSdMosi, kSdCs);

  if (!SD.begin(kSdCs, sdSpi)) {
    g_sdStatus = "Check card or wiring";
    return;
  }

  if (SD.cardType() == CARD_NONE) {
    g_sdStatus = "No card present";
    SD.end();
    return;
  }

  g_sdReady = true;
  g_sdSizeMb = SD.cardSize() / (1024ULL * 1024ULL);
  loadBrowserDirectory("/");
}

void moveCursor(int delta) {
  if (g_browserEntryCount == 0) return;
  const size_t count = g_browserEntryCount;
  if (delta > 0) {
    g_browserCursor = (g_browserCursor + 1) % count;
  } else {
    g_browserCursor = (g_browserCursor + count - 1) % count;
  }
}

void openSelection() {
  if (!g_sdReady) {
    mountSd(true);
    return;
  }

  if (g_uiMode == UiMode::Viewer) {
    g_uiMode = UiMode::Browser;
    return;
  }

  if (g_browserEntryCount == 0) return;
  const auto& entry = g_browserEntries[g_browserCursor];
  if (entry.isParent) {
    loadBrowserDirectory(parentPathOf(g_browserPath));
    return;
  }

  const String fullPath = joinPath(g_browserPath, entry.name);
  if (entry.isDir) {
    loadBrowserDirectory(fullPath);
  } else {
    openReadableFile(fullPath);
  }
}

void goBack() {
  if (!g_sdReady) return;
  if (g_uiMode == UiMode::Viewer) {
    g_uiMode = UiMode::Browser;
    return;
  }
  if (g_browserPath != "/") {
    loadBrowserDirectory(parentPathOf(g_browserPath));
  }
}

void goHome() {
  if (!g_sdReady) return;
  loadBrowserDirectory("/");
}

void drawHeader(const char* title) {
  tft.fillScreen(kBg);
  tft.drawRoundRect(2, 2, tft.width() - 4, tft.height() - 4, 4, kAccent);
  tft.setTextColor(kAccent, kBg);
  tft.setCursor(8, 8);
  tft.print(title);
  tft.setCursor(126, 8);
  tft.printf("%d%%", g_batteryPercent);
  tft.setCursor(160, 8);
  tft.print(g_batteryStatus);
}

void drawFooter(const char* text) {
  const int y = 218;
  tft.fillRect(4, y, tft.width() - 8, 18, kBg);
  tft.drawFastHLine(6, y, tft.width() - 12, kAccent);
  tft.setTextColor(kAccent, kBg);
  tft.setCursor(8, y + 4);
  tft.print(text);
}

void drawBrowserScreen() {
  drawHeader("IDK-TS-TEST");

  tft.setTextColor(g_sdReady ? kOk : kWarn, kBg);
  tft.setCursor(8, 24);
  tft.print(g_sdReady ? "microSD browser" : "microSD not mounted");

  tft.setTextColor(kFg, kBg);
  tft.setCursor(8, 38);
  tft.printf("%s  %.2fV", g_browserPath.c_str(), g_batteryVoltage);

  if (!g_sdReady) {
    tft.setCursor(8, 56);
    tft.print(g_sdStatus);
    tft.setCursor(8, 74);
    tft.print("CENTER or RIGHT: remount");
    drawFooter("5WAY move  CTR open  RT retry");
    return;
  }

  if (g_browserCursor < g_browserScroll) g_browserScroll = g_browserCursor;
  if (g_browserCursor >= g_browserScroll + kVisibleRows) {
    g_browserScroll = g_browserCursor - kVisibleRows + 1;
  }

  const size_t end = (g_browserScroll + kVisibleRows < g_browserEntryCount) ? (g_browserScroll + kVisibleRows)
                                                                             : g_browserEntryCount;
  for (size_t i = g_browserScroll; i < end; ++i) {
    String label = g_browserEntries[i].name;
    if (g_browserEntries[i].isDir && !g_browserEntries[i].isParent) label += "/";
    if (label.length() > 24) label = label.substring(0, 21) + "...";
    tft.setTextColor(i == g_browserCursor ? TFT_YELLOW : TFT_WHITE, kBg);
    tft.setCursor(8, 56 + static_cast<int>(i - g_browserScroll) * 18);
    tft.printf("%c %s", i == g_browserCursor ? '>' : ' ', label.c_str());
  }

  tft.setTextColor(kFg, kBg);
  tft.fillRect(4, 202, tft.width() - 8, 12, kBg);
  tft.setCursor(8, 202);
  tft.print(g_sdStatus);
  drawFooter("UP/DN move LT back RT/CT open");
}

void drawViewerScreen() {
  drawHeader("FILE VIEW");
  tft.setTextColor(kAccent, kBg);
  tft.setCursor(8, 24);
  tft.print(g_viewerTitle);

  tft.setTextColor(kFg, kBg);
  tft.setCursor(8, 38);
  tft.printf("%s  %.2fV", g_viewerStatus.c_str(), g_batteryVoltage);

  for (size_t i = 0; i < g_viewerLineCount; ++i) {
    tft.setCursor(8, 56 + static_cast<int>(i) * 18);
    tft.print(g_viewerLines[i]);
  }

  if (g_viewerLineCount == 0) {
    tft.setCursor(8, 56);
    tft.print("(empty)");
  }

  drawFooter("UP/DN page LT/CT back RT next");
}

void drawUi() {
  if (g_uiMode == UiMode::Viewer) {
    drawViewerScreen();
  } else {
    drawBrowserScreen();
  }
}

void handlePressed(const char* name) {
  g_lastButton = name;
  Serial.printf("[BTN] %s PRESSED\n", name);
}

bool pollButtons() {
  bool changed = false;
  for (auto& button : g_buttons) {
    const bool nowPressed = readPressed(button.pin);
    if (nowPressed == button.pressed) continue;
    button.pressed = nowPressed;
    changed = true;

    if (!nowPressed) {
      Serial.printf("[BTN] %s RELEASED\n", button.name);
      continue;
    }

    handlePressed(button.name);

    if (!g_sdReady) {
      if (button.pin == kBtnCenter || button.pin == kBtnRight) {
        mountSd(true);
      }
      continue;
    }

    if (g_uiMode == UiMode::Browser) {
      if (button.pin == kBtnUp) {
        moveCursor(-1);
      } else if (button.pin == kBtnDown) {
        moveCursor(+1);
      } else if (button.pin == kBtnLeft) {
        goBack();
      } else if (button.pin == kBtnRight || button.pin == kBtnCenter) {
        openSelection();
      }
      continue;
    }

    if (button.pin == kBtnUp) {
      if (g_viewerPage > 0) loadViewerPage(g_viewerPath, g_viewerPage - 1);
    } else if (button.pin == kBtnDown || button.pin == kBtnRight) {
      if (g_viewerHasNext) loadViewerPage(g_viewerPath, g_viewerPage + 1);
    } else if (button.pin == kBtnLeft || button.pin == kBtnCenter) {
      goBack();
    }
  }
  return changed;
}

void printWiring() {
  Serial.println();
  Serial.println("idk-ts-test");
  Serial.println("microSD wiring:");
  Serial.printf("3V3->3V3 GND->GND CS->GPIO%d MOSI->GPIO%d CLK->GPIO%d MISO->GPIO%d\n", kSdCs, kSdMosi,
                kSdSck, kSdMiso);
  Serial.println("5-way wiring:");
  Serial.printf("VCC->3V3 GND->GND RIGHT->GPIO%d DOWN->GPIO%d LEFT->GPIO%d CENTER->GPIO%d UP->GPIO%d\n",
                kBtnRight, kBtnDown, kBtnLeft, kBtnCenter, kBtnUp);
  Serial.println("ASSUMPTION: all buttons are active LOW with INPUT_PULLUP.");
  Serial.printf("Battery ADC: GPIO%d with assumed 2:1 divider\n", kBatteryAdcPin);
  Serial.println("Browser: UP/DOWN move, LEFT back, RIGHT/CENTER open.");
  Serial.println();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(150);

  enableDisplayPower();
  enableBacklight();
  disableOnboardLed();
  setupBatteryAdc();
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(false);
  tft.setTextSize(1);

  setupButtons();
  mountSd(true);
  updateBatteryStatus(true);
  drawUi();
  printWiring();
}

void loop() {
  bool dirty = false;

  if (pollButtons()) {
    dirty = true;
  }

  const bool oldSdReady = g_sdReady;
  mountSd(false);
  if (oldSdReady != g_sdReady) {
    dirty = true;
    Serial.printf("[SD] %s\n", g_sdReady ? "MOUNTED" : "NOT DETECTED");
  }

  const int oldBatteryPercent = g_batteryPercent;
  updateBatteryStatus(false);
  if (oldBatteryPercent != g_batteryPercent) {
    dirty = true;
  }

  const uint32_t now = millis();
  if (dirty || now - g_lastUiMs > 1000) {
    g_lastUiMs = now;
    drawUi();
  }

  delay(5);
}

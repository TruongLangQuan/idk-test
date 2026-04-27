#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>

namespace {

constexpr int kSdCs = 13;
constexpr int kSdMosi = 11;
constexpr int kSdSck = 12;
constexpr int kSdMiso = 10;

constexpr int kBtnRight = 18;
constexpr int kBtnDown = 17;
constexpr int kBtnLeft = 16;
constexpr int kBtnCenter = 15;
constexpr int kBtnUp = 14;

constexpr int kNeoPixelData = 33;
constexpr int kNeoPixelPower = 34;
constexpr int kDisplayPower = 21;
constexpr int kMaxWads = 32;

struct WadEntry {
  String path;
  String label;
};

struct Button {
  bool now = false;
  bool last = false;
};

TFT_eSPI tft;
SPIClass sdSpi(FSPI);
WadEntry g_wads[kMaxWads];
size_t g_wadCount = 0;
size_t g_cursor = 0;
size_t g_scroll = 0;
String g_status = "Mounting SD...";
Button g_up, g_down, g_left, g_right, g_center;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void setButton(Button& b, bool pressed) {
  b.last = b.now;
  b.now = pressed;
}

bool edge(const Button& b) {
  return b.now && !b.last;
}

void enableDisplayPower() {
  pinMode(kDisplayPower, OUTPUT);
  digitalWrite(kDisplayPower, HIGH);
}

void enableBacklight() {
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#endif
}

void disableOnboardLed() {
  pinMode(kNeoPixelPower, OUTPUT);
  digitalWrite(kNeoPixelPower, LOW);
  pinMode(kNeoPixelData, OUTPUT);
  digitalWrite(kNeoPixelData, LOW);
}

void setupButtons() {
  pinMode(kBtnUp, INPUT_PULLUP);
  pinMode(kBtnDown, INPUT_PULLUP);
  pinMode(kBtnLeft, INPUT_PULLUP);
  pinMode(kBtnRight, INPUT_PULLUP);
  pinMode(kBtnCenter, INPUT_PULLUP);
}

void updateButtons() {
  setButton(g_up, readPressed(kBtnUp));
  setButton(g_down, readPressed(kBtnDown));
  setButton(g_left, readPressed(kBtnLeft));
  setButton(g_right, readPressed(kBtnRight));
  setButton(g_center, readPressed(kBtnCenter));
}

bool mountSd() {
  sdSpi.begin(kSdSck, kSdMiso, kSdMosi, kSdCs);
  delay(10);
  if (!SD.begin(kSdCs, sdSpi)) {
    g_status = "microSD mount failed";
    return false;
  }
  g_status = "microSD mounted";
  return true;
}

void scanWads() {
  g_wadCount = 0;
  File dir = SD.open("/doom");
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    g_status = "No /doom folder";
    return;
  }

  while (g_wadCount < kMaxWads) {
    File entry = dir.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory()) {
      String name = entry.name();
      String lower = name;
      lower.toLowerCase();
      if (lower.endsWith(".wad")) {
        int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);
        g_wads[g_wadCount++] = {String(entry.name()), name};
      }
    }
    entry.close();
  }
  dir.close();

  if (g_wadCount == 0) g_status = "No WAD found in /doom";
}

void drawUi() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("Tenstar Doom Base", 6, 8, 1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("doomgeneric port scaffold", 6, 20, 1);

  if (g_cursor < g_scroll) g_scroll = g_cursor;
  if (g_cursor >= g_scroll + 10) g_scroll = g_cursor - 9;

  for (size_t i = 0; i < 10; ++i) {
    size_t idx = g_scroll + i;
    int y = 42 + static_cast<int>(i) * 16;
    if (idx >= g_wadCount) break;
    bool selected = idx == g_cursor;
    if (selected) {
      tft.fillRoundRect(4, y - 2, 127, 14, 3, TFT_MAROON);
      tft.setTextColor(TFT_WHITE, TFT_MAROON);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(g_wads[idx].label.substring(0, 20), 8, y, 1);
  }

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(g_status, 6, 204, 1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Map for port:", 6, 218, 1);
  tft.drawString("5-way move, center fire", 6, 230, 1);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  enableDisplayPower();
  enableBacklight();
  disableOnboardLed();
  setupButtons();

  tft.init();
  tft.setRotation(0);

  mountSd();
  scanWads();
  drawUi();
}

void loop() {
  updateButtons();
  bool dirty = false;

  if (edge(g_up) && g_cursor > 0) {
    --g_cursor;
    dirty = true;
  }
  if (edge(g_down) && g_cursor + 1 < g_wadCount) {
    ++g_cursor;
    dirty = true;
  }
  if (edge(g_center) && g_wadCount > 0) {
    g_status = "Base ready for " + g_wads[g_cursor].label.substring(0, 14);
    dirty = true;
  }

  if (dirty) drawUi();
  delay(30);
}

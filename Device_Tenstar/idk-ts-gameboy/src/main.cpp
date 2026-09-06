#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <esp32-hal.h>

#include "cpu.h"
#include "lcd.h"
#include "mem.h"
#include "rom.h"
#include "sdl.h"
#include "timer.h"

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
constexpr int kRomMax = 48;
constexpr int kScaledW = 120;
constexpr int kScaledH = 108;
constexpr int kFrameX = 7;
constexpr int kFrameY = 28;
constexpr int kLongPressMs = 280;

struct RomEntry {
  String path;
  String label;
};

enum class AppMode {
  RomMenu,
  Emulator,
  Error,
};

struct HoldState {
  bool now = false;
  bool last = false;
  uint32_t since = 0;
};

TFT_eSPI tft;
SPIClass sdSpi(FSPI);
RomEntry g_roms[kRomMax];
size_t g_romCount = 0;
size_t g_romCursor = 0;
size_t g_romScroll = 0;
AppMode g_mode = AppMode::RomMenu;
String g_status = "Mounting SD...";
String g_error = "";

HoldState g_up;
HoldState g_down;
HoldState g_left;
HoldState g_right;
HoldState g_center;

uint8_t g_frameBuffer[160 * 144];
uint16_t g_scaledFrame[kScaledW * kScaledH];
uint8_t* g_romData = nullptr;
size_t g_romSize = 0;
bool g_frameReady = false;

bool g_btnUp = false;
bool g_btnDown = false;
bool g_btnLeft = false;
bool g_btnRight = false;
bool g_btnA = false;
bool g_btnB = false;
bool g_btnStart = false;
bool g_btnSelect = false;

uint32_t g_cpuFreq = 0;
uint32_t g_cyclesPerFrame = 0;
uint32_t g_cyclesInMicroSec = 0;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void setHold(HoldState& state, bool pressed) {
  state.last = state.now;
  state.now = pressed;
  if (state.now && !state.last) state.since = millis();
}

bool pressedEdge(const HoldState& state) {
  return state.now && !state.last;
}

bool longPressed(const HoldState& state) {
  return state.now && (millis() - state.since >= kLongPressMs);
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
  setHold(g_up, readPressed(kBtnUp));
  setHold(g_down, readPressed(kBtnDown));
  setHold(g_left, readPressed(kBtnLeft));
  setHold(g_right, readPressed(kBtnRight));
  setHold(g_center, readPressed(kBtnCenter));

  g_btnUp = g_up.now && !longPressed(g_up);
  g_btnDown = g_down.now && !longPressed(g_down);
  g_btnLeft = g_left.now;
  g_btnRight = g_right.now;
  g_btnA = g_center.now && !longPressed(g_center);
  g_btnB = longPressed(g_center);
  g_btnStart = longPressed(g_up);
  g_btnSelect = longPressed(g_down);
}

bool mountSd() {
  sdSpi.begin(kSdSck, kSdMiso, kSdMosi, kSdCs);
  delay(10);
  if (!SD.begin(kSdCs, sdSpi)) {
    g_status = "SD mount failed";
    return false;
  }
  uint64_t sizeMb = SD.cardSize() / (1024ULL * 1024ULL);
  g_status = "SD " + String(sizeMb) + "MB";
  return true;
}

void appendRomList(const String& base) {
  File dir = SD.open(base.c_str());
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    return;
  }

  while (g_romCount < kRomMax) {
    File entry = dir.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory()) {
      String name = entry.name();
      String lower = name;
      lower.toLowerCase();
      if (lower.endsWith(".gb") || lower.endsWith(".gbc")) {
        int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);
        g_roms[g_romCount++] = {String(entry.name()), name};
      }
    }
    entry.close();
  }
  dir.close();
}

void scanRoms() {
  g_romCount = 0;
  appendRomList("/gameboy");
  appendRomList("/");
  if (g_romCount == 0) {
    g_mode = AppMode::Error;
    g_error = "No .gb/.gbc in /gameboy";
  }
}

void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Tenstar Game Boy", 8, 6, 1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("UP/DN browse  CENTER boot", 8, 16, 1);

  size_t visibleRows = 11;
  if (g_romCursor < g_romScroll) g_romScroll = g_romCursor;
  if (g_romCursor >= g_romScroll + visibleRows) g_romScroll = g_romCursor - visibleRows + 1;

  for (size_t i = 0; i < visibleRows; ++i) {
    size_t index = g_romScroll + i;
    int y = 34 + static_cast<int>(i) * 16;
    if (index >= g_romCount) break;
    bool selected = index == g_romCursor;
    if (selected) {
      tft.fillRoundRect(4, y - 2, 127, 14, 3, TFT_DARKCYAN);
      tft.setTextColor(TFT_WHITE, TFT_DARKCYAN);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString(g_roms[index].label.substring(0, 20), 8, y, 1);
  }

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(g_status, 8, 222, 1);
}

void drawError() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("Game Boy Error", 8, 10, 1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(g_error, 8, 34, 1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Put ROMs in /gameboy", 8, 58, 1);
}

bool loadRomFile(const String& path) {
  if (g_romData) {
    free(g_romData);
    g_romData = nullptr;
    g_romSize = 0;
  }

  File file = SD.open(path.c_str(), FILE_READ);
  if (!file || file.isDirectory()) {
    g_error = "Cannot open ROM";
    if (file) file.close();
    return false;
  }

  g_romSize = static_cast<size_t>(file.size());
  g_romData = static_cast<uint8_t*>(ps_malloc(g_romSize));
  if (!g_romData) g_romData = static_cast<uint8_t*>(malloc(g_romSize));
  if (!g_romData) {
    g_error = "Not enough RAM";
    file.close();
    return false;
  }

  size_t read = file.read(g_romData, g_romSize);
  file.close();
  if (read != g_romSize) {
    free(g_romData);
    g_romData = nullptr;
    g_romSize = 0;
    g_error = "ROM read failed";
    return false;
  }

  if (!rom_init(g_romData)) {
    free(g_romData);
    g_romData = nullptr;
    g_romSize = 0;
    g_error = "Bad ROM header";
    return false;
  }

  return true;
}

void renderFrame() {
  static const uint16_t palette[4] = {0xFFFF, 0xBDF7, 0x632C, 0x0000};
  for (int y = 0; y < kScaledH; ++y) {
    int srcY = (y * 144) / kScaledH;
    for (int x = 0; x < kScaledW; ++x) {
      int srcX = (x * 160) / kScaledW;
      g_scaledFrame[y * kScaledW + x] = palette[g_frameBuffer[srcY * 160 + srcX] & 0x03];
    }
  }

  tft.pushImage(kFrameX, kFrameY, kScaledW, kScaledH, g_scaledFrame);
  tft.drawRect(kFrameX - 1, kFrameY - 1, kScaledW + 2, kScaledH + 2, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("A=CENTER  B=HOLD CENTER", 4, 150, 1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("START=HOLD UP  SELECT=HOLD DOWN", 4, 162, 1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(g_roms[g_romCursor].label.substring(0, 20), 4, 8, 1);
}

void startEmulator() {
  sdl_init();
  gameboy_mem_init();
  cpu_init();

  g_cpuFreq = getCpuFrequencyMhz() * 1000000UL;
  g_cyclesPerFrame = g_cpuFreq / 60;
  g_cyclesInMicroSec = g_cpuFreq / 1000000UL;

  tft.fillScreen(TFT_BLACK);
  g_mode = AppMode::Emulator;
}

void handleMenu() {
  updateButtons();
  bool dirty = false;

  if (pressedEdge(g_up) && g_romCursor > 0) {
    --g_romCursor;
    dirty = true;
  }
  if (pressedEdge(g_down) && g_romCursor + 1 < g_romCount) {
    ++g_romCursor;
    dirty = true;
  }
  if (pressedEdge(g_center) && g_romCount > 0) {
    g_status = "Loading " + g_roms[g_romCursor].label;
    drawMenu();
    if (!loadRomFile(g_roms[g_romCursor].path)) {
      g_mode = AppMode::Error;
    } else {
      startEmulator();
      return;
    }
    dirty = true;
  }

  if (dirty) drawMenu();
  delay(30);
}

void handleEmulator() {
  bool screenUpdated = false;
  uint32_t startFrameCycle = ESP.getCycleCount();

  while (!screenUpdated) {
    uint32_t emulatedCycles = cpu_cycle();
    screenUpdated = lcd_cycle(emulatedCycles);
    timer_cycle(emulatedCycles);
  }

  sdl_update();

  uint32_t cyclesDelta = ESP.getCycleCount() - startFrameCycle;
  if (cyclesDelta < g_cyclesPerFrame) {
    delayMicroseconds((g_cyclesPerFrame - cyclesDelta) / g_cyclesInMicroSec);
  }
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
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  if (!mountSd()) {
    g_mode = AppMode::Error;
    g_error = "microSD mount failed";
    drawError();
    return;
  }

  scanRoms();
  if (g_mode == AppMode::Error) {
    drawError();
    return;
  }

  drawMenu();
}

void loop() {
  switch (g_mode) {
    case AppMode::RomMenu:
      handleMenu();
      break;
    case AppMode::Emulator:
      handleEmulator();
      break;
    case AppMode::Error:
      updateButtons();
      if (pressedEdge(g_center)) {
        g_mode = AppMode::RomMenu;
        scanRoms();
        drawMenu();
      }
      delay(30);
      break;
  }
}

int sdl_update(void) {
  updateButtons();
  renderFrame();
  return 0;
}

void sdl_init(void) {
  memset(g_frameBuffer, 0, sizeof(g_frameBuffer));
}

void sdl_frame(void) {
  g_frameReady = true;
}

void sdl_quit(void) {}

uint8_t* sdl_get_framebuffer(void) {
  return g_frameBuffer;
}

unsigned int sdl_get_buttons(void) {
  return (g_btnStart ? 8 : 0) | (g_btnSelect ? 4 : 0) | (g_btnB ? 2 : 0) | (g_btnA ? 1 : 0);
}

unsigned int sdl_get_directions(void) {
  return (g_btnDown ? 8 : 0) | (g_btnUp ? 4 : 0) | (g_btnLeft ? 2 : 0) | (g_btnRight ? 1 : 0);
}

#ifdef BIT
#undef BIT
#endif

#include "/home/truonglangquan/idk-git-idk/esp32-gameboy/interrupt.cpp"
#include "/home/truonglangquan/idk-git-idk/esp32-gameboy/mbc.cpp"
#include "/home/truonglangquan/idk-git-idk/esp32-gameboy/rom.cpp"
#include "/home/truonglangquan/idk-git-idk/esp32-gameboy/timer.cpp"
#include "/home/truonglangquan/idk-git-idk/esp32-gameboy/mem.cpp"
#include "/home/truonglangquan/idk-git-idk/esp32-gameboy/lcd.cpp"
#include "/home/truonglangquan/idk-git-idk/esp32-gameboy/cpu.cpp"

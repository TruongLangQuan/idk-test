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

constexpr int kCols = 22;
constexpr int kRows = 18;
constexpr int kCellW = 6;
constexpr int kCellH = 12;
constexpr int kHeaderY = 8;
constexpr int kGridY = 24;

enum class Mode {
  Matrix,
  Tunnel,
  Rain,
  Noise,
};

struct Hold {
  bool now = false;
  bool last = false;
};

TFT_eSPI tft;
SPIClass sdSpi(FSPI);
Mode g_mode = Mode::Matrix;
Hold g_up, g_down, g_left, g_right, g_center;
uint32_t g_lastFrame = 0;
int g_matrix[kCols];
char g_grid[kRows][kCols + 1];
String g_sdStatus = "SD ?";
uint32_t g_tick = 0;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void setHold(Hold& h, bool pressed) {
  h.last = h.now;
  h.now = pressed;
}

bool edge(const Hold& h) {
  return h.now && !h.last;
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
}

void tryMountSd() {
  sdSpi.begin(kSdSck, kSdMiso, kSdMosi, kSdCs);
  delay(10);
  if (!SD.begin(kSdCs, sdSpi)) {
    g_sdStatus = "SD fail";
    return;
  }
  g_sdStatus = "SD ok";
}

const char* modeName(Mode mode) {
  switch (mode) {
    case Mode::Matrix: return "Matrix";
    case Mode::Tunnel: return "Tunnel";
    case Mode::Rain: return "Rain";
    case Mode::Noise: return "Noise";
  }
  return "?";
}

void clearGrid() {
  for (int y = 0; y < kRows; ++y) {
    for (int x = 0; x < kCols; ++x) g_grid[y][x] = ' ';
    g_grid[y][kCols] = '\0';
  }
}

void seedModes() {
  for (int i = 0; i < kCols; ++i) g_matrix[i] = random(-kRows, kRows);
}

void generateMatrix() {
  clearGrid();
  const char glyphs[] = "01#@$%&*+=:.";
  for (int x = 0; x < kCols; ++x) {
    int head = g_matrix[x];
    for (int trail = 0; trail < 5; ++trail) {
      int y = head - trail;
      if (y >= 0 && y < kRows) g_grid[y][x] = glyphs[(x + y + g_tick) % (sizeof(glyphs) - 1)];
    }
    g_matrix[x] += 1;
    if (g_matrix[x] > kRows + random(3, 9)) g_matrix[x] = -random(1, kRows);
  }
}

void generateTunnel() {
  clearGrid();
  float cx = kCols / 2.0f;
  float cy = kRows / 2.0f;
  for (int y = 0; y < kRows; ++y) {
    for (int x = 0; x < kCols; ++x) {
      float dx = x - cx;
      float dy = y - cy;
      float dist = sqrtf(dx * dx + dy * dy);
      int band = static_cast<int>(dist * 2.0f + g_tick * 0.6f) & 7;
      g_grid[y][x] = " .:-=+*#"[band];
    }
  }
}

void generateRain() {
  clearGrid();
  for (int x = 0; x < kCols; ++x) {
    int y = (g_tick + x * 3) % kRows;
    g_grid[y][x] = '|';
    g_grid[(y + 1) % kRows][x] = '.';
  }
}

void generateNoise() {
  clearGrid();
  const char glyphs[] = " .,:;+xX$#";
  for (int y = 0; y < kRows; ++y) {
    for (int x = 0; x < kCols; ++x) {
      g_grid[y][x] = glyphs[random(0, static_cast<int>(sizeof(glyphs) - 1))];
    }
  }
}

void drawGrid(uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("ASCII Saver", 6, kHeaderY, 1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("UP/DN mode  L/R speed  C reset", 6, kHeaderY + 10, 1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(String(modeName(g_mode)) + "  " + g_sdStatus, 6, kHeaderY + 20, 1);

  tft.setTextColor(color, TFT_BLACK);
  for (int y = 0; y < kRows; ++y) {
    tft.drawString(g_grid[y], 2, kGridY + y * kCellH, 1);
  }
}

void render() {
  switch (g_mode) {
    case Mode::Matrix:
      generateMatrix();
      drawGrid(TFT_GREEN);
      break;
    case Mode::Tunnel:
      generateTunnel();
      drawGrid(TFT_WHITE);
      break;
    case Mode::Rain:
      generateRain();
      drawGrid(TFT_CYAN);
      break;
    case Mode::Noise:
      generateNoise();
      drawGrid(TFT_YELLOW);
      break;
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
  seedModes();
  tryMountSd();
}

void loop() {
  updateButtons();

  if (edge(g_up)) {
    g_mode = static_cast<Mode>((static_cast<int>(g_mode) + 3) % 4);
  }
  if (edge(g_down)) {
    g_mode = static_cast<Mode>((static_cast<int>(g_mode) + 1) % 4);
  }
  if (edge(g_center)) {
    seedModes();
  }
  if (edge(g_left) && g_tick > 2) {
    g_tick -= 2;
  }
  if (edge(g_right)) {
    g_tick += 2;
  }

  if (millis() - g_lastFrame >= 90) {
    g_lastFrame = millis();
    ++g_tick;
    render();
  }
}

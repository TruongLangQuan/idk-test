#include <M5Unified.h>

namespace {

static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

static constexpr int kCanvasW = 120;
static constexpr int kCanvasH = 60;
static constexpr int kScale = 2;
static constexpr int kStatusY = 120;
static constexpr uint16_t kBg = TFT_BLACK;
static constexpr uint16_t kGrid = 0x1082;
static constexpr uint16_t kCursor = TFT_WHITE;

struct ExtButton {
  int pin;
  bool pressed;
  uint32_t lastRepeatMs;
};

ExtButton g_ext[] = {
    {kPinUp, false, 0},
    {kPinDown, false, 0},
    {kPinLeft, false, 0},
    {kPinRight, false, 0},
    {kPinCenter, false, 0},
};

uint16_t g_canvas[kCanvasH][kCanvasW];
const uint16_t kPalette[] = {
    TFT_WHITE,  TFT_BLACK,   TFT_RED,     TFT_MAROON, TFT_GREEN,  0x03EF, TFT_BLUE,   TFT_NAVY,
    TFT_YELLOW, TFT_ORANGE,  TFT_CYAN,    0x0410,     TFT_MAGENTA, 0x8010, 0xC618,     0x7BEF,
};

constexpr int kPaletteCount = sizeof(kPalette) / sizeof(kPalette[0]);

int g_colorIndex = 0;
int g_cursorX = kCanvasW / 2;
int g_cursorY = kCanvasH / 2;
bool g_eraseMode = false;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void clearCanvas() {
  memset(g_canvas, 0, sizeof(g_canvas));
}

void moveCursor(int dx, int dy) {
  g_cursorX += dx;
  g_cursorY += dy;
  if (g_cursorX < 0) g_cursorX = 0;
  if (g_cursorX >= kCanvasW) g_cursorX = kCanvasW - 1;
  if (g_cursorY < 0) g_cursorY = 0;
  if (g_cursorY >= kCanvasH) g_cursorY = kCanvasH - 1;
}

void drawPixelCell(int x, int y) {
  M5.Display.fillRect(x * kScale, y * kScale, kScale, kScale, g_canvas[y][x]);
}

void drawCanvas() {
  M5.Display.startWrite();
  for (int y = 0; y < kCanvasH; ++y) {
    for (int x = 0; x < kCanvasW; ++x) {
      drawPixelCell(x, y);
    }
  }
  M5.Display.endWrite();
}

void drawStatusBar() {
  M5.Display.fillRect(0, kStatusY, 240, 15, kBg);
  M5.Display.drawFastHLine(0, kStatusY, 240, kGrid);

  for (int i = 0; i < kPaletteCount; ++i) {
    const int x = 4 + i * 11;
    M5.Display.fillRect(x, kStatusY + 3, 8, 8, kPalette[i]);
    if (i == g_colorIndex) {
      M5.Display.drawRect(x - 1, kStatusY + 2, 10, 10, TFT_WHITE);
    }
  }

  M5.Display.setTextColor(g_eraseMode ? TFT_RED : TFT_GREEN, kBg);
  M5.Display.setCursor(182, 123);
  M5.Display.print(g_eraseMode ? "ER" : "DR");
  M5.Display.setTextColor(TFT_WHITE, kBg);
  M5.Display.setCursor(204, 123);
  M5.Display.printf("%03d,%02d", g_cursorX, g_cursorY);
}

void drawCursor() {
  const int px = g_cursorX * kScale;
  const int py = g_cursorY * kScale;
  const uint16_t borderColor = g_eraseMode ? TFT_RED : kPalette[g_colorIndex];
  M5.Display.drawRect(px, py, kScale, kScale, kCursor);
  M5.Display.drawRect(px - 1, py - 1, kScale + 2, kScale + 2, borderColor);
}

void redrawAll() {
  drawCanvas();
  drawStatusBar();
  drawCursor();
}

void redrawCursorMove(int oldX, int oldY) {
  drawPixelCell(oldX, oldY);
  drawPixelCell(g_cursorX, g_cursorY);
  drawStatusBar();
  drawCursor();
}

void paintAtCursor() {
  g_canvas[g_cursorY][g_cursorX] = g_eraseMode ? kBg : kPalette[g_colorIndex];
  drawPixelCell(g_cursorX, g_cursorY);
  drawCursor();
}

void handleM5Buttons() {
  if (M5.BtnA.wasPressed()) {
    g_colorIndex = (g_colorIndex + 1) % kPaletteCount;
    drawStatusBar();
    drawCursor();
  }
  if (M5.BtnB.wasPressed()) {
    g_colorIndex = (g_colorIndex + kPaletteCount - 1) % kPaletteCount;
    drawStatusBar();
    drawCursor();
  }
  if (M5.BtnPWR.wasPressed()) {
    g_eraseMode = !g_eraseMode;
    drawStatusBar();
    drawCursor();
  }
  if (M5.BtnPWR.pressedFor(700)) {
    clearCanvas();
    redrawAll();
    delay(250);
  }
}

void handleExternalButtons() {
  const uint32_t now = millis();
  auto actMove = [&](ExtButton& btn, int dx, int dy) {
    bool trigger = false;
    if (!btn.pressed) {
      btn.pressed = true;
      btn.lastRepeatMs = now;
      trigger = true;
    } else if (now - btn.lastRepeatMs > 85) {
      btn.lastRepeatMs = now;
      trigger = true;
    }

    if (trigger) {
      const int oldX = g_cursorX;
      const int oldY = g_cursorY;
      moveCursor(dx, dy);
      if (oldX != g_cursorX || oldY != g_cursorY) {
        redrawCursorMove(oldX, oldY);
      }
    }
  };

  for (auto& btn : g_ext) {
    const bool down = readPressed(btn.pin);
    if (!down) {
      btn.pressed = false;
      continue;
    }

    if (btn.pin == kPinUp) {
      actMove(btn, 0, -1);
    } else if (btn.pin == kPinDown) {
      actMove(btn, 0, 1);
    } else if (btn.pin == kPinLeft) {
      actMove(btn, -1, 0);
    } else if (btn.pin == kPinRight) {
      actMove(btn, 1, 0);
    } else if (btn.pin == kPinCenter) {
      if (!btn.pressed) {
        btn.pressed = true;
        paintAtCursor();
      }
    }
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);

  clearCanvas();
  redrawAll();
}

void loop() {
  M5.update();
  handleM5Buttons();
  handleExternalButtons();
  delay(5);
}

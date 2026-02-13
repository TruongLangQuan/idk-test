#include <M5Unified.h>

enum class Mode : uint8_t {
  FASTFETCH = 0,
  CMATRIX = 1,
  PIPE = 2,
};

static Mode g_mode = Mode::FASTFETCH;
static bool g_mode_changed = true;
static uint32_t g_next_tick = 0;

static constexpr int kCellW = 6;
static constexpr int kCellH = 8;
static constexpr int kCols = 40;
static constexpr int kRows = 16;

static int g_drop_row[kCols];
static int g_drop_speed[kCols];

static uint16_t g_pipe_offset = 0;
static int g_pipe_ball_x = 0;
static int g_pipe_ball_y = 0;

static char randChar() {
  const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ#$%&*+-=<>[]{}()";
  return charset[random(sizeof(charset) - 1)];
}

static void setTextStyle() {
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(1);
  M5.Display.setTextWrap(false, false);
}

static void drawHeader(const char* title) {
  setTextStyle();
  M5.Display.setCursor(2, 2);
  M5.Display.printf("idk-linux | %s", title);
}

static void drawFastfetch() {
  M5.Display.fillScreen(TFT_BLACK);
  drawHeader("fastfetch");

  esp_chip_info_t chip{};
  esp_chip_info(&chip);
  uint32_t up = millis() / 1000;
  uint8_t bat = M5.Power.getBatteryLevel();

  int y = 16;
  M5.Display.setCursor(2, y);   M5.Display.println("  __  __ _____");
  y += 8;
  M5.Display.setCursor(2, y);   M5.Display.println(" |  \\/  | ____|");
  y += 8;
  M5.Display.setCursor(2, y);   M5.Display.println(" | |\\/| |  _|");
  y += 8;
  M5.Display.setCursor(2, y);   M5.Display.println(" | |  | | |___");
  y += 8;
  M5.Display.setCursor(2, y);   M5.Display.println(" |_|  |_|_____|");

  y = 16;
  M5.Display.setCursor(102, y); M5.Display.printf("OS: idk-linux");
  y += 8;
  M5.Display.setCursor(102, y); M5.Display.printf("Host: M5StickC Plus2");
  y += 8;
  M5.Display.setCursor(102, y); M5.Display.printf("CPU: ESP32 @ %dMHz", getCpuFrequencyMhz());
  y += 8;
  M5.Display.setCursor(102, y); M5.Display.printf("Cores: %d", chip.cores);
  y += 8;
  M5.Display.setCursor(102, y); M5.Display.printf("Heap: %u KB", ESP.getFreeHeap() / 1024);
  y += 8;
  M5.Display.setCursor(102, y); M5.Display.printf("Battery: %u%%", bat);
  y += 8;
  M5.Display.setCursor(102, y); M5.Display.printf("Uptime: %lus", (unsigned long)up);

  M5.Display.setCursor(2, 126);
  M5.Display.print("M5/Next:mode Prev:back");
}

static void initMatrix() {
  for (int c = 0; c < kCols; ++c) {
    g_drop_row[c] = random(-kRows, 0);
    g_drop_speed[c] = random(1, 4);
  }
}

static void drawMatrix() {
  M5.Display.fillScreen(TFT_BLACK);
  drawHeader("cmatrix");

  for (int c = 0; c < kCols; ++c) {
    int x = c * kCellW;
    for (int t = 0; t < 5; ++t) {
      int r = g_drop_row[c] - t;
      if (r < 0 || r >= kRows) continue;
      int y = 12 + r * kCellH;
      M5.Display.setCursor(x, y);
      M5.Display.print(randChar());
    }
    g_drop_row[c] += g_drop_speed[c];
    if (g_drop_row[c] - 5 > kRows) {
      g_drop_row[c] = random(-kRows, 0);
      g_drop_speed[c] = random(1, 4);
    }
  }

  M5.Display.setCursor(2, 126);
  M5.Display.print("M5/Next:mode Prev:back");
}

static void drawPipe() {
  static const char* rows[] = {
    "   /----\\        /----\\        /----\\   ",
    "   |    |--------|    |--------|    |   ",
    "   \\----/        \\----/        \\----/   ",
    "        \\----------------------------/    ",
    " /----\\        /----\\        /----\\    ",
    " |    |--------|    |--------|    |    ",
    " \\----/        \\----/        \\----/    ",
  };

  M5.Display.fillScreen(TFT_BLACK);
  drawHeader("pipe.sh");

  const int row_count = sizeof(rows) / sizeof(rows[0]);
  for (int i = 0; i < row_count; ++i) {
    M5.Display.setCursor(-(int)g_pipe_offset, 16 + i * 8);
    M5.Display.print(rows[i]);
    M5.Display.setCursor(220 - (int)g_pipe_offset, 16 + i * 8);
    M5.Display.print(rows[i]);
  }

  g_pipe_ball_x = 10 + (millis() / 22) % 220;
  g_pipe_ball_y = 24 + ((millis() / 120) % 7) * 8;
  M5.Display.setCursor(g_pipe_ball_x, g_pipe_ball_y);
  M5.Display.print("o");

  g_pipe_offset = (g_pipe_offset + 1) % 220;

  M5.Display.setCursor(2, 126);
  M5.Display.print("M5/Next:mode Prev:back");
}

static void nextMode() {
  g_mode = static_cast<Mode>((static_cast<uint8_t>(g_mode) + 1) % 3);
  g_mode_changed = true;
}

static void prevMode() {
  g_mode = static_cast<Mode>((static_cast<uint8_t>(g_mode) + 2) % 3);
  g_mode_changed = true;
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  randomSeed(micros());
  initMatrix();
  drawFastfetch();
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
    nextMode();
  }
  if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
    prevMode();
  }

  if (g_mode_changed) {
    M5.Display.fillScreen(TFT_BLACK);
    g_mode_changed = false;
    g_next_tick = 0;
  }

  const uint32_t now = millis();
  if (g_mode == Mode::FASTFETCH) {
    if ((int32_t)(now - g_next_tick) >= 0) {
      drawFastfetch();
      g_next_tick = now + 900;
    }
  } else if (g_mode == Mode::CMATRIX) {
    if ((int32_t)(now - g_next_tick) >= 0) {
      drawMatrix();
      g_next_tick = now + 65;
    }
  } else {
    if ((int32_t)(now - g_next_tick) >= 0) {
      drawPipe();
      g_next_tick = now + 45;
    }
  }

  delay(1);
}

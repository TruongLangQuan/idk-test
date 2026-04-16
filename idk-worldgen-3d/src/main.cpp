#include <M5Unified.h>
#include <math.h>

namespace {

static const int kMapW = 12;
static const int kMapH = 10;
static const uint32_t kMoveCooldownMs = 180;

struct Tile {
  uint8_t height = 0;
  bool building = false;
};

Tile g_map[kMapH][kMapW];
int g_cursor_x = 0;
int g_cursor_y = 0;
uint32_t g_last_move_ms = 0;
uint32_t g_seed = 1;

int isoX(int x, int y) { return 120 + (x - y) * 10; }
int isoY(int x, int y, int h) { return 90 + (x + y) * 5 - h * 5; }

void generateWorld() {
  g_seed = esp_random();
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      uint32_t n = g_seed ^ (x * 1664525u) ^ (y * 1013904223u);
      int ridge = abs(x - kMapW / 2) + abs(y - kMapH / 2);
      g_map[y][x].height = 2 + ((n >> 18) % 6) + (ridge < 4 ? 2 : 0);
      g_map[y][x].building = false;
    }
  }
  for (int i = 0; i < 5; ++i) {
    int x = 1 + (esp_random() % (kMapW - 2));
    int y = 1 + (esp_random() % (kMapH - 2));
    if (g_map[y][x].height >= 5) g_map[y][x].building = true;
  }
  g_cursor_x = kMapW / 2;
  g_cursor_y = kMapH / 2;
}

uint16_t topColor(const Tile& tile) {
  if (tile.building) return TFT_ORANGE;
  if (tile.height <= 3) return M5.Display.color565(50, 150, 70);
  if (tile.height <= 5) return M5.Display.color565(120, 170, 80);
  if (tile.height <= 7) return M5.Display.color565(125, 100, 70);
  return TFT_LIGHTGREY;
}

void drawColumn(int x, int y, const Tile& tile, bool selected) {
  int sx = isoX(x, y);
  int sy = isoY(x, y, tile.height);
  int hpx = tile.height * 4 + (tile.building ? 10 : 0);
  uint16_t top = selected ? TFT_YELLOW : topColor(tile);
  uint16_t left = M5.Display.color565(40, 80, 40);
  uint16_t right = M5.Display.color565(70, 110, 60);
  M5.Display.fillTriangle(sx, sy, sx + 10, sy + 5, sx, sy + 10, left);
  M5.Display.fillTriangle(sx, sy + 10, sx + 10, sy + 5, sx + 10, sy + 15, right);
  M5.Display.fillRect(sx, sy + 5, 10, hpx, right);
  M5.Display.fillRect(sx - 10, sy + 10, 10, hpx, left);
  M5.Display.fillTriangle(sx, sy - 5, sx + 10, sy, sx, sy + 5, top);
  M5.Display.fillTriangle(sx, sy + 5, sx + 10, sy, sx + 10, sy + 10, top);
}

void drawWorld() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("idk-worldgen-3d seed:%u", g_seed);
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      drawColumn(x, y, g_map[y][x], x == g_cursor_x && y == g_cursor_y);
    }
  }
  const Tile& cur = g_map[g_cursor_y][g_cursor_x];
  M5.Display.setCursor(2, 124);
  M5.Display.printf("X:%02d Y:%02d H:%d %s", g_cursor_x, g_cursor_y, cur.height,
                    cur.building ? "BUILD" : "LAND");
}

void moveCursor(int dx, int dy) {
  g_cursor_x = std::max(0, std::min(kMapW - 1, g_cursor_x + dx));
  g_cursor_y = std::max(0, std::min(kMapH - 1, g_cursor_y + dy));
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  generateWorld();
  drawWorld();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    generateWorld();
    drawWorld();
  }
  float ax = 0, ay = 0, az = 0;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    uint32_t now = millis();
    if (now - g_last_move_ms > kMoveCooldownMs) {
      int dx = 0;
      int dy = 0;
      float sx = -ay;
      float sy = -ax;
      if (fabsf(sx) > fabsf(sy)) {
        if (sx > 0.35f) dx = 1;
        else if (sx < -0.35f) dx = -1;
      } else {
        if (sy > 0.35f) dy = 1;
        else if (sy < -0.35f) dy = -1;
      }
      if (dx || dy) {
        moveCursor(dx, dy);
        g_last_move_ms = now;
        drawWorld();
      }
    }
  }
  delay(10);
}

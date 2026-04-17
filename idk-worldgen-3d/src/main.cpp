#include <M5Unified.h>
#include <math.h>

namespace {

static const int kMapW = 10;
static const int kMapH = 8;
static const uint32_t kMoveCooldownMs = 180;

struct Tile {
  uint8_t height = 0;
  bool town = false;
  bool water = false;
};

Tile g_map[kMapH][kMapW];
int g_cursor_x = 0;
int g_cursor_y = 0;
uint32_t g_last_move_ms = 0;
uint32_t g_seed = 1;

int isoX(int x, int y) { return 120 + (x - y) * 13; }
int isoY(int x, int y, int h) { return 80 + (x + y) * 7 - h * 6; }

void generateWorld() {
  g_seed = esp_random();
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      uint32_t n = g_seed ^ (x * 1664525u) ^ (y * 1013904223u);
      g_map[y][x].height = 2 + ((n >> 18) % 6);
      g_map[y][x].water = g_map[y][x].height <= 2;
      g_map[y][x].town = false;
    }
  }
  for (int i = 0; i < 4; ++i) {
    int x = 1 + (esp_random() % (kMapW - 2));
    int y = 1 + (esp_random() % (kMapH - 2));
    if (!g_map[y][x].water && g_map[y][x].height >= 4) g_map[y][x].town = true;
  }
  g_cursor_x = kMapW / 2;
  g_cursor_y = kMapH / 2;
}

uint16_t topColor(const Tile& tile) {
  if (tile.water) return M5.Display.color565(48, 102, 194);
  if (tile.town) return M5.Display.color565(219, 194, 138);
  if (tile.height <= 3) return M5.Display.color565(67, 154, 88);
  if (tile.height <= 5) return M5.Display.color565(124, 157, 86);
  return M5.Display.color565(152, 132, 94);
}

void drawPrism(int x, int y, const Tile& tile, bool selected) {
  int sx = isoX(x, y);
  int sy = isoY(x, y, tile.height);
  int topY = sy - 8;
  int bodyH = tile.height * 6 + (tile.town ? 10 : 0);
  uint16_t top = selected ? TFT_YELLOW : topColor(tile);
  uint16_t left = tile.water ? M5.Display.color565(28, 65, 140) : M5.Display.color565(74, 101, 58);
  uint16_t right = tile.water ? M5.Display.color565(35, 84, 167) : M5.Display.color565(102, 122, 71);

  M5.Display.fillTriangle(sx, topY, sx + 13, topY + 7, sx, topY + 14, top);
  M5.Display.fillTriangle(sx, topY + 14, sx + 13, topY + 7, sx + 13, topY + 21, top);
  M5.Display.fillTriangle(sx - 13, topY + 7, sx, topY + 14, sx - 13, topY + 14 + bodyH, left);
  M5.Display.fillTriangle(sx - 13, topY + 14 + bodyH, sx, topY + 14, sx, topY + 21 + bodyH, left);
  M5.Display.fillTriangle(sx, topY + 14, sx + 13, topY + 21, sx, topY + 21 + bodyH, right);
  M5.Display.fillTriangle(sx, topY + 21 + bodyH, sx + 13, topY + 21, sx + 13, topY + 28 + bodyH, right);

  if (tile.town) {
    M5.Display.fillRect(sx - 3, topY + 6, 7, 12, M5.Display.color565(195, 79, 61));
    M5.Display.fillTriangle(sx - 4, topY + 6, sx, topY + 1, sx + 4, topY + 6, TFT_RED);
  }
}

void drawWorld() {
  M5.Display.fillScreen(M5.Display.color565(17, 22, 30));
  M5.Display.fillRect(0, 0, 240, 18, M5.Display.color565(28, 36, 48));
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("idk-worldgen-3d seed:%u", g_seed);
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      drawPrism(x, y, g_map[y][x], x == g_cursor_x && y == g_cursor_y);
    }
  }
  const Tile& cur = g_map[g_cursor_y][g_cursor_x];
  M5.Display.fillRect(0, 122, 240, 13, M5.Display.color565(28, 36, 48));
  M5.Display.setCursor(4, 125);
  M5.Display.printf("X:%02d Y:%02d H:%d %s", g_cursor_x, g_cursor_y, cur.height,
                    cur.town ? "TOWN" : cur.water ? "WATER" : "LAND");
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

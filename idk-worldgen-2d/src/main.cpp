#include <M5Unified.h>
#include <math.h>

namespace {

static const int kMapW = 32;
static const int kMapH = 18;
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

uint8_t clampHeight(int v) {
  if (v < 0) return 0;
  if (v > 9) return 9;
  return static_cast<uint8_t>(v);
}

uint16_t tileColor(const Tile& tile) {
  if (tile.building) return TFT_ORANGE;
  if (tile.height <= 2) return M5.Display.color565(20, 70, 180);
  if (tile.height <= 4) return M5.Display.color565(30, 150, 60);
  if (tile.height <= 6) return M5.Display.color565(120, 160, 70);
  if (tile.height <= 8) return M5.Display.color565(110, 90, 60);
  return TFT_LIGHTGREY;
}

void smoothMap() {
  for (int pass = 0; pass < 3; ++pass) {
    Tile copy[kMapH][kMapW];
    memcpy(copy, g_map, sizeof(g_map));
    for (int y = 0; y < kMapH; ++y) {
      for (int x = 0; x < kMapW; ++x) {
        int total = 0;
        int count = 0;
        for (int oy = -1; oy <= 1; ++oy) {
          for (int ox = -1; ox <= 1; ++ox) {
            int nx = x + ox;
            int ny = y + oy;
            if (nx < 0 || ny < 0 || nx >= kMapW || ny >= kMapH) continue;
            total += copy[ny][nx].height;
            count++;
          }
        }
        g_map[y][x].height = clampHeight((total + count / 2) / count);
      }
    }
  }
}

bool canPlaceBuilding(int x0, int y0, int w, int h) {
  if (x0 < 1 || y0 < 1 || x0 + w >= kMapW - 1 || y0 + h >= kMapH - 1) return false;
  int base = g_map[y0][x0].height;
  if (base < 3 || base > 6) return false;
  for (int y = y0; y < y0 + h; ++y) {
    for (int x = x0; x < x0 + w; ++x) {
      if (g_map[y][x].building) return false;
      if (abs(g_map[y][x].height - base) > 1) return false;
    }
  }
  return true;
}

void placeBuildings() {
  int count = 4 + (esp_random() % 4);
  for (int i = 0; i < count; ++i) {
    for (int tries = 0; tries < 24; ++tries) {
      int bw = 2 + (esp_random() % 3);
      int bh = 2 + (esp_random() % 3);
      int x0 = esp_random() % (kMapW - bw);
      int y0 = esp_random() % (kMapH - bh);
      if (!canPlaceBuilding(x0, y0, bw, bh)) continue;
      for (int y = y0; y < y0 + bh; ++y) {
        for (int x = x0; x < x0 + bw; ++x) {
          g_map[y][x].building = true;
        }
      }
      break;
    }
  }
}

void generateWorld() {
  g_seed = esp_random();
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      uint32_t n = g_seed ^ (x * 1103515245u) ^ (y * 12345u);
      g_map[y][x].height = 1 + ((n >> 16) % 9);
      g_map[y][x].building = false;
    }
  }
  smoothMap();
  placeBuildings();
  g_cursor_x = kMapW / 2;
  g_cursor_y = kMapH / 2;
}

void drawWorld() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("idk-worldgen-2d  seed:%u", g_seed);
  const int tile = 6;
  const int ox = 24;
  const int oy = 18;
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      const Tile& t = g_map[y][x];
      int px = ox + x * tile;
      int py = oy + y * tile;
      M5.Display.fillRect(px, py, tile, tile, tileColor(t));
      if (t.building) {
        M5.Display.drawRect(px + 1, py + 1, tile - 2, tile - 2, TFT_RED);
      }
    }
  }
  int cx = ox + g_cursor_x * tile;
  int cy = oy + g_cursor_y * tile;
  M5.Display.drawRect(cx - 1, cy - 1, tile + 2, tile + 2, TFT_YELLOW);
  M5.Display.setCursor(2, 124);
  const Tile& cur = g_map[g_cursor_y][g_cursor_x];
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

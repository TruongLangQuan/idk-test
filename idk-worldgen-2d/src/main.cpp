#include <M5Unified.h>
#include <math.h>

namespace {

static const int kMapW = 24;
static const int kMapH = 14;
static const uint32_t kMoveCooldownMs = 180;

enum Terrain : uint8_t {
  Water,
  Grass,
  Dirt,
  Hill,
  Rock,
};

struct Tile {
  uint8_t height = 0;
  Terrain terrain = Grass;
  bool building = false;
  bool road = false;
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

uint16_t terrainColor(const Tile& tile) {
  switch (tile.terrain) {
    case Water:
      return M5.Display.color565(26, 86, 178);
    case Grass:
      return M5.Display.color565(57, 150, 82);
    case Dirt:
      return M5.Display.color565(148, 118, 76);
    case Hill:
      return M5.Display.color565(110, 143, 75);
    case Rock:
    default:
      return M5.Display.color565(145, 150, 156);
  }
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

void classifyTerrain() {
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      Tile& t = g_map[y][x];
      t.building = false;
      t.road = false;
      if (t.height <= 2) t.terrain = Water;
      else if (t.height <= 4) t.terrain = Grass;
      else if (t.height <= 6) t.terrain = Dirt;
      else if (t.height <= 8) t.terrain = Hill;
      else t.terrain = Rock;
    }
  }
}

bool canPlaceBuilding(int x0, int y0, int w, int h) {
  if (x0 < 1 || y0 < 1 || x0 + w >= kMapW - 1 || y0 + h >= kMapH - 1) return false;
  for (int y = y0; y < y0 + h; ++y) {
    for (int x = x0; x < x0 + w; ++x) {
      const Tile& t = g_map[y][x];
      if (t.terrain == Water || t.terrain == Rock || t.building) return false;
    }
  }
  return true;
}

void layRoad(int x0, int y0, int w, int h) {
  int cx = x0 + w / 2;
  int cy = y0 + h / 2;
  while (cx > 1) {
    g_map[cy][cx].road = true;
    cx--;
  }
  while (cy > 1) {
    g_map[cy][cx].road = true;
    cy--;
  }
}

void placeBuildings() {
  int count = 4 + (esp_random() % 4);
  for (int i = 0; i < count; ++i) {
    for (int tries = 0; tries < 24; ++tries) {
      int bw = 2 + (esp_random() % 3);
      int bh = 2 + (esp_random() % 2);
      int x0 = 1 + (esp_random() % (kMapW - bw - 2));
      int y0 = 1 + (esp_random() % (kMapH - bh - 2));
      if (!canPlaceBuilding(x0, y0, bw, bh)) continue;
      for (int y = y0; y < y0 + bh; ++y) {
        for (int x = x0; x < x0 + bw; ++x) g_map[y][x].building = true;
      }
      layRoad(x0, y0, bw, bh);
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
    }
  }
  smoothMap();
  classifyTerrain();
  placeBuildings();
  g_cursor_x = kMapW / 2;
  g_cursor_y = kMapH / 2;
}

void drawTile(int px, int py, int tile, const Tile& t) {
  uint16_t base = terrainColor(t);
  M5.Display.fillRoundRect(px, py, tile, tile, 2, base);
  if (t.terrain == Water) {
    M5.Display.drawFastHLine(px + 1, py + tile / 2, tile - 2, TFT_CYAN);
  } else if (t.terrain == Grass) {
    M5.Display.drawPixel(px + 2, py + 2, TFT_DARKGREEN);
    M5.Display.drawPixel(px + tile - 3, py + tile - 3, TFT_DARKGREEN);
  } else if (t.terrain == Hill) {
    M5.Display.drawFastHLine(px + 1, py + 1, tile - 2, TFT_OLIVE);
  } else if (t.terrain == Rock) {
    M5.Display.drawLine(px + 1, py + tile - 2, px + tile - 2, py + 1, TFT_WHITE);
  }
  if (t.road) {
    M5.Display.fillRect(px + tile / 2 - 1, py, 2, tile, M5.Display.color565(90, 70, 45));
    M5.Display.fillRect(px, py + tile / 2 - 1, tile, 2, M5.Display.color565(90, 70, 45));
  }
  if (t.building) {
    M5.Display.fillRect(px + 2, py + 3, tile - 4, tile - 5, M5.Display.color565(222, 205, 154));
    M5.Display.fillTriangle(px + 1, py + 4, px + tile / 2, py, px + tile - 1, py + 4,
                            M5.Display.color565(165, 62, 49));
    M5.Display.drawRect(px + 2, py + 3, tile - 4, tile - 5, TFT_BROWN);
  }
}

void drawWorld() {
  M5.Display.fillScreen(M5.Display.color565(14, 25, 24));
  M5.Display.fillRect(0, 0, 240, 16, M5.Display.color565(24, 38, 35));
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("idk-worldgen-2d  seed:%u", g_seed);

  const int tile = 9;
  const int ox = 12;
  const int oy = 20;
  for (int y = 0; y < kMapH; ++y) {
    for (int x = 0; x < kMapW; ++x) {
      drawTile(ox + x * tile, oy + y * tile, tile - 1, g_map[y][x]);
    }
  }

  int cx = ox + g_cursor_x * tile;
  int cy = oy + g_cursor_y * tile;
  M5.Display.drawRoundRect(cx - 1, cy - 1, tile + 1, tile + 1, 2, TFT_YELLOW);
  M5.Display.drawRoundRect(cx - 2, cy - 2, tile + 3, tile + 3, 2, TFT_RED);

  const Tile& cur = g_map[g_cursor_y][g_cursor_x];
  M5.Display.fillRect(0, 122, 240, 13, M5.Display.color565(24, 38, 35));
  M5.Display.setCursor(4, 125);
  M5.Display.printf("X:%02d Y:%02d H:%d %s", g_cursor_x, g_cursor_y, cur.height,
                    cur.building ? "TOWN" : cur.road ? "ROAD" : "FIELD");
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

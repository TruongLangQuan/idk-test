#include <M5Unified.h>
#include <vector>
#include <cmath>

namespace {

static const uint16_t kBg = TFT_BLACK;
static const uint16_t kFg = TFT_GREEN;
static const uint16_t kAccent = TFT_RED;
static const uint32_t kMoveCooldownMs = 250;

static const int kCols = 12;
static const int kRows = 8;
static const int kLevels3D = 3;

enum Direction { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

struct Cell {
  bool wall[4];
  bool visited;
  bool stairUp;
  bool stairDown;
};

static Cell g_cells[kLevels3D][kRows][kCols];
static int g_level_count = 1;
static bool g_mode_3d = false;

static int g_px = 0;
static int g_py = 0;
static int g_pz = 0;
static uint32_t g_last_move_ms = 0;

static int cellSize() {
  int w = M5.Display.width();
  int h = M5.Display.height();
  int size_w = (w - 20) / kCols;
  int size_h = (h - 30) / kRows;
  return size_w < size_h ? size_w : size_h;
}

static void resetCells(int levels) {
  for (int z = 0; z < levels; ++z) {
    for (int y = 0; y < kRows; ++y) {
      for (int x = 0; x < kCols; ++x) {
        g_cells[z][y][x].wall[NORTH] = true;
        g_cells[z][y][x].wall[EAST] = true;
        g_cells[z][y][x].wall[SOUTH] = true;
        g_cells[z][y][x].wall[WEST] = true;
        g_cells[z][y][x].visited = false;
        g_cells[z][y][x].stairUp = false;
        g_cells[z][y][x].stairDown = false;
      }
    }
  }
}

static bool inBounds(int x, int y) {
  return x >= 0 && x < kCols && y >= 0 && y < kRows;
}

static void carve(int z, int x1, int y1, int x2, int y2, int dir) {
  g_cells[z][y1][x1].wall[dir] = false;
  int opp = (dir + 2) % 4;
  g_cells[z][y2][x2].wall[opp] = false;
}

static void generateLayer(int z) {
  std::vector<int> stack;
  stack.reserve(kCols * kRows);
  int start = 0;
  stack.push_back(start);
  g_cells[z][0][0].visited = true;

  while (!stack.empty()) {
    int idx = stack.back();
    int cx = idx % kCols;
    int cy = idx / kCols;

    int dirs[4];
    int count = 0;
    if (inBounds(cx, cy - 1) && !g_cells[z][cy - 1][cx].visited) dirs[count++] = NORTH;
    if (inBounds(cx + 1, cy) && !g_cells[z][cy][cx + 1].visited) dirs[count++] = EAST;
    if (inBounds(cx, cy + 1) && !g_cells[z][cy + 1][cx].visited) dirs[count++] = SOUTH;
    if (inBounds(cx - 1, cy) && !g_cells[z][cy][cx - 1].visited) dirs[count++] = WEST;

    if (count == 0) {
      stack.pop_back();
      continue;
    }

    int dir = dirs[esp_random() % count];
    int nx = cx;
    int ny = cy;
    if (dir == NORTH) ny--;
    if (dir == EAST) nx++;
    if (dir == SOUTH) ny++;
    if (dir == WEST) nx--;

    carve(z, cx, cy, nx, ny, dir);
    g_cells[z][ny][nx].visited = true;
    stack.push_back(ny * kCols + nx);
  }
}

static void placeStairs(int levels) {
  if (levels <= 1) return;
  for (int z = 0; z < levels - 1; ++z) {
    int sx = esp_random() % kCols;
    int sy = esp_random() % kRows;
    g_cells[z][sy][sx].stairDown = true;
    g_cells[z + 1][sy][sx].stairUp = true;
  }
}

static void generateMaze() {
  g_level_count = g_mode_3d ? kLevels3D : 1;
  resetCells(g_level_count);
  for (int z = 0; z < g_level_count; ++z) {
    generateLayer(z);
  }
  placeStairs(g_level_count);
  g_px = 0;
  g_py = 0;
  g_pz = 0;
}

static void drawMaze() {
  int w = M5.Display.width();
  int h = M5.Display.height();
  int cs = cellSize();
  int maze_w = kCols * cs;
  int maze_h = kRows * cs;
  int ox = (w - maze_w) / 2;
  int oy = (h - maze_h) / 2 + 6;

  M5.Display.fillScreen(kBg);
  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("idk-maze %s", g_mode_3d ? "3D" : "2D");
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setCursor(w - 70, 2);
  M5.Display.printf("L:%d/%d", g_pz + 1, g_level_count);

  for (int y = 0; y < kRows; ++y) {
    for (int x = 0; x < kCols; ++x) {
      Cell &c = g_cells[g_pz][y][x];
      int x0 = ox + x * cs;
      int y0 = oy + y * cs;
      if (c.wall[NORTH]) M5.Display.drawLine(x0, y0, x0 + cs, y0, kFg);
      if (c.wall[EAST]) M5.Display.drawLine(x0 + cs, y0, x0 + cs, y0 + cs, kFg);
      if (c.wall[SOUTH]) M5.Display.drawLine(x0, y0 + cs, x0 + cs, y0 + cs, kFg);
      if (c.wall[WEST]) M5.Display.drawLine(x0, y0, x0, y0 + cs, kFg);

      if (c.stairUp) {
        M5.Display.fillCircle(x0 + cs / 2, y0 + cs / 2, 2, TFT_CYAN);
      }
      if (c.stairDown) {
        M5.Display.drawCircle(x0 + cs / 2, y0 + cs / 2, 3, TFT_CYAN);
      }
    }
  }

  // Exit
  int ex = ox + (kCols - 1) * cs + cs / 2;
  int ey = oy + (kRows - 1) * cs + cs / 2;
  M5.Display.drawCircle(ex, ey, 4, TFT_YELLOW);

  // Player
  int px = ox + g_px * cs + cs / 2;
  int py = oy + g_py * cs + cs / 2;
  M5.Display.fillCircle(px, py, 3, TFT_RED);

  M5.Display.setTextColor(TFT_DARKGREY, kBg);
  M5.Display.setCursor(2, h - 10);
  M5.Display.print("Tilt move | A:Up B:Down PWR:New");
}

static bool tryMove(int dx, int dy) {
  if (dx == 0 && dy == 0) return false;
  int nx = g_px + dx;
  int ny = g_py + dy;
  if (!inBounds(nx, ny)) return false;
  Cell &c = g_cells[g_pz][g_py][g_px];
  if (dx == 1 && c.wall[EAST]) return false;
  if (dx == -1 && c.wall[WEST]) return false;
  if (dy == 1 && c.wall[SOUTH]) return false;
  if (dy == -1 && c.wall[NORTH]) return false;
  g_px = nx;
  g_py = ny;
  return true;
}

static void checkWin() {
  if (g_px == kCols - 1 && g_py == kRows - 1 && g_pz == g_level_count - 1) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Display.setCursor(40, 60);
    M5.Display.print("YOU WIN!");
    delay(700);
    generateMaze();
    drawMaze();
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  generateMaze();
  drawMaze();
}

void loop() {
  M5.update();

  if (M5.BtnPWR.wasPressed()) {
    g_mode_3d = !g_mode_3d;
    generateMaze();
    drawMaze();
  }

  if (M5.BtnA.wasPressed()) {
    Cell &c = g_cells[g_pz][g_py][g_px];
    if (c.stairUp && g_pz < g_level_count - 1) {
      g_pz++;
      drawMaze();
    }
  }

  if (M5.BtnB.wasPressed()) {
    Cell &c = g_cells[g_pz][g_py][g_px];
    if (c.stairDown && g_pz > 0) {
      g_pz--;
      drawMaze();
    }
  }

  float ax = 0, ay = 0, az = 0;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    uint32_t now = millis();
    if (now - g_last_move_ms > kMoveCooldownMs) {
      int dx = 0;
      int dy = 0;
      // Tilt right -> move right, left -> move left
      if (ax > 0.35f) dx = 1;
      else if (ax < -0.35f) dx = -1;
      // Tilt up -> move up, down -> move down
      if (ay > 0.35f) dy = 1;
      else if (ay < -0.35f) dy = -1;
      if (tryMove(dx, dy)) {
        g_last_move_ms = now;
        drawMaze();
        checkWin();
      }
    }
  }

  delay(10);
}

#include <M5Unified.h>
#include <math.h>

namespace {

static const int kSizeX = 8;
static const int kSizeY = 8;
static const int kSizeZ = 4;
static const uint32_t kMoveCooldownMs = 170;

uint8_t g_voxels[kSizeZ][kSizeY][kSizeX];
int g_cursor_x = 0;
int g_cursor_y = 0;
int g_cursor_z = 0;
uint32_t g_last_move_ms = 0;

int isoX(int x, int y) { return 120 + (x - y) * 12; }
int isoY(int x, int y, int z) { return 92 + (x + y) * 6 - z * 16; }

void resetScene() {
  memset(g_voxels, 0, sizeof(g_voxels));
  for (int y = 0; y < kSizeY; ++y) {
    for (int x = 0; x < kSizeX; ++x) g_voxels[0][y][x] = 1;
  }
  g_cursor_x = 3;
  g_cursor_y = 3;
  g_cursor_z = 1;
}

void drawCube(int x, int y, int z, uint16_t top, uint16_t left, uint16_t right) {
  int sx = isoX(x, y);
  int sy = isoY(x, y, z);
  M5.Display.fillTriangle(sx, sy - 6, sx + 12, sy, sx, sy + 6, top);
  M5.Display.fillTriangle(sx, sy + 6, sx + 12, sy, sx + 12, sy + 12, top);
  M5.Display.fillTriangle(sx - 12, sy, sx, sy + 6, sx - 12, sy + 18, left);
  M5.Display.fillTriangle(sx - 12, sy + 18, sx, sy + 6, sx, sy + 24, left);
  M5.Display.fillTriangle(sx, sy + 6, sx + 12, sy + 12, sx, sy + 24, right);
  M5.Display.fillTriangle(sx, sy + 24, sx + 12, sy + 12, sx + 12, sy + 30, right);
}

void drawScene() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-block-builder-3d");
  for (int z = 0; z < kSizeZ; ++z) {
    for (int y = 0; y < kSizeY; ++y) {
      for (int x = 0; x < kSizeX; ++x) {
        if (!g_voxels[z][y][x]) continue;
        drawCube(x, y, z, M5.Display.color565(80, 180, 255), M5.Display.color565(35, 90, 170),
                 M5.Display.color565(55, 130, 210));
      }
    }
  }
  drawCube(g_cursor_x, g_cursor_y, g_cursor_z, TFT_YELLOW, M5.Display.color565(120, 120, 0),
           M5.Display.color565(180, 180, 0));
  M5.Display.setCursor(2, 124);
  M5.Display.printf("X:%d Y:%d Z:%d A:Z+ B:Z- PWR:block", g_cursor_x, g_cursor_y, g_cursor_z);
}

void moveCursor(int dx, int dy) {
  g_cursor_x = std::max(0, std::min(kSizeX - 1, g_cursor_x + dx));
  g_cursor_y = std::max(0, std::min(kSizeY - 1, g_cursor_y + dy));
}

void toggleBlock() {
  g_voxels[g_cursor_z][g_cursor_y][g_cursor_x] = !g_voxels[g_cursor_z][g_cursor_y][g_cursor_x];
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  resetScene();
  drawScene();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    g_cursor_z = std::min(kSizeZ - 1, g_cursor_z + 1);
    drawScene();
  }
  if (M5.BtnB.wasPressed()) {
    g_cursor_z = std::max(0, g_cursor_z - 1);
    drawScene();
  }
  if (M5.BtnPWR.wasPressed()) {
    toggleBlock();
    drawScene();
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
        drawScene();
      }
    }
  }
  delay(10);
}

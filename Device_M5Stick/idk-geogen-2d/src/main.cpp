#include <M5Unified.h>
#include <math.h>

namespace {

enum SceneType {
  TriangleNet,
  CircleTangents,
  ConicField,
  LissajousWeb,
  PolygonBloom,
  Count,
};

struct SceneConfig {
  SceneType type;
  uint32_t seed;
  int points;
  int radius_a;
  int radius_b;
  float phase;
  float warp;
};

SceneConfig g_scene = {};

static uint16_t pickColor(int index) {
  static const uint16_t kPalette[] = {TFT_CYAN, TFT_GREEN, TFT_YELLOW, TFT_MAGENTA, TFT_ORANGE, TFT_RED};
  return kPalette[index % (sizeof(kPalette) / sizeof(kPalette[0]))];
}

static void randomizeScene() {
  g_scene.seed = esp_random();
  randomSeed(g_scene.seed);
  g_scene.type = static_cast<SceneType>(random(0, static_cast<long>(Count)));
  g_scene.points = random(5, 11);
  g_scene.radius_a = random(18, 50);
  g_scene.radius_b = random(12, 40);
  g_scene.phase = static_cast<float>(random(0, 628)) / 100.0f;
  g_scene.warp = static_cast<float>(random(30, 150)) / 100.0f;
}

static void drawHeader(const char* label) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("idk-geogen-2d : %s", label);
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 14);
  M5.Display.printf("seed:%lu pts:%d", static_cast<unsigned long>(g_scene.seed), g_scene.points);
}

static void drawTriangleNet() {
  drawHeader("triangle-net");
  int cx = 120;
  int cy = 72;
  for (int i = 0; i < g_scene.points; ++i) {
    float a = i * 2.0f * PI / g_scene.points - PI / 2.0f + g_scene.phase;
    int x = cx + cosf(a) * g_scene.radius_a;
    int y = cy + sinf(a * g_scene.warp) * g_scene.radius_b;
    for (int j = i + 1; j < g_scene.points; ++j) {
      float b = j * 2.0f * PI / g_scene.points - PI / 2.0f + g_scene.phase;
      int x2 = cx + cosf(b) * g_scene.radius_a;
      int y2 = cy + sinf(b * g_scene.warp) * g_scene.radius_b;
      M5.Display.drawLine(x, y, x2, y2, pickColor(i + j));
    }
    M5.Display.fillCircle(x, y, 2 + (i % 2), TFT_WHITE);
  }
}

static void drawCircleTangents() {
  drawHeader("circle-tangents");
  for (int i = 0; i < 5; ++i) {
    int x = 36 + i * 42 + random(-8, 9);
    int y = 70 + random(-22, 23);
    int r = random(10, 28);
    M5.Display.drawCircle(x, y, r, pickColor(i));
    M5.Display.fillCircle(x, y, 2, TFT_WHITE);
    for (int j = i + 1; j < 5; ++j) {
      int x2 = 36 + j * 42 + random(-8, 9);
      int y2 = 70 + random(-22, 23);
      M5.Display.drawLine(x, y, x2, y2, TFT_DARKCYAN);
    }
  }
}

static void drawConicField() {
  drawHeader("conic-field");
  for (int a = 0; a < 4; ++a) {
    int rx = g_scene.radius_a / 2 + a * 8;
    int ry = g_scene.radius_b / 2 + a * 6;
    M5.Display.drawEllipse(40 + a * 46, 70 + ((a % 2) ? -10 : 10), rx, ry, pickColor(a));
  }
  for (int x = 10; x < 230; x += 4) {
    float xf = (x - 120) * 0.05f;
    int y = 70 + static_cast<int>(sinf(xf * g_scene.warp + g_scene.phase) * g_scene.radius_b * 0.8f);
    M5.Display.drawPixel(x, y, TFT_WHITE);
  }
}

static void drawLissajousWeb() {
  drawHeader("lissajous-web");
  int cx = 120;
  int cy = 72;
  for (int i = 0; i < 96; ++i) {
    float t = i * 2.0f * PI / 96.0f;
    int x = cx + cosf(t * g_scene.warp + g_scene.phase) * g_scene.radius_a;
    int y = cy + sinf(t * (g_scene.warp + 1.0f)) * g_scene.radius_b;
    int x2 = cx + cosf((t + 0.7f) * (g_scene.warp + 0.3f)) * (g_scene.radius_a + 18);
    int y2 = cy + sinf((t + 0.7f) * (g_scene.warp + 1.2f)) * (g_scene.radius_b + 10);
    M5.Display.drawLine(x, y, x2, y2, pickColor(i));
  }
}

static void drawPolygonBloom() {
  drawHeader("polygon-bloom");
  int sides = g_scene.points;
  for (int ring = 0; ring < 4; ++ring) {
    float rot = g_scene.phase + ring * 0.25f;
    int radius = 16 + ring * 16;
    for (int i = 0; i < sides; ++i) {
      float a = i * 2.0f * PI / sides + rot;
      float b = ((i + 1) % sides) * 2.0f * PI / sides + rot;
      int x1 = 120 + cosf(a) * radius;
      int y1 = 72 + sinf(a) * radius;
      int x2 = 120 + cosf(b) * radius;
      int y2 = 72 + sinf(b) * radius;
      M5.Display.drawLine(x1, y1, x2, y2, pickColor(i + ring));
      M5.Display.drawLine(120, 72, x1, y1, TFT_DARKGREY);
    }
  }
}

static void drawScene() {
  switch (g_scene.type) {
    case TriangleNet:
      drawTriangleNet();
      break;
    case CircleTangents:
      drawCircleTangents();
      break;
    case ConicField:
      drawConicField();
      break;
    case LissajousWeb:
      drawLissajousWeb();
      break;
    case PolygonBloom:
      drawPolygonBloom();
      break;
    default:
      break;
  }
  M5.Display.setCursor(4, 124);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.print("A: random scene");
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
#if defined(STICKS3)
  M5.Display.setRotation(1);
#else
  M5.Display.setRotation(3);
#endif
  M5.Display.setBrightness(180);
  randomSeed(micros());
  randomizeScene();
  drawScene();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    randomizeScene();
    drawScene();
  }
  delay(10);
}

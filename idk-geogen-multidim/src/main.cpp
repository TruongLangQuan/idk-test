#include <M5Unified.h>
#include <math.h>

namespace {

struct ShapeConfig {
  int dim;
  int points;
  float phase;
  float twist;
  float depth;
  int variant;
  uint32_t seed;
};

ShapeConfig g_cfg = {};

static uint16_t palette(int index) {
  static const uint16_t kPalette[] = {TFT_CYAN, TFT_GREEN, TFT_MAGENTA, TFT_ORANGE, TFT_YELLOW, TFT_RED};
  return kPalette[index % (sizeof(kPalette) / sizeof(kPalette[0]))];
}

static void randomizeConfig() {
  g_cfg.seed = esp_random();
  randomSeed(g_cfg.seed);
  g_cfg.dim = random(3, 9);
  g_cfg.points = random(6, 18);
  g_cfg.phase = static_cast<float>(random(0, 628)) / 100.0f;
  g_cfg.twist = static_cast<float>(random(40, 220)) / 100.0f;
  g_cfg.depth = static_cast<float>(random(20, 90)) / 100.0f;
  g_cfg.variant = random(0, 4);
}

static void drawAxes() {
  M5.Display.drawFastHLine(18, 68, 204, TFT_DARKGREY);
  M5.Display.drawFastVLine(120, 16, 104, TFT_DARKGREY);
}

static void projectPoint(float angle, float radius, float layer, int& x, int& y) {
  float px = cosf(angle + g_cfg.phase) * radius;
  float py = sinf(angle * g_cfg.twist + g_cfg.phase) * radius * 0.65f;
  float pz = layer * 30.0f;
  float pw = sinf(angle * 0.7f + g_cfg.phase) * (g_cfg.dim - 2) * 6.0f;

  if (g_cfg.variant == 1) {
    px += pw * 0.6f;
    py += pz * 0.35f;
  } else if (g_cfg.variant == 2) {
    px += pz * 0.5f;
    py -= pw * 0.4f;
  } else if (g_cfg.variant == 3) {
    px += pw * 0.45f;
    py += sinf(layer + angle) * 18.0f;
  } else {
    px += pz * 0.35f;
    py -= pz * 0.2f;
  }

  x = 120 + static_cast<int>(px);
  y = 68 + static_cast<int>(py);
}

static void drawShape() {
  M5.Display.fillScreen(TFT_BLACK);
  drawAxes();

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("idk-geogen-%dd", g_cfg.dim);
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 14);
  M5.Display.printf("seed:%lu v:%d", static_cast<unsigned long>(g_cfg.seed), g_cfg.variant + 1);

  const int layers = g_cfg.dim - 1;
  const int points = g_cfg.points;
  for (int layer = 0; layer < layers; ++layer) {
    float layer_pos = ((static_cast<float>(layer) / (layers - 1 + (layers == 1))) - 0.5f) * g_cfg.depth;
    for (int i = 0; i < points; ++i) {
      float a = (2.0f * PI * i) / points;
      int x1, y1, x2, y2;
      projectPoint(a, 22.0f + layer * 6.0f, layer_pos, x1, y1);
      projectPoint((2.0f * PI * ((i + 1) % points)) / points, 22.0f + layer * 6.0f, layer_pos, x2, y2);
      M5.Display.drawLine(x1, y1, x2, y2, palette(i + layer));
      if (layer + 1 < layers) {
        int x3, y3;
        float next_pos = ((static_cast<float>(layer + 1) / (layers - 1 + (layers == 1))) - 0.5f) * g_cfg.depth;
        projectPoint(a + 0.12f * g_cfg.twist, 22.0f + (layer + 1) * 6.0f, next_pos, x3, y3);
        M5.Display.drawLine(x1, y1, x3, y3, TFT_DARKCYAN);
      }
      M5.Display.fillCircle(x1, y1, 2, TFT_WHITE);
    }
  }

  for (int i = 0; i < g_cfg.dim; ++i) {
    float a = (2.0f * PI * i) / g_cfg.dim + g_cfg.phase;
    int x = 120 + static_cast<int>(cosf(a) * (28 + g_cfg.dim * 3));
    int y = 68 + static_cast<int>(sinf(a * 1.3f) * (16 + g_cfg.dim * 2));
    M5.Display.drawLine(120, 68, x, y, TFT_DARKGREY);
  }

  M5.Display.setCursor(4, 124);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.print("A: random multidim");
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
  randomizeConfig();
  drawShape();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    randomizeConfig();
    drawShape();
  }
  delay(10);
}

#include <M5Unified.h>
#include <math.h>

static constexpr int kOrbitals = 3;
static constexpr int kPoints = 420;

static float g_angle[kOrbitals][kPoints];
static float g_radius[kOrbitals][kPoints];
static uint8_t g_glow[kOrbitals][kPoints];

static int g_w = 0;
static int g_h = 0;
static int g_cx = 0;
static int g_cy = 0;
static float g_base_r = 0.0f;
static int g_orbital = 0;

static float frand(float lo, float hi) {
  return lo + (hi - lo) * (random(0, 10000) / 10000.0f);
}

static void generateOrbital(int idx) {
  for (int i = 0; i < kPoints; ++i) {
    float a = frand(0.0f, 6.28318f);
    float v = frand(0.0f, 1.0f);
    float r = g_base_r;
    if (idx == 0) {
      float g = sqrtf(-logf(fmaxf(0.0001f, 1.0f - v)));
      r *= 0.35f + 0.35f * g;
    } else if (idx == 1) {
      r *= 0.25f + 0.75f * fabsf(cosf(a));
    } else {
      r *= 0.25f + 0.75f * fabsf(cosf(2.0f * a));
    }
    g_angle[idx][i] = a;
    g_radius[idx][i] = r;
    g_glow[idx][i] = static_cast<uint8_t>(frand(90.0f, 255.0f));
  }
}

static void drawNucleus(float t) {
  for (int i = 12; i >= 1; --i) {
    uint8_t v = static_cast<uint8_t>(180 + i * 6 + 20 * sinf(t * 1.5f));
    uint16_t c = M5.Display.color565(v, v * 0.6f, 30);
    M5.Display.fillCircle(g_cx, g_cy, i, c);
  }
}

static void drawOrbitalCloud(float t) {
  int idx = g_orbital;
  float wobble = 0.12f * sinf(t * 0.8f);
  for (int i = 0; i < kPoints; ++i) {
    float a = g_angle[idx][i] + t * (0.3f + 0.1f * idx);
    float r = g_radius[idx][i] * (1.0f + wobble * sinf(a * 3.0f));
    float x = g_cx + cosf(a) * r;
    float y = g_cy + sinf(a) * r * 0.65f;
    uint8_t v = g_glow[idx][i];
    uint16_t c = M5.Display.color565(40, v, 200);
    M5.Display.drawPixel(static_cast<int>(x), static_cast<int>(y), c);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3); // Left landscape
  M5.Display.setBrightness(180);

  g_w = M5.Display.width();
  g_h = M5.Display.height();
  g_cx = g_w / 2;
  g_cy = g_h / 2;
  g_base_r = fminf(g_w, g_h) * 0.45f;

  randomSeed(esp_random());
  for (int i = 0; i < kOrbitals; ++i) {
    generateOrbital(i);
  }
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    g_orbital = (g_orbital + 1) % kOrbitals;
  }
  if (M5.BtnB.wasPressed()) {
    g_orbital = (g_orbital + kOrbitals - 1) % kOrbitals;
  }

  float t = millis() / 1000.0f;

  M5.Display.startWrite();
  M5.Display.fillScreen(TFT_BLACK);

  drawOrbitalCloud(t);
  drawNucleus(t);

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.print("A/B: orbital  ");
  const char *labels[kOrbitals] = {"1s", "2p", "3d"};
  M5.Display.print(labels[g_orbital]);

  M5.Display.endWrite();
  delay(16);
}

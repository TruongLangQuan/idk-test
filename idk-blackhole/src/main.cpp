#include <M5Unified.h>
#include <math.h>

struct Star {
  float r;
  float a;
  float speed;
  uint8_t b;
};

static constexpr int kStarCount = 90;
static Star g_stars[kStarCount];
static int g_w = 0;
static int g_h = 0;
static int g_cx = 0;
static int g_cy = 0;
static float g_event_r = 0.0f;
static float g_disk_inner = 0.0f;
static float g_disk_outer = 0.0f;
static bool g_paused = false;

static float frand(float lo, float hi) {
  return lo + (hi - lo) * (random(0, 10000) / 10000.0f);
}

static void initStars() {
  for (int i = 0; i < kStarCount; ++i) {
    float r = frand(g_event_r + 6.0f, fminf(g_w, g_h) * 0.62f);
    g_stars[i] = {
      r,
      frand(0.0f, 6.28318f),
      frand(-0.004f, 0.004f),
      static_cast<uint8_t>(frand(120.0f, 255.0f)),
    };
  }
}

static void drawGlow(float t) {
  for (int i = 0; i < 20; ++i) {
    float r = g_event_r + 2.0f + i * 0.8f;
    uint8_t v = 120 - i * 5;
    uint16_t c = M5.Display.color565(v / 2, v / 2, v);
    M5.Display.drawCircle(g_cx, g_cy, static_cast<int>(r), c);
  }

  for (int deg = 0; deg < 360; deg += 3) {
    float a = deg * 0.0174533f;
    float wobble = sinf(t * 0.7f + a * 3.0f) * 2.4f;
    float r = g_disk_inner + (g_disk_outer - g_disk_inner) * 0.5f + wobble;
    float x = g_cx + cosf(a) * r;
    float y = g_cy + sinf(a) * r * 0.55f;
    uint8_t heat = static_cast<uint8_t>(150 + 80 * sinf(a + t));
    uint16_t c = M5.Display.color565(heat, heat * 0.6f, 30);
    M5.Display.drawPixel(static_cast<int>(x), static_cast<int>(y), c);
  }
}

static void drawStars(float t) {
  for (int i = 0; i < kStarCount; ++i) {
    Star &s = g_stars[i];
    float bend = 18.0f / (s.r + 6.0f);
    float a = s.a + bend;
    float r = s.r + sinf(t * 0.6f + s.r) * 0.6f;
    float x = g_cx + cosf(a) * r;
    float y = g_cy + sinf(a) * r * 0.7f;
    if (r > g_event_r + 1.5f) {
      uint8_t v = s.b;
      uint16_t c = M5.Display.color565(v, v, v);
      M5.Display.drawPixel(static_cast<int>(x), static_cast<int>(y), c);
    }
    if (!g_paused) {
      s.a += s.speed;
      if (s.a > 6.28318f) s.a -= 6.28318f;
      if (s.a < 0.0f) s.a += 6.28318f;
    }
  }
}

static void drawDisk(float t) {
  for (int deg = 0; deg < 360; deg += 4) {
    float a = deg * 0.0174533f;
    float band = 0.5f + 0.5f * sinf(a * 2.0f + t * 1.2f);
    float r = g_disk_inner + band * (g_disk_outer - g_disk_inner);
    float x = g_cx + cosf(a) * r;
    float y = g_cy + sinf(a) * r * 0.45f;
    uint8_t heat = static_cast<uint8_t>(120 + 100 * band);
    uint16_t c = M5.Display.color565(heat, heat * 0.5f, 20);
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
  g_event_r = fminf(g_w, g_h) * 0.12f;
  g_disk_inner = g_event_r + 8.0f;
  g_disk_outer = g_event_r + 28.0f;

  randomSeed(esp_random());
  initStars();
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    initStars();
  }
  if (M5.BtnB.wasPressed()) {
    g_paused = !g_paused;
  }

  uint32_t now = millis();
  float t = now / 1000.0f;

  M5.Display.startWrite();
  M5.Display.fillScreen(TFT_BLACK);

  drawStars(t);
  drawDisk(t);
  drawGlow(t);

  M5.Display.fillCircle(g_cx, g_cy, static_cast<int>(g_event_r), TFT_BLACK);

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.print("A: reseed  B: pause");
  if (g_paused) {
    M5.Display.setCursor(4, g_h - 14);
    M5.Display.print("Paused");
  }

  M5.Display.endWrite();
  delay(16);
}

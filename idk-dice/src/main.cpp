#include <M5Unified.h>
#include <cmath>

namespace {

static const uint16_t kBg = TFT_BLACK;
static const uint16_t kFg = TFT_GREEN;
static const uint16_t kAccent = TFT_RED;

uint32_t g_last_roll_ms = 0;
int g_d1 = 1;
int g_d2 = 1;
int g_d3 = 1;

void drawFrame() {
  M5.Display.fillScreen(kBg);
  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(4, 4);
  M5.Display.print("idk-dice");
  M5.Display.setTextColor(TFT_DARKGREY, kBg);
  M5.Display.setCursor(4, 20);
  M5.Display.print("Shake to roll");
}

void drawDie(int x, int y, int v) {
  M5.Display.drawRoundRect(x, y, 60, 60, 6, kFg);
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(x + 24, y + 20);
  M5.Display.print(v);
  M5.Display.setTextSize(1);
}

void drawDice() {
  drawFrame();
  drawDie(10, 40, g_d1);
  drawDie(88, 40, g_d2);
  drawDie(166, 40, g_d3);
  M5.Display.setTextColor(TFT_DARKGREY, kBg);
  M5.Display.setCursor(4, 124);
  M5.Display.print("A:Roll  B:Hold");
}

void rollDice() {
  g_d1 = 1 + (esp_random() % 6);
  g_d2 = 1 + (esp_random() % 6);
  g_d3 = 1 + (esp_random() % 6);
  drawDice();
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  drawDice();
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    rollDice();
    g_last_roll_ms = millis();
  }

  float ax = 0, ay = 0, az = 0;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    float mag = sqrtf(ax * ax + ay * ay + az * az);
    float delta = fabsf(mag - 1.0f);
    uint32_t now = millis();
    if (delta > 1.4f && (now - g_last_roll_ms) > 800) {
      rollDice();
      g_last_roll_ms = now;
    }
  }

  delay(10);
}

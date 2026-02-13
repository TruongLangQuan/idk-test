#include <M5Unified.h>
#include "generated/img_assets.h"

static size_t g_index = 0;

static inline const uint16_t* imagePtr(const uint8_t* p) {
  return reinterpret_cast<const uint16_t*>(p);
}

void drawCurrent() {
  M5.Display.fillScreen(TFT_BLACK);
  if (IMAGE_COUNT == 0) {
    M5.Display.setCursor(2, 2);
    M5.Display.print("No image compiled");
    return;
  }

  const auto& img = IMAGE_TABLE[g_index];
  M5.Display.pushImage(0, 0, img.width, img.height, imagePtr(img.rgb565));
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  drawCurrent();
}

void loop() {
  M5.update();
  if (IMAGE_COUNT == 0) {
    delay(100);
    return;
  }

  if (M5.BtnA.wasPressed()) {
    g_index = (g_index + 1) % IMAGE_COUNT;
    drawCurrent();
  }
  if (M5.BtnB.wasPressed()) {
    g_index = (g_index + IMAGE_COUNT - 1) % IMAGE_COUNT;
    drawCurrent();
  }
  delay(10);
}

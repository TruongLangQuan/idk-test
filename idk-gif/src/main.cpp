#include <M5Unified.h>
#include "generated/gif_asset.h"

static uint16_t g_frame = 0;
static uint16_t g_asset = 0;
static uint32_t g_next_ms = 0;

static inline const uint16_t* framePtr(const uint8_t* p) {
  return reinterpret_cast<const uint16_t*>(p);
}

void drawFrame(uint16_t asset_idx, uint16_t frame_idx) {
  if (GIF_ASSET_COUNT == 0) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(4, 4);
    M5.Display.print("No GIF compiled");
    return;
  }
  const auto& asset = GIF_ASSETS[asset_idx % GIF_ASSET_COUNT];
  if (asset.frame_count == 0) return;
  const auto& fr = asset.frames[frame_idx % asset.frame_count];
  M5.Display.pushImage(0, 0, GIF_WIDTH, GIF_HEIGHT, framePtr(fr.rgb565));
  g_next_ms = millis() + (fr.delay_ms ? fr.delay_ms : 100);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.fillScreen(TFT_BLACK);
  drawFrame(0, 0);
}

void loop() {
  M5.update();
  if (GIF_ASSET_COUNT == 0) {
    delay(100);
    return;
  }

  if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
    g_asset = (g_asset + GIF_ASSET_COUNT - 1) % GIF_ASSET_COUNT;
    g_frame = 0;
    drawFrame(g_asset, g_frame);
  }

  const auto& asset = GIF_ASSETS[g_asset];
  if (asset.frame_count == 0) {
    delay(50);
    return;
  }
  if ((int32_t)(millis() - g_next_ms) >= 0) {
    g_frame = (g_frame + 1) % asset.frame_count;
    drawFrame(g_asset, g_frame);
  }
  delay(1);
}

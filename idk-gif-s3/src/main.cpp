#include <Arduino.h>
#include <TFT_eSPI.h>
#include "generated/gif_asset.h"

#ifndef TFT_I2C_POWER
#define TFT_I2C_POWER 21
#endif

static TFT_eSPI tft = TFT_eSPI();

static uint16_t g_frame = 0;
static uint16_t g_asset = 0;
static uint32_t g_next_ms = 0;
static uint32_t g_last_btn_ms = 0;
static bool g_last_btn = true;

static inline const uint16_t* framePtr(const uint8_t* p) {
  return reinterpret_cast<const uint16_t*>(p);
}

static void enableDisplayPower() {
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
}

static void enableBacklight() {
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  #ifdef TFT_BACKLIGHT_ON
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  #else
  digitalWrite(TFT_BL, HIGH);
  #endif
#endif
}

static void drawFrame(uint16_t asset_idx, uint16_t frame_idx) {
  if (GIF_ASSET_COUNT == 0) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(4, 4);
    tft.print("No GIF compiled");
    return;
  }

  const auto& asset = GIF_ASSETS[asset_idx % GIF_ASSET_COUNT];
  if (asset.frame_count == 0) {
    return;
  }

  const auto& fr = asset.frames[frame_idx % asset.frame_count];
  tft.pushImage(0, 0, GIF_WIDTH, GIF_HEIGHT, framePtr(fr.rgb565));
  (void)fr;
  g_next_ms = millis() + (fr.delay_ms ? fr.delay_ms : 100);
}

void setup() {
  enableDisplayPower();
  enableBacklight();

  tft.init();
  tft.setSwapBytes(false);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  drawFrame(0, 0);

  pinMode(0, INPUT_PULLUP); // BOOT button
  g_last_btn = digitalRead(0);
}

void loop() {
  if (GIF_ASSET_COUNT == 0) {
    delay(100);
    return;
  }

  const uint32_t now = millis();
  const bool btn = digitalRead(0);
  if (g_last_btn && !btn && (now - g_last_btn_ms) > 200) {
    g_last_btn_ms = now;
    g_asset = (g_asset + GIF_ASSET_COUNT - 1) % GIF_ASSET_COUNT;
    g_frame = 0;
    drawFrame(g_asset, g_frame);
  }
  g_last_btn = btn;

  const auto& asset = GIF_ASSETS[g_asset];
  if (asset.frame_count == 0) {
    delay(50);
    return;
  }

  if ((int32_t)(now - g_next_ms) >= 0) {
    g_frame = (g_frame + 1) % asset.frame_count;
    drawFrame(g_asset, g_frame);
  }

  delay(1);
}

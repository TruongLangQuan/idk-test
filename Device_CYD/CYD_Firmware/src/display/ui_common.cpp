#include "ui_common.h"
#include "../config.h"
#include "../hardware/battery.h"
#include "../hardware/sdcard.h"
#include "../network/wifi_manager.h"
#include "../media/builtin_assets.h"
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();
int ui_rotation = 3;

void ui_init() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW); // Keep off during init
    
    tft.begin();
    tft.writecommand(0x01); // Software Reset
    delay(150);
    
    // RGB Confirmation Test
    tft.setRotation(0);
    tft.fillScreen(TFT_RED); delay(200);
    tft.fillScreen(TFT_GREEN); delay(200);
    tft.fillScreen(TFT_BLUE); delay(200);
    
    tft.setRotation(3); // Native Landscape for many CYDs
    tft.fillScreen(COLOR_BG);
    digitalWrite(TFT_BL, HIGH); // Backlight ON
    
    DEBUG_PRINTF("ULTIMATE INIT. Rotation: %d, W: %d, H: %d\n", tft.getRotation(), tft.width(), tft.height());
}

void ui_clear() {
    tft.fillScreen(COLOR_BG);
}

void ui_draw_status_bar() {
    tft.fillRect(0, 0, SCREEN_WIDTH, 22, COLOR_BG); // Clear status bar
    
    // WiFi Icon
    if (wifi_is_ap_mode()) {
        tft.drawBitmap(5, 5, image_wifi_full_bits, 19, 16, COLOR_ACCENT);
        tft.setTextColor(COLOR_ACCENT);
        tft.setTextSize(1);
        tft.drawString("AP Mode", 28, 8);
    } else if (wifi_is_connected()) {
        tft.drawBitmap(5, 5, image_wifi_full_bits, 19, 16, COLOR_TEXT);
    } else {
        tft.drawBitmap(5, 5, image_wifi_none_bits, 19, 16, 0x7BEF); // Gray
    }
    
    // SD Icon
    int sd_x = wifi_is_ap_mode() ? 85 : 30; // Shift if "AP Mode" text is shown
    if (sd_mounted()) {
        tft.drawBitmap(sd_x, 8, image_SDcardMounted_bits, 11, 8, COLOR_TEXT);
    }
    
    // Battery
    int batt_pct = battery_get_percent();
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.drawRightString(String(batt_pct) + "%", 285, 8, 1);
    
    uint16_t batt_color = (batt_pct < 20) ? 0xF800 : COLOR_TEXT; // Red if low
    tft.drawBitmap(289, 5, image_battery_full_bits, 24, 16, batt_color);
}
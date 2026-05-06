#include "screen1.h"
#include "ui_common.h"
#include "../config.h"
#include "../media/builtin_assets.h"
#include "../network/ntp_time.h"
#include "../network/weather.h"
#include "../hardware/battery.h"
#include "../hardware/sdcard.h"
#include "../network/wifi_manager.h"
#include "../storage/preferences.h"

static unsigned long last_time_update = 0;
static unsigned long last_weather_update = 0;

void screen1_init() {
    ui_clear();
    screen1_draw();
}

void screen1_draw() {
    ui_clear();
    
    // Icons and Status (handled by ui_draw_status_bar mostly, but aligning with Lopaka)
    ui_draw_status_bar();
    
    // Avatar (ellenjoenobr)
    tft.pushImage(214, 119, 102, 120, image_ellenjoenobr_pixels);
    
    // Note Box
    tft.drawRect(22, 160, 176, 72, COLOR_TEXT);
    
    screen1_update();
}

void screen1_update() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - last_time_update >= 1000) {
        last_time_update = currentMillis;
        
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.setTextSize(2);
        String timeStr = get_current_time_string();
        tft.drawString(timeStr, 56, 80);
        
        String dateStr = get_current_date_string();
        tft.drawString(dateStr, 76, 104);
        
        ui_draw_status_bar(); // Update status bar every second
    }
    
    if (currentMillis - last_weather_update >= 600000 || last_weather_update == 0) {
        last_weather_update = currentMillis;
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.setTextSize(1);
        String weatherStr = get_weather_string();
        
        if (wifi_is_ap_mode()) {
            weatherStr = "Setup: connect to " + wifi_get_ssid();
        }
        
        tft.fillRect(20, 128, 190, 16, COLOR_BG); // Clear area for weather
        tft.drawString(weatherStr, 20, 128);
    }
    
    // Update Note
    static String last_note = "";
    String current_note = prefs_get_note();
    if (current_note != last_note) {
        last_note = current_note;
        tft.fillRect(23, 161, 174, 70, COLOR_BG);
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.setTextSize(1);
        tft.setCursor(26, 164);
        tft.print(current_note);
    }
}
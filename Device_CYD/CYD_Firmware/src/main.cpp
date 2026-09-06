#include <Arduino.h>
#include "config.h"
#include "display/ui_common.h"
#include "display/screen1.h"
#include "display/screen2.h"
#include "display/screen3.h"
#include "hardware/battery.h"
#include "hardware/sdcard.h"
#include "hardware/touch.h"
#include "network/wifi_manager.h"
#include "network/ntp_time.h"
#include "network/weather.h"
#include "storage/preferences.h"
#include "storage/file_browser.h"
#include "webui/webserver.h"

static int current_screen = 1;
SemaphoreHandle_t displayMutex;

#ifndef USER_SETUP_LOADED
#error "CRITICAL: build_flags in platformio.ini are being ignored! Orientation will be broken."
#endif

void switch_screen(int screen) {
    if (displayMutex != NULL) xSemaphoreTake(displayMutex, portMAX_DELAY);
    current_screen = screen;
    if (screen == 1) screen1_init();
    else if (screen == 2) screen2_init();
    else if (screen == 3) screen3_init();
    if (displayMutex != NULL) xSemaphoreGive(displayMutex);
}

void task_core0(void *pvParameters) {
    wifi_init();
    ntp_init();
    webserver_init();
    
    for(;;) {
        wifi_update();
        ntp_update();
        weather_update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void task_core1(void *pvParameters) {
    pinMode(0, INPUT_PULLUP);
    static bool last_btn = HIGH;
    
    for(;;) {
        battery_update();
        touch_update();
        
        // Physical button fallback (BOOT button / GPIO 0)
        bool btn = digitalRead(0);
        if (btn == LOW && last_btn == HIGH) {
            // Cycle through rotations to find landscape
            ui_rotation++;
            if (ui_rotation > 3) {
                ui_rotation = 0;
                // Move to next screen after cycling all rotations
                current_screen++;
                if (current_screen > 3) current_screen = 1;
            }
            
            if (displayMutex != NULL) xSemaphoreTake(displayMutex, portMAX_DELAY);
            tft.setRotation(ui_rotation);
            DEBUG_PRINTF("Manual Rotation Change: %d. Size: %dx%d\n", ui_rotation, tft.width(), tft.height());
            if (current_screen == 1) screen1_init();
            else if (current_screen == 2) screen2_init();
            else if (current_screen == 3) screen3_init();
            if (displayMutex != NULL) xSemaphoreGive(displayMutex);
            
            vTaskDelay(pdMS_TO_TICKS(300)); // Debounce
        }
        last_btn = btn;
        
        if (displayMutex != NULL) {
            xSemaphoreTake(displayMutex, portMAX_DELAY);
            if (current_screen == 1) screen1_update();
            else if (current_screen == 2) screen2_update();
            else if (current_screen == 3) screen3_update();
            xSemaphoreGive(displayMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup() {
    Serial.begin(115200);
    DEBUG_PRINTLN("CYD Firmware Booting...");
    
    displayMutex = xSemaphoreCreateMutex();
    
    prefs_init();
    ui_init();
    battery_init();
    touch_init();
    
    if (sd_init()) {
        DEBUG_PRINTLN("SD Card mounted.");
    }
    
    set_on_swipe_left([](){
        int next = current_screen + 1;
        if(next > 3) next = 1;
        switch_screen(next);
    });
    
    set_on_swipe_right([](){
        int prev = current_screen - 1;
        if(prev < 1) prev = 3;
        switch_screen(prev);
    });
    
    set_on_tap_sd([](){
        if(current_screen == 3) {
            if (displayMutex != NULL) xSemaphoreTake(displayMutex, portMAX_DELAY);
            screen3_next_mode();
            if (displayMutex != NULL) xSemaphoreGive(displayMutex);
        } else {
            switch_screen(3); // Switch to media screen on SD tap
        }
    });
    
    set_on_double_tap([](){
        // Toggle status bar logic could go here
    });

    set_on_long_press([](){
        // Cycle brightness
        int br = prefs_get_brightness();
        br += 64;
        if (br > 255) br = 64;
        prefs_set_brightness(br);
        analogWrite(TFT_BL, br);
    });
    
    analogWrite(TFT_BL, prefs_get_brightness());
    switch_screen(1);
    
    xTaskCreatePinnedToCore(task_core0, "Task_Core0", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_core1, "Task_Core1", 8192, NULL, 1, NULL, 1);
}

void loop() {
    vTaskDelete(NULL); // Delete main loop task to save memory
}
#include "touch.h"
#include "../config.h"
#include "../display/ui_common.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

static XPT2046_Touchscreen ts(TOUCH_CS);
static SPIClass touchSPI(HSPI);

static void (*cb_swipe_left)() = nullptr;
static void (*cb_swipe_right)() = nullptr;
static void (*cb_tap_sd)() = nullptr;
static void (*cb_double_tap)() = nullptr;
static void (*cb_long_press)() = nullptr;

static bool is_pressed = false;
static int start_x = 0;
static int start_y = 0;
static unsigned long press_time = 0;
static unsigned long last_tap_time = 0;
static int tap_count = 0;

void touch_init() {
    // Some CYDs use pin 25 for Touch CS, others use 33
    // We try to start with build flag pin first
    touchSPI.begin(14, 12, 13, TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1); // Calibration rotation
    DEBUG_PRINTLN("Dual-Pin Touch Init Sequence Started.");
}

void touch_update() {
    // Simple toggle for troubleshooting: if touch fails, we could swap to 25 here
    // But for now, we just log raw values to see if SPI is alive
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        
        // Calibration mapping for Rotation 3 (Landscape)
        int x = map(p.x, 200, 3800, 0, 320);
        int y = map(p.y, 250, 3750, 0, 240);
        
        DEBUG_PRINTF("TOUCH EVENT! Raw X:%d, Y:%d | Screen X:%d, Y:%d\n", p.x, p.y, x, y);
        
        if (!is_pressed) {
            is_pressed = true;
            start_x = x;
            start_y = y;
            press_time = millis();
        }
    } else if (is_pressed) {
        is_pressed = false;
        unsigned long duration = millis() - press_time;
        
        // Since we don't have current x/y on release with this library easily,
        // we use start_x/start_y for tap location.
        
        if (duration > 1000) {
            if (cb_long_press) cb_long_press();
        } else {
            // Check for tap in top bar
            if (start_y <= 40) {
                if (cb_tap_sd) cb_tap_sd();
            } else {
                unsigned long current_time = millis();
                if (current_time - last_tap_time < 300) {
                    tap_count++;
                    if (tap_count == 2) {
                        if (cb_double_tap) cb_double_tap();
                        tap_count = 0;
                    }
                } else {
                    tap_count = 1;
                }
                last_tap_time = current_time;
            }
        }
    }
}

void set_on_swipe_left(void (*cb)()) { cb_swipe_left = cb; }
void set_on_swipe_right(void (*cb)()) { cb_swipe_right = cb; }
void set_on_tap_sd(void (*cb)()) { cb_tap_sd = cb; }
void set_on_double_tap(void (*cb)()) { cb_double_tap = cb; }
void set_on_long_press(void (*cb)()) { cb_long_press = cb; }
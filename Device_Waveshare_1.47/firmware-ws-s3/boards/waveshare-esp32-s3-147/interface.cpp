#include "core/powerSave.h"
#include <interface.h>
#include <SD.h>

void _setup_gpio() {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_CENTER, INPUT_PULLUP);

    // Setup SD_MMC pins for waveshare 1.47"
    // CLK=14, CMD=15, D0=16, D1=18, D2=17, D3=21
    SD.setPins(14, 15, 16);
}

void _post_setup_gpio() {}

int getBattery() { return 100; } // No battery monitor

void _setBrightness(uint8_t brightval) {
    // TFT_BL is 46, handled by TFT_eSPI or backlight PWM
}

void InputHandler(void) {
    checkPowerSaveTime();
    PrevPress = false;
    NextPress = false;
    SelPress = false;
    AnyKeyPress = false;
    EscPress = false;

    static unsigned long esc_tm = millis();
    static bool esc_armed = false;

    bool up = (digitalRead(BTN_UP) == LOW);
    bool down = (digitalRead(BTN_DOWN) == LOW);
    bool left = (digitalRead(BTN_LEFT) == LOW);
    bool right = (digitalRead(BTN_RIGHT) == LOW);
    bool center = (digitalRead(BTN_CENTER) == LOW);

    if (up || down || left || right || center) {
        if (!wakeUpScreen()) AnyKeyPress = true;
        else goto END;
    }
    
    if (left && center) {
        EscPress = true;
        goto END;
    }

    if (left) { 
        PrevPress = true; 
        if (!esc_armed) {
            esc_tm = millis();
            esc_armed = true;
        }
    } else {
        esc_armed = false;
    }

    if (esc_armed && (millis() - esc_tm > 800)) {
        esc_armed = false;
        esc_tm = millis();
        PrevPress = false;
        EscPress = true;
    }

    if (right) { NextPress = true; }
    if (up) { 
        UpPress = true;
        PrevPagePress = true;
    }
    if (down) { 
        DownPress = true;
        NextPagePress = true;
    }
    if (center) { SelPress = true; }

END:
    if (AnyKeyPress) {
        long tmp = millis();
        while ((millis() - tmp) < 200 && (digitalRead(BTN_UP) == LOW || digitalRead(BTN_DOWN) == LOW || digitalRead(BTN_LEFT) == LOW || digitalRead(BTN_RIGHT) == LOW || digitalRead(BTN_CENTER) == LOW));
    }
}

// String keyboard(String mytext, int maxSize, String msg) { return mytext; }

void powerOff() {}

void checkReboot() {}

#include "idf/launcher_platform.h"
#include "powerSave.h"
#include <Arduino.h>
#include <SD_MMC.h>
#include <interface.h>

// K1 5-way Tactile Switch Pins
constexpr int kPinK1_UP     = 0;
constexpr int kPinK1_DOWN   = 2;
constexpr int kPinK1_LEFT   = 3;
constexpr int kPinK1_RIGHT  = 4;
constexpr int kPinK1_PRESS  = 5;

void _setup_gpio() {
#ifdef USE_SD_MMC
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
#endif

    pinMode(TFT_BL, OUTPUT);
    launcherGpioWrite(TFT_BL, HIGH);

    launcherGpioOutput(TFT_CS);
    launcherGpioWrite(TFT_CS, HIGH);

    launcherGpioOutput(TFT_DC);
    launcherGpioWrite(TFT_DC, HIGH);

    launcherGpioOutput(TFT_RST);
    launcherGpioWrite(TFT_RST, HIGH);
    launcherDelayMs(10);
    launcherGpioWrite(TFT_RST, LOW);
    launcherDelayMs(20);
    launcherGpioWrite(TFT_RST, HIGH);
    launcherDelayMs(120);

    launcherGpioInputPullup(kPinK1_UP);
    launcherGpioInputPullup(kPinK1_DOWN);
    launcherGpioInputPullup(kPinK1_LEFT);
    launcherGpioInputPullup(kPinK1_RIGHT);
    launcherGpioInputPullup(kPinK1_PRESS);
}

void InputHandler(void) {
    static unsigned long tm = 0;
    static unsigned long pressStartTime = 0;
    static bool wasPressed = false;
    static int lastPressedBtn = -1;
    
    checkPowerSaveTime();

    bool isUp    = (launcherGpioRead(kPinK1_UP) == LOW);
    bool isDown  = (launcherGpioRead(kPinK1_DOWN) == LOW);
    bool isLeft  = (launcherGpioRead(kPinK1_LEFT) == LOW);
    bool isRight = (launcherGpioRead(kPinK1_RIGHT) == LOW);
    bool isPress = (launcherGpioRead(kPinK1_PRESS) == LOW);

    bool anyBtnDown = isUp || isDown || isLeft || isRight || isPress;

    if (anyBtnDown && !wasPressed) {
        // Just pressed
        wasPressed = true;
        pressStartTime = launcherMillis();
        
        if (wakeUpScreen()) {
            return;
        }

        if (isUp) lastPressedBtn = kPinK1_UP;
        else if (isDown) lastPressedBtn = kPinK1_DOWN;
        else if (isLeft) lastPressedBtn = kPinK1_LEFT;
        else if (isRight) lastPressedBtn = kPinK1_RIGHT;
        else if (isPress) lastPressedBtn = kPinK1_PRESS;

    } else if (!anyBtnDown && wasPressed) {
        // Just released
        wasPressed = false;
        unsigned long holdTime = launcherMillis() - pressStartTime;
        
        AnyKeyPress = true;
        
        // Long press on LEFT or PRESS becomes EscPress to allow exiting menus
        if (holdTime > 600 && (lastPressedBtn == kPinK1_LEFT || lastPressedBtn == kPinK1_PRESS)) {
            EscPress = true;
        } else {
            // Short press (normal actions)
            if (lastPressedBtn == kPinK1_UP)    UpPress = true;
            if (lastPressedBtn == kPinK1_DOWN)  DownPress = true;
            if (lastPressedBtn == kPinK1_LEFT)  PrevPress = true;
            if (lastPressedBtn == kPinK1_RIGHT) NextPress = true;
            if (lastPressedBtn == kPinK1_PRESS) SelPress = true;
        }
        
        lastPressedBtn = -1;
    }
}

int getBattery_old() {
    int raw_adc = analogRead(1);
    if (raw_adc <= 0) return 100; // Fallback if unplugged/error
    float voltage = (raw_adc / 4095.0f) * 3.3f * 2.0f;
    if (voltage > 4.2f) voltage = 4.2f;
    if (voltage < 3.2f) voltage = 3.2f;
    int percentage = (int)(((voltage - 3.2f) / 1.0f) * 100.0f);
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    return percentage;
}

int getBattery() {
    uint32_t adc_mv = analogReadMilliVolts(1);
    float voltage = (adc_mv * 2.0f) / 1000.0f;
    if (voltage > 4.2f) voltage = 4.2f;
    if (voltage < 3.2f) voltage = 3.2f;
    int percentage = (int)(((voltage - 3.2f) / 1.0f) * 100.0f);
    if (percentage > 100) percentage = 100;
    if (percentage <= 0) percentage = 1; // Don't return 0 or it hides the icon
    
    // Quick hardware setup for GPIO 1 ADC:
    static bool init_adc = false;
    if(!init_adc) {
        pinMode(1, INPUT);
        init_adc = true;
    }
    
    return percentage;
}

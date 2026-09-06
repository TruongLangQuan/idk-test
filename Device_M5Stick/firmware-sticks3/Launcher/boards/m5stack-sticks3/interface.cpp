#include <Arduino.h>
#include "idf/launcher_platform.h"
#include "powerSave.h"
#include <M5Unified.h>
#include <Wire.h>
#include <interface.h>

// Independent I2C bus for CardKB on external Grove port (pins 9 and 10)
TwoWire CardKBWire(1);

constexpr uint32_t kBtnBDoublePressWindowMs = 270;
constexpr uint32_t kBtnBLongPressMs = 500;

bool checkCardKB() {
    CardKBWire.beginTransmission(0x5F);
    return (CardKBWire.endTransmission() == 0);
}

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    M5.begin();
    // Enable 5V output to external port to power CardKB
    M5.Power.setExtOutput(true);
    
    // Initialize 5-way tactile switch pins
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    pinMode(43, INPUT_PULLUP);

    // Initialize I2C for CardKB on Grove port pins (SDA=9, SCL=10)
    CardKBWire.begin(9, 10, 100000UL);

    /*
  | Device  | SCK   | MISO  | MOSI  | CS    | GDO0/CE   |
  | ---     | :---: | :---: | :---: | :---: | :---:     |
  | SD Card | 5     | 4     | 6     | 7     | ---       |
  | CC1101  | 5     | 4     | 6     | 2     | 3         |
  | NRF24   | 5     | 4     | 6     | 8     | 1         |
  | PN532   | 5     | 4     | 6     | 43    | --        |
  | WS500   | 5     | 4     | 6     | **    | **        |
  | LoRa    | 5     | 4     | 6     | **    | **        |
      */
    launcherGpioOutput(7);
    launcherGpioWrite(7, HIGH); // SD Card CS
    // launcherGpioOutput(2);
    // launcherGpioWrite(2, HIGH); // CC1101 CS
    // launcherGpioOutput(8);
    // launcherGpioWrite(8, HIGH); // nRF24L01 CS
    // launcherGpioOutput(43);
    // launcherGpioWrite(43, HIGH); // PN532 CS
    launcherGpioOutput(9);
    launcherGpioWrite(9, LOW); // M5RF433 avoid Jamming
    launcherGpioOutput(46);
    launcherGpioWrite(46, LOW); // Infrared LED Off

    M5.BtnA.setDebounceThresh(8);
    M5.BtnB.setDebounceThresh(8);
    M5.BtnB.setHoldThresh(kBtnBLongPressMs);
}
/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) { M5.Display.setBrightness(brightval); }

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() {
    static int lastState = -1;
    bool charging = M5.Power.isCharging();
    if (charging && lastState != 1) {
        lastState = 1;
        M5.Power.setExtOutput(false);
    } else if (!charging && lastState != 0) {
        lastState = 0;
        M5.Power.setExtOutput(true);
    }
    int level = M5.Power.getBatteryLevel();
    return (level < 0) ? 0 : (level >= 100) ? 100 : level;
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    // Poll CardKB over CardKBWire
    static bool cardkb_checked = false;
    static bool cardkb_online = false;
    static unsigned long last_cardkb_check = 0;
    unsigned long now_ms = millis();
    
    if (!cardkb_checked || now_ms - last_cardkb_check > 2000) {
        cardkb_checked = true;
        cardkb_online = checkCardKB();
        last_cardkb_check = now_ms;
    }
    
    if (cardkb_online) {
        CardKBWire.requestFrom((uint8_t)0x5F, (uint8_t)1);
        if (CardKBWire.available()) {
            uint8_t key = CardKBWire.read();
            if (key != 0) {
                AnyKeyPress = true;
                wakeUpScreen();
                
                KeyStroke.Clear();
                KeyStroke.pressed = true;
                
                if (key == 0x08) { // Backspace
                    KeyStroke.del = true;
                } else if (key == 0x0D || key == 0x0A) { // Enter
                    KeyStroke.enter = true;
                    SelPress = true;
                } else if (key == 0x1B) { // Escape
                    KeyStroke.exit_key = true;
                    EscPress = true;
                } else if (key == 0xB5) { // Arrow Up
                    UpPress = true;
                } else if (key == 0xB6) { // Arrow Down
                    DownPress = true;
                } else if (key == 0xB4) { // Arrow Left
                    PrevPress = true;
                } else if (key == 0xB7) { // Arrow Right
                    NextPress = true;
                } else {
                    KeyStroke.word.push_back((char)key);
                }
            }
        }
    }

    bool up5way = (digitalRead(1) == LOW);
    bool down5way = (digitalRead(2) == LOW);
    bool left5way = (digitalRead(3) == LOW);
    bool right5way = (digitalRead(8) == LOW);
    bool center5way = (digitalRead(43) == LOW);

    if (up5way || down5way || left5way || right5way || center5way) {
        AnyKeyPress = true;
        if (wakeUpScreen()) {
            delay(200);
            return;
        }
        
        static unsigned long last_5way_press = 0;
        unsigned long now_5way = millis();
        if (now_5way - last_5way_press > 200) {
            last_5way_press = now_5way;
            if (up5way) UpPress = true;      // Physical UP is software UP
            if (down5way) DownPress = true;  // Physical DOWN is software DOWN
            if (left5way) PrevPress = true;  // Physical LEFT is software LEFT
            if (right5way) NextPress = true; // Physical RIGHT is software RIGHT
            if (center5way) SelPress = true; // Physical CENTER is software SELECT
        }
        return;
    }

    static uint32_t btnBFirstReleaseMs = 0;
    static bool btnBWaitingSecondClick = false;
    static bool btnBLongPressFired = false;

    M5.update();

    bool emitNext = false;
    bool emitPrev = false;
    bool emitEsc = false;
    uint32_t now = launcherMillis();
    bool btnAActive = M5.BtnA.isPressed() || M5.BtnA.isHolding();
    bool btnBActive = M5.BtnB.isPressed() || M5.BtnB.isHolding();

    if (M5.BtnB.wasPressed()) btnBLongPressFired = false;

    if (btnBActive && !btnBLongPressFired && M5.BtnB.pressedFor(kBtnBLongPressMs)) {
        btnBLongPressFired = true;
        btnBWaitingSecondClick = false;
        emitEsc = true;
    }

    if (M5.BtnB.wasReleased()) {
        if (btnBLongPressFired) {
            btnBLongPressFired = false;
        } else if (btnBWaitingSecondClick && now - btnBFirstReleaseMs <= kBtnBDoublePressWindowMs) {
            btnBWaitingSecondClick = false;
            emitPrev = true;
        } else {
            btnBWaitingSecondClick = true;
            btnBFirstReleaseMs = now;
        }
    }

    if (btnBWaitingSecondClick && !btnBActive && now - btnBFirstReleaseMs > kBtnBDoublePressWindowMs) {
        btnBWaitingSecondClick = false;
        emitNext = true;
    }

    AnyKeyPress = btnAActive || btnBActive || btnBWaitingSecondClick || M5.BtnA.wasClicked() || emitNext ||
                  emitPrev || emitEsc;
    if (!AnyKeyPress) return;

    if ((btnAActive || btnBActive) && wakeUpScreen()) return;

    if (M5.BtnA.wasClicked()) SelPress = true;
    if (emitNext) NextPress = true;
    if (emitPrev) PrevPress = true;
    if (emitEsc) EscPress = true;
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() { M5.Power.powerOff(); }

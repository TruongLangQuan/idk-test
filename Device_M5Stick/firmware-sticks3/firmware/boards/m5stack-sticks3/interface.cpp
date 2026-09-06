#include <Arduino.h>
#include <globals.h>
#include "core/powerSave.h"
#include "core/utils.h"
#include <M5Unified.h>
#include <interface.h>
#include <Wire.h>

// Independent I2C bus for CardKB on external Grove port (pins 9 and 10)
TwoWire CardKBWire(1);

bool checkCardKB() {
    CardKBWire.beginTransmission(0x5F);
    return (CardKBWire.endTransmission() == 0);
}

#define TFT_BRIGHT_CHANNEL 0
#define TFT_BRIGHT_Bits 8
#define TFT_BRIGHT_FREQ 5000

constexpr uint32_t kDwDoublePressWindowMs = 250;
constexpr uint32_t kDwLongPressMs = 600;
constexpr uint32_t kDwDebounceMs = 8;

static volatile uint32_t dw_last_isr_ms = 0;
static volatile uint32_t dw_press_ms = 0;
static volatile uint32_t dw_first_release_ms = 0;
static volatile bool dw_is_down = false;
static volatile bool dw_waiting = false;
static volatile bool dw_double_ready = false;
static volatile bool dw_long_seen = false;

void IRAM_ATTR isr_dw_btn() {
    uint32_t now = millis();
    if (now - dw_last_isr_ms < kDwDebounceMs) return;
    dw_last_isr_ms = now;
    bool pressed = (digitalRead(DW_BTN) == BTN_ACT);
    if (pressed) {
        dw_is_down = true;
        dw_press_ms = now;
        return;
    }

    dw_is_down = false;
    if (dw_long_seen) {
        dw_long_seen = false;
        dw_waiting = false;
        return;
    }

    if ((now - dw_press_ms) < kDwLongPressMs) {
        if (dw_waiting && (now - dw_first_release_ms) <= kDwDoublePressWindowMs) {
            dw_double_ready = true;
            dw_waiting = false;
        } else {
            dw_waiting = true;
            dw_first_release_ms = now;
        }
    } else {
        dw_waiting = false;
    }
}
/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    M5.begin();
    Wire1.begin(47, 48);

    pinMode(SEL_BTN, INPUT);
    pinMode(DW_BTN, INPUT);

    // Enable 5V output to external port to power CardKB
    M5.Power.setExtOutput(true);
    
    // Initialize I2C for CardKB on Grove port pins (SDA=9, SCL=10)
    CardKBWire.begin(9, 10, 100000UL);

    // Initialize 5-way tactile switch pins
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    pinMode(43, INPUT_PULLUP);

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
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH); // SD Card CS
    // pinMode(2, OUTPUT);
    // digitalWrite(2, HIGH); // CC1101 CS
    // pinMode(8, OUTPUT);
    // digitalWrite(8, HIGH); // nRF24L01 CS
    // pinMode(43, OUTPUT);
    // digitalWrite(43, HIGH); // PN532 CS
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW); // M5RF433 avoid Jamming
    pinMode(46, OUTPUT);
    digitalWrite(46, LOW); // Infrared LED Off

    pinMode(SEL_BTN, INPUT_PULLUP);
    pinMode(DW_BTN, INPUT_PULLUP);
    attachInterrupt(DW_BTN, isr_dw_btn, CHANGE);
    pinMode(TFT_BL, OUTPUT);
    bruceConfig.colorInverted = 0;
    bruceConfigPins.rotation = 1; // Force rotation 1 to match idk-s3-sd-5way
}
/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
    // PWM backlight setup
    ledcAttach(TFT_BL, TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits);
    ledcWrite(TFT_BL, 250);
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    int dutyCycle;
    if (brightval == 100) dutyCycle = 250;
    else if (brightval == 75) dutyCycle = 130;
    else if (brightval == 50) dutyCycle = 70;
    else if (brightval == 25) dutyCycle = 20;
    else if (brightval == 0) dutyCycle = 5;
    else dutyCycle = ((brightval * 250) / 100);

    // Serial.printf("dutyCycle for bright 0-255: %d\n", dutyCycle);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    if (!ledcWrite(TFT_BL, dutyCycle)) {
        // Serial.println("Failed to set brightness");
        ledcDetach(TFT_BL);
        ledcAttach(TFT_BL, TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits);
        ledcWrite(TFT_BL, dutyCycle);
    }
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() {
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
        if (wakeUpScreen()) {
            delay(200);
            return;
        }
        
        static unsigned long last_5way_press = 0;
        unsigned long now_5way = millis();
        if (now_5way - last_5way_press > 200) {
            last_5way_press = now_5way;
            AnyKeyPress = true;
            if (up5way) {
                UpPress = true;   // Physical UP is software UP
            }
            if (down5way) {
                DownPress = true; // Physical DOWN is software DOWN
            }
            if (left5way) {
                PrevPress = true; // Physical LEFT is software LEFT
            }
            if (right5way) {
                NextPress = true; // Physical RIGHT is software RIGHT
            }
            if (center5way) {
                SelPress = true;  // Physical CENTER is software SELECT
            }
        }
        return;
    }

    static unsigned long tm = 0;
    static bool dwLongFired = false;
    unsigned long now = millis();
    if (now - tm < 200 && !LongPress) return;
    if (!wakeUpScreen()) AnyKeyPress = true;
    else return;

    bool selPressed = (digitalRead(SEL_BTN) == BTN_ACT);
    bool dwPressed = dw_is_down;
    bool dwWaiting = dw_waiting;
    bool dwDoubleReady = dw_double_ready;
    unsigned long dwPressStart = dw_press_ms;
    unsigned long dwFirstRelease = dw_first_release_ms;

    AnyKeyPress = selPressed || dwPressed || dwWaiting || dwDoubleReady;

    if (selPressed) {
        SelPress = true;
        tm = now;
    }
    if (dwPressed) {
        if (!dwLongFired && (now - dwPressStart) > kDwLongPressMs) {
            EscPress = true;
            dwLongFired = true;
            dw_waiting = false;
            dw_double_ready = false;
            dw_long_seen = true;
            tm = now;
        }
    } else if (dwLongFired) {
        dwLongFired = false;
    }

    if (dwDoubleReady) {
        PrevPress = true;
        dw_double_ready = false;
        dw_waiting = false;
        tm = now;
    } else if (dwWaiting && !dwPressed && (now - dwFirstRelease) > kDwDoublePressWindowMs) {
        NextPress = true;
        dw_waiting = false;
        tm = now;
    }
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() { M5.Power.powerOff(); }

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {}

bool isCharging() {
    // Strategy to stop buzzing
    static int lastState = -1;
    bool charging = M5.Power.isCharging();
    if (charging && lastState != 1) {
        lastState = 1;
        M5.Power.setExtOutput(false);
    } else if (!charging && lastState != 0) {
        lastState = 0;
        M5.Power.setExtOutput(true);
    }
    return charging;
}

/*********************************************************************
** Function: _setup_codec_speaker
** location: modules/others/audio.cpp
** Handles audio CODEC to enable/disable speaker
**********************************************************************/
static TimerHandle_t speaker_off_timer = NULL;

static void speaker_off_timer_cb(TimerHandle_t xTimer) {
    if (!speaker_off_timer) return;
    static constexpr const uint8_t disabled_bulk_data[] = {0};
    i2c_bulk_write(&Wire1, ES8311_ADDR, disabled_bulk_data); // Shutdown ES8311
    M5.In_I2C.bitOff(0x6E, 0x11, 1 << 3, 100000); // Set gpio3 output low (turn off PA)
}

void _setup_codec_speaker(bool enable) {

    static constexpr const uint8_t enabled_bulk_data[] = {
        2, 0x00, 0x80, // 0x00 RESET/  CSM POWER ON
        2, 0x01, 0xB5, // 0x01 CLOCK_MANAGER/ MCLK=BCLK
        2, 0x02, 0x18, // 0x02 CLOCK_MANAGER/ MULT_PRE=3
        2, 0x0D, 0x01, // 0x0D SYSTEM/ Power up analog circuitry
        2, 0x12, 0x00, // 0x12 SYSTEM/ power-up DAC - NOT default
        2, 0x13, 0x10, // 0x13 SYSTEM/ Enable output to HP drive - NOT default
        2, 0x32, 0xBF, // 0x32 DAC/ DAC volume (0xBF == +-0 dB )
        2, 0x37, 0x08, // 0x37 DAC/ Bypass DAC equalizer - NOT default
        0
    };

    if (speaker_off_timer == NULL) {
        speaker_off_timer = xTimerCreate("SpkOffTimer", pdMS_TO_TICKS(100), pdFALSE, (void *)0, speaker_off_timer_cb);
    }

    if (enable) {
        if (speaker_off_timer != NULL && xTimerIsTimerActive(speaker_off_timer)) {
            xTimerStop(speaker_off_timer, 0); // Cancel pending shutdown
        } else {
            i2c_bulk_write(&Wire1, ES8311_ADDR, enabled_bulk_data);
            M5.In_I2C.bitOn(0x6E, 0x11, 1 << 3, 100000); // Set gpio3 output high (turn on PA)
        }
    } else {
        if (speaker_off_timer != NULL) {
            xTimerReset(speaker_off_timer, 0); // Start/reset shutdown timeout for 100ms
        }
    }
}

/*********************************************************************
** Function: _setup_codec_mic
** location: modules/others/mic.cpp
** Handles audio CODEC to enable/disable microphone
**********************************************************************/
void _setup_codec_mic(bool enable) {
    // Set microfone pin for ADV
    mic_bclk_pin = (gpio_num_t)17;

    static constexpr const uint8_t enabled_bulk_data[] = {
        2, 0x00, 0x80, // 0x00 RESET/  CSM POWER ON
        2, 0x01, 0xBA, // 0x01 CLOCK_MANAGER/ MCLK=BCLK
        2, 0x02, 0x18, // 0x02 CLOCK_MANAGER/ MULT_PRE=3
        2, 0x0D, 0x01, // 0x0D SYSTEM/ Power up analog circuitry
        2, 0x0E, 0x02, // 0x0E SYSTEM/ : Enable analog PGA, enable ADC modulator
        2, 0x14, 0x10, // ES8311_ADC_REG14 : select Mic1p-Mic1n / PGA GAIN (minimum)
        2, 0x17, 0xBF, // ES8311_ADC_REG17 : ADC_VOLUME 0xBF == +- 0 dB
        2, 0x1C, 0x6A, // ES8311_ADC_REG1C : ADC Equalizer bypass, cancel DC offset in digital domain
        0
    };
    static constexpr const uint8_t disabled_bulk_data[] = {
        2,
        0x0D,
        0xFC, // 0x0D SYSTEM/ Power down analog circuitry
        2,
        0x0E,
        0x6A, // 0x0E SYSTEM
        2,
        0x00,
        0x00, // 0x00 RESET/  CSM POWER DOWN
        0
    };

    i2c_bulk_write(&Wire1, ES8311_ADDR, enable ? enabled_bulk_data : disabled_bulk_data);
}

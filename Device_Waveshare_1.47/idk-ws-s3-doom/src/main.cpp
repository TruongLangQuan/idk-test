#include <Arduino.h>
#include <SPI.h>
#include <SD_MMC.h>
#include <LovyanGFX.hpp>
#include "doomgeneric/doomgeneric.h"
#include "doomgeneric/doomkeys.h"

// ============================================================================
// 1. LovyanGFX Driver Configuration for Waveshare ESP32-S3-LCD-1.47B (ST7789)
// ============================================================================
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:
  LGFX(void) {
    {
      auto bus_cfg = _bus_instance.config();
      bus_cfg.spi_host   = SPI2_HOST; // FSPI/SPI2 on ESP32-S3
      bus_cfg.spi_mode   = 0;
      bus_cfg.freq_write = 40000000;
      bus_cfg.freq_read  = 16000000;
      bus_cfg.pin_sclk   = 40; // GPIO 40
      bus_cfg.pin_mosi   = 45; // GPIO 45
      bus_cfg.pin_miso   = -1;
      bus_cfg.pin_dc     = 41; // GPIO 41
      _bus_instance.config(bus_cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto panel_cfg = _panel_instance.config();
      panel_cfg.pin_cs           = 42; // GPIO 42
      panel_cfg.pin_rst          = 39; // GPIO 39
      panel_cfg.pin_busy         = -1;
      panel_cfg.panel_width      = 172;
      panel_cfg.panel_height     = 320;
      panel_cfg.offset_x         = 34; // Centering offset
      panel_cfg.offset_y         = 0;
      panel_cfg.offset_rotation  = 0;
      panel_cfg.dummy_read_bits  = 8;
      panel_cfg.readable         = false;
      panel_cfg.invert           = true;  // ST7789 Inversion ON
      panel_cfg.rgb_order        = false; // BGR
      _panel_instance.config(panel_cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX display;

// SD MMC Pins (1-bit mode)
constexpr int kPinSD_CLK    = 14;
constexpr int kPinSD_CMD    = 15;
constexpr int kPinSD_D0     = 16;

// 5-Way Tactile Switch 1
constexpr int kPinK1_UP     = 0;
constexpr int kPinK1_DOWN   = 2;
constexpr int kPinK1_LEFT   = 3;
constexpr int kPinK1_RIGHT  = 4;
constexpr int kPinK1_PRESS  = 5;

extern "C" {
  void DG_Init() {}

  void DG_DrawFrame() {
    uint16_t* src = (uint16_t*)DG_ScreenBuffer + (14 * 320);
    display.pushImage(0, 0, 320, 172, src);
  }

  void DG_SleepMs(uint32_t ms) { delay(ms); }
  uint32_t DG_GetTicksMs() { return millis(); }

  int DG_GetKey(int* pressed, unsigned char* doomKey) {
    static int last_up = 1, last_down = 1, last_left = 1, last_right = 1, last_center = 1;
    
    int up = digitalRead(kPinK1_UP);
    int down = digitalRead(kPinK1_DOWN);
    int left = digitalRead(kPinK1_LEFT);
    int right = digitalRead(kPinK1_RIGHT);
    int center = digitalRead(kPinK1_PRESS);

    if (up != last_up) { *pressed = (up == LOW); *doomKey = KEY_UPARROW; last_up = up; return 1; }
    if (down != last_down) { *pressed = (down == LOW); *doomKey = KEY_DOWNARROW; last_down = down; return 1; }
    if (left != last_left) { *pressed = (left == LOW); *doomKey = KEY_LEFTARROW; last_left = left; return 1; }
    if (right != last_right) { *pressed = (right == LOW); *doomKey = KEY_RIGHTARROW; last_right = right; return 1; }
    if (center != last_center) { *pressed = (center == LOW); *doomKey = KEY_FIRE; last_center = center; return 1; }
    return 0; 
  }

  void DG_SetWindowTitle(const char * title) {}
}

void setup() {
  Serial.begin(115200);

  // Turn on backlight manually
  pinMode(46, OUTPUT);
  digitalWrite(46, HIGH);
  
  // Hardware Reset
  pinMode(39, OUTPUT);
  digitalWrite(39, HIGH);
  delay(10);
  digitalWrite(39, LOW);
  delay(20);
  digitalWrite(39, HIGH);
  delay(120);

  pinMode(kPinK1_UP, INPUT_PULLUP);
  pinMode(kPinK1_DOWN, INPUT_PULLUP);
  pinMode(kPinK1_LEFT, INPUT_PULLUP);
  pinMode(kPinK1_RIGHT, INPUT_PULLUP);
  pinMode(kPinK1_PRESS, INPUT_PULLUP);

  display.init();
  display.setRotation(1); 
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("Initializing SD MMC...");
  
  Serial.println("Initializing SD MMC...");

  SD_MMC.setPins(kPinSD_CLK, kPinSD_CMD, kPinSD_D0);
  if (!SD_MMC.begin("/sdcard", true, false, 20000000)) { // 1-bit mode
    display.setTextColor(TFT_RED);
    display.println("SD Card Mount Failed!");
    Serial.println("SD Card Mount Failed!");
    while (1) delay(100);
  }

  display.println("Starting DOOM...");
  Serial.println("Starting DOOM...");
  
  char* argv[] = {(char*)"doom", (char*)"-iwad", (char*)"/sdcard/doom/doom1.wad", (char*)"-rgb565"};
  doomgeneric_Create(4, argv);
}

void loop() {
  doomgeneric_Tick();
}

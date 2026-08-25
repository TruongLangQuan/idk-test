//Port x86 John Hawthorn to ESP32 TTGO VGA v1.x by ackerman
//Port to DAC output to oscilloskop in X-Y mode by OzmoOzmo
//Arduino IDE 1.8.19 Espressif System 1.0.6
//March 2023 P.Sieg - removed: PS2*, OSD, WIFI + cleanups uncommented/unnused sections
//April 2023 P.Sieg - removed remaining fabgl leftovers
//Nov 2023 - Port to M5StickC Plus2 with LCD display

#include <Arduino.h>
#include <M5StickCPlus2.h>
#include "gbConfig.h"
#include "gbGlobals.h"
#include "osint.h"

// JoyC joystick

unsigned int gb_stats_time_cur_unified = 0;
unsigned int gb_stats_time_min_unified = 500000;
unsigned int gb_stats_time_max_unified = 0;
unsigned int gb_stats_video_cur_unified = 0;
unsigned int gb_stats_video_min_unified = 500000;
unsigned int gb_stats_video_max_unified = 0;
unsigned long gb_currentTime=0;
unsigned long gb_fps_time_ini_unified= 0;
unsigned long gb_fps_unified= 0;
unsigned long gb_fps_ini_unified= 0;

unsigned int gb_time_vga_before=0;
unsigned int gb_time_now=0;

void setup()
{
  // Initialize M5StickC Plus2
  M5.begin();
#if defined(STICKS3)
  M5.Lcd.setRotation(1);
#else
  M5.Lcd.setRotation(3);
#endif  // Landscape 3
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(20, 50);
  M5.Lcd.println("VECTREX");
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.setTextSize(1);
  M5.Lcd.println("Starting emulator...");
  
  #ifdef use_lib_log_serial
   Serial.begin(115200);
   Serial.println("M5StickVectrex starting...");
  #endif
  
  // Initialize 5-way switch pins
  pinMode(32, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);
  pinMode(26, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  
  delay(2000);  // Show splash screen
}


void loop()
{
  mainEmulator();
}

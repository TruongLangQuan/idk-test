#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define WIFI_SSID     "Quan Le"
#define WIFI_PASS     "15032011"
#define NTP_SERVER    "pool.ntp.org"
#define TZ_OFFSET     25200   // UTC+7 Vietnam (seconds)
#define WEATHER_API   "http://api.openweathermap.org/data/2.5/weather"
#define WEATHER_CITY  "Cat Lai,VN"
#define WEATHER_LAT   10.7551
#define WEATHER_LON   106.7764
#define WEATHER_KEY   "1f5736e44e5b924481a3a000f0b215af"

#define SD_CS 5
#define SD_MOSI 13
#define SD_MISO 12
#define SD_SCLK 14

#define BATTERY_ADC 34

#define DEBUG_ENABLE 1

#if DEBUG_ENABLE
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
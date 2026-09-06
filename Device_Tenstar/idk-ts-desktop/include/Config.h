#pragma once

#include <Arduino.h>

namespace Config {
constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 135;

constexpr int PIN_TFT_MOSI = 35;
constexpr int PIN_TFT_SCLK = 36;
constexpr int PIN_TFT_CS = 7;
constexpr int PIN_TFT_DC = 39;
constexpr int PIN_TFT_RST = 40;
constexpr int PIN_TFT_BL = 45;
constexpr int PIN_TFT_POWER = 21;

constexpr int BTN_UP = 14;
constexpr int BTN_DOWN = 17;
constexpr int BTN_LEFT = 16;
constexpr int BTN_RIGHT = 18;
constexpr int BTN_CENTER = 15;

constexpr int SD_CS = 13;
constexpr int SD_MOSI = 11;
constexpr int SD_MISO = 10;
constexpr int SD_SCK = 12;

constexpr uint32_t DEBOUNCE_MS = 45;
constexpr uint32_t UI_FRAME_MS = 80;
constexpr uint32_t STATUS_AUTOHIDE_MS = 3000;
constexpr uint32_t POMODORO_FOCUS_SEC = 25UL * 60UL;
constexpr uint32_t POMODORO_REST_SEC = 15UL * 60UL;
constexpr uint32_t WEATHER_REFRESH_MS = 10UL * 60UL * 1000UL;

constexpr const char* WIFI_SSID = "Quan Le";
constexpr const char* WIFI_PASS = "15032011";
constexpr const char* AP_SSID = "idk-ts-desktop";
constexpr const char* AP_PASS = "15032011";
constexpr const char* WEATHER_API_KEY = "";
constexpr const char* TZ_VIETNAM = "ICT-7";
constexpr float CAT_LAI_LAT = 10.7698f;
constexpr float CAT_LAI_LON = 106.7897f;

enum class ScreenState : uint8_t {
  Dashboard = 0,
  Timetable,
  Pomodoro,
  AsciiShowcase,
  MediaDisplay,
  Games,
  Settings,
  Count
};

enum class Button : uint8_t {
  Up = 0,
  Down,
  Left,
  Right,
  Center,
  Count
};
}  // namespace Config

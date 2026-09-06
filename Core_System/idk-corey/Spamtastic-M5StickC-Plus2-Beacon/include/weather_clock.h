#ifndef WEATHER_CLOCK_H
#define WEATHER_CLOCK_H

#include <M5StickCPlus2.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include "wifi_manager.h"

// Note: Copy config_template.h to src/config.h and customize your settings
// For now, we'll use these placeholder values:
#define OPENWEATHERMAP_API_KEY "YOUR_API_KEY_HERE"  // Replace with your OpenWeatherMap API key
#define WEATHER_UPDATE_INTERVAL 300000  // 5 minutes in milliseconds
#define TIME_UPDATE_INTERVAL 1000       // 1 second

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"      // Replace with your WiFi SSID
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"  // Replace with your WiFi password

// Display colors
#define COLOR_WHITE     0xFFFF
#define COLOR_BLACK     0x0000
#define COLOR_BLUE      0x001F
#define COLOR_GREEN     0x07E0
#define COLOR_RED       0xF800
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_GRAY      0x8410

struct WeatherData {
    String city;
    String description;
    float temperature;
    int humidity;
    String icon;
    bool isValid;
};

class WeatherClock {
private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    ESP32Time rtc;
    Preferences preferences;
    WiFiManager wifiManager;
    
    WeatherData currentWeather;
    unsigned long lastWeatherUpdate;
    unsigned long lastTimeUpdate;
    bool wifiConnected;
    
public:
    WeatherClock();
    
    // Core functions
    void begin();
    void update();
    void displayClock();
    void displayWeather();
    
    // WiFi functions
    bool connectToWiFi();
    bool showWiFiMenu();
    void disconnectWiFi();
    
    // Weather functions
    bool fetchWeatherData();
    void parseWeatherJson(String json);
    
    // Time functions
    void updateTime();
    String getFormattedTime();
    String getFormattedDate();
    
    // Settings
    void saveSettings();
    void loadSettings();
    
    // Status
    bool isWiFiConnected() { return wifiConnected; }
    WeatherData getWeatherData() { return currentWeather; }
};

#endif
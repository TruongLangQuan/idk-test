#include "weather.h"
#include "../config.h"
#include "../network/wifi_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

static String current_weather = "No weather data";
static unsigned long last_fetch = 0;

void weather_update() {
    if (!wifi_is_connected()) return;
    
    unsigned long currentMillis = millis();
    if (currentMillis - last_fetch >= 600000 || last_fetch == 0) {
        last_fetch = currentMillis;
        
        HTTPClient http;
        String url = String(WEATHER_API) + "?lat=" + String(WEATHER_LAT, 4) + "&lon=" + String(WEATHER_LON, 4) + "&appid=" + String(WEATHER_KEY) + "&units=metric";
        
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                float temp = doc["main"]["temp"];
                int humidity = doc["main"]["humidity"];
                float wind = doc["wind"]["speed"];
                String desc = doc["weather"][0]["description"].as<String>();
                if (desc.length() > 0) desc[0] = toupper(desc[0]); // Capitalize
                
                current_weather = String((int)temp) + "C, " + desc + " | " + String(humidity) + "% Hum | " + String(wind, 1) + "m/s";
            }
        }
        http.end();
    }
}

String get_weather_string() {
    if (last_fetch == 0) weather_update();
    return current_weather;
}
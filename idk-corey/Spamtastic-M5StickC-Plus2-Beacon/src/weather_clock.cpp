#include "weather_clock.h"

WeatherClock::WeatherClock() : timeClient(ntpUDP, "pool.ntp.org", 0, 60000) {
    wifiConnected = false;
    lastWeatherUpdate = 0;
    lastTimeUpdate = 0;
    currentWeather.isValid = false;
}

void WeatherClock::begin() {
    // Initialize preferences
    preferences.begin("weather_clock", false);
    
    // Initialize WiFi manager
    wifiManager.begin();
    
    // Load saved settings
    loadSettings();
    
    // Initialize time client
    timeClient.begin();
    
    // Don't immediately connect to WiFi - wait for user to enter weather mode
    wifiConnected = false;
}

void WeatherClock::update() {
    unsigned long currentTime = millis();
    
    // Check WiFi connection status
    wifiConnected = wifiManager.isConnected();
    
    // Update time every second
    if (currentTime - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
        updateTime();
        lastTimeUpdate = currentTime;
    }
    
    // Update weather every 5 minutes
    if (wifiConnected && (currentTime - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL)) {
        fetchWeatherData();
        lastWeatherUpdate = currentTime;
    }
}

bool WeatherClock::connectToWiFi() {
    return wifiManager.isConnected();
}

bool WeatherClock::showWiFiMenu() {
    return wifiManager.showConnectionMenu();
}

void WeatherClock::disconnectWiFi() {
    wifiManager.disconnect();
    wifiConnected = false;
}

bool WeatherClock::fetchWeatherData() {
    if (!wifiConnected) return false;
    
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=London&appid=" + String(OPENWEATHERMAP_API_KEY) + "&units=metric";
    
    http.begin(url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
        String payload = http.getString();
        parseWeatherJson(payload);
        http.end();
        return true;
    }
    
    http.end();
    return false;
}

void WeatherClock::parseWeatherJson(String json) {
    JsonDocument doc;
    deserializeJson(doc, json);
    
    if (doc["cod"] == 200) {
        currentWeather.city = doc["name"].as<String>();
        currentWeather.description = doc["weather"][0]["description"].as<String>();
        currentWeather.temperature = doc["main"]["temp"].as<float>();
        currentWeather.humidity = doc["main"]["humidity"].as<int>();
        currentWeather.icon = doc["weather"][0]["icon"].as<String>();
        currentWeather.isValid = true;
    }
}

void WeatherClock::updateTime() {
    if (wifiConnected) {
        timeClient.update();
        rtc.setTime(timeClient.getEpochTime());
    }
}

String WeatherClock::getFormattedTime() {
    return rtc.getTime("%H:%M:%S");
}

String WeatherClock::getFormattedDate() {
    return rtc.getTime("%A, %B %d");
}

void WeatherClock::displayClock() {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(COLOR_WHITE, COLOR_BLACK);
    
    // Display time
    String timeStr = getFormattedTime();
    M5.Lcd.drawString(timeStr, 10, 30);
    
    // Display date
    M5.Lcd.setTextSize(1);
    String dateStr = getFormattedDate();
    M5.Lcd.drawString(dateStr, 10, 60);
}

void WeatherClock::displayWeather() {
    if (!currentWeather.isValid) {
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(COLOR_RED, COLOR_BLACK);
        M5.Lcd.drawString("No Weather Data", 10, 90);
        return;
    }
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(COLOR_CYAN, COLOR_BLACK);
    
    // Display city
    M5.Lcd.drawString(currentWeather.city, 10, 90);
    
    // Display temperature
    String tempStr = String(currentWeather.temperature, 1) + "°C";
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(COLOR_YELLOW, COLOR_BLACK);
    M5.Lcd.drawString(tempStr, 10, 110);
    
    // Display description
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(COLOR_WHITE, COLOR_BLACK);
    M5.Lcd.drawString(currentWeather.description, 10, 140);
    
    // Display humidity
    String humStr = "Humidity: " + String(currentWeather.humidity) + "%";
    M5.Lcd.drawString(humStr, 10, 160);
}

void WeatherClock::saveSettings() {
    preferences.putBool("wifi_connected", wifiConnected);
    // Add more settings as needed
}

void WeatherClock::loadSettings() {
    wifiConnected = preferences.getBool("wifi_connected", false);
    // Load more settings as needed
}
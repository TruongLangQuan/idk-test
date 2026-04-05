#include <Arduino.h>

// TEST MODE: Reading battery via serial (display pins to be determined)
// You should see serial output on monitor

// Battery ADC pin
#define BATTERY_PIN 3

// LED pins to test (GPIO46 is common on ESP32-S3)
#define LED_PIN 46

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n=== TENSTAR TS-ESP32-S3 Battery Monitor ===");
    Serial.println("Display init skipped - serial output mode");
    Serial.println("Looking for display pin configuration...");
    
    // Setup LED
    pinMode(LED_PIN, OUTPUT);
    
    // Setup ADC for battery
    pinMode(BATTERY_PIN, INPUT);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    // Blink pattern to show device is alive
    for(int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
    
    Serial.println("Battery monitor ready");
    Serial.println("==========================================");
    Serial.println("ADC_VAL | VOLTAGE | PERCENT | STATUS");
    Serial.println("--------|---------|---------|--------");
}

void loop() {
    // Read battery voltage
    uint16_t rawValue = analogRead(BATTERY_PIN);
    
    // Convert to voltage (4.3V reference for ESP32-S3, with 2.0 voltage divider)
    float vRef = 4.3;  // ESP32-S3 VREF
    float voltage = (rawValue * vRef / 4095.0) * 2.0;  // With voltage divider
    
    // Calculate percentage (3.0V to 4.2V range)
    float voltagePercent = ((voltage - 3.0) / (4.2 - 3.0)) * 100.0;
    if (voltagePercent < 0) voltagePercent = 0;
    if (voltagePercent > 100) voltagePercent = 100;
    
    // Determine status
    const char* statusText;
    if (voltage < 3.0) {
        statusText = "CRITICAL";
    } else if (voltage < 3.3) {
        statusText = "Low";
    } else if (voltage > 4.2) {
        statusText = "Charged";
    } else {
        statusText = "OK";
    }
    
    // Print formatted output
    Serial.printf("%6d  | %5.2f V | %6d%% | %s\n", 
        rawValue, voltage, (int)voltagePercent, statusText);
    
    // Blink LED to show it's working
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    
    delay(2000);  // Update every 2 seconds
}

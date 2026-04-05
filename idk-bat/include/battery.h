#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

class BatteryMonitor {
public:
    BatteryMonitor(uint8_t adcPin = 3, uint8_t detPin = -1);
    
    // Initialize battery monitoring with calibration
    void begin();
    
    // Read raw ADC value
    uint16_t readRawVoltage();
    
    // Read battery voltage in mV
    float readBatteryVoltage();
    
    // Calculate battery percentage (using 3.0V - 4.2V range)
    uint8_t getBatteryPercent();
    
    // Check if battery is connected
    bool isBatteryConnected();
    
    // Set voltage divider ratio (default 2.0 for typical ESP32 setup)
    void setVoltageDivider(float ratio);
    
    // Set calibration values
    void setCalibration(float minVoltage = 3000.0, float maxVoltage = 4200.0);
    
private:
    uint8_t _adcPin;
    uint8_t _detPin;
    float _voltageDivider;
    float _minVoltage;
    float _maxVoltage;
    float _vRef;  // Reference voltage (typically 3.3V or 4.3V for ESP32S3)
};

#endif // BATTERY_H

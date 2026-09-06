#include "battery.h"

BatteryMonitor::BatteryMonitor(uint8_t adcPin, uint8_t detPin)
    : _adcPin(adcPin), _detPin(detPin), _voltageDivider(2.0),
      _minVoltage(3000.0), _maxVoltage(4200.0), _vRef(4.3) {
}

void BatteryMonitor::begin() {
    pinMode(_adcPin, INPUT);
    if (_detPin != (uint8_t)-1) {
        pinMode(_detPin, INPUT);
    }
    
    // Set ADC resolution to 12-bit (0-4095)
    analogReadResolution(12);
    
    // Set attenuation to 11dB for full voltage range (0-3.3V input)
    analogSetAttenuation(ADC_11db);
}

uint16_t BatteryMonitor::readRawVoltage() {
    return analogRead(_adcPin);
}

float BatteryMonitor::readBatteryVoltage() {
    uint16_t rawValue = readRawVoltage();
    // Convert raw ADC value to voltage
    // rawValue is 0-4095 for 0-4.3V
    float voltage = (float)rawValue * _vRef / 4095.0;
    
    // Apply voltage divider correction
    float batteryVoltage = voltage * _voltageDivider;
    
    return batteryVoltage * 1000.0;  // Return in mV
}

uint8_t BatteryMonitor::getBatteryPercent() {
    float voltage = readBatteryVoltage();
    
    if (voltage <= _minVoltage) {
        return 0;
    }
    if (voltage >= _maxVoltage) {
        return 100;
    }
    
    // Linear interpolation between min and max
    uint8_t percent = (uint8_t)(((voltage - _minVoltage) / (_maxVoltage - _minVoltage)) * 100.0);
    return percent;
}

bool BatteryMonitor::isBatteryConnected() {
    if (_detPin == (uint8_t)-1) {
        // If no detection pin, estimate based on voltage
        float voltage = readBatteryVoltage();
        return voltage > 2500.0;  // Battery connected if voltage > 2.5V
    }
    
    // If detection pin is available, use it
    return digitalRead(_detPin) == LOW;  // Adjust based on your schematics
}

void BatteryMonitor::setVoltageDivider(float ratio) {
    _voltageDivider = ratio;
}

void BatteryMonitor::setCalibration(float minVoltage, float maxVoltage) {
    _minVoltage = minVoltage;
    _maxVoltage = maxVoltage;
}

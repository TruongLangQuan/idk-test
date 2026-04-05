# idk-bat: Battery Monitor for TENSTAR TS-ESP32-S3

Battery monitoring system for the TENSTAR TS-ESP32-S3 Development Board with 1.14" TFT Display, BMP280 and QMI8658C sensors.

## Features

- **Battery Voltage Monitoring**: Read 3.7V LiPo battery voltage via ADC
- **Battery Percentage Calculation**: Convert voltage to percentage (3.0V - 4.2V range)
- **Battery Connection Detection**: Check if battery is connected
- **Configurable**: Support for custom voltage divider ratios and calibration
- **Serial Monitor**: Real-time battery status output

## Hardware

- **Board**: TENSTAR TS-ESP32-S3 Development Board
- **Sensors**:
  - BMP280 (Temperature & Pressure)
  - QMI8658C (Accelerometer & Gyroscope)
- **Display**: 1.14" TFT (240x135 resolution typical)
- **Battery**: 3.7V LiPo (typical capacity: 600-1000mAh)

## Pin Configuration

Default ADC pin for battery voltage: **GPIO3 (ADC1_CH2)**

You can customize the pin in `src/main.cpp`:
```cpp
BatteryMonitor battery(3);  // ADC pin number
```

## Building and Flashing

### Build
```bash
pio run -e tenstar-esp32s3
```

### Flash
```bash
pio run -e tenstar-esp32s3 -t upload
```

### Monitor
```bash
pio device monitor -b 115200
```

## Usage

The project includes a `BatteryMonitor` class for battery monitoring:

```cpp
BatteryMonitor battery(3);  // Initialize with ADC pin 3

battery.begin();  // Initialize

// Read battery voltage (mV)
float voltage = battery.readBatteryVoltage();

// Get battery percentage
uint8_t percent = battery.getBatteryPercent();

// Check if battery is connected
bool connected = battery.isBatteryConnected();
```

## Calibration

To calibrate for your specific circuit and battery:

```cpp
// Set voltage divider ratio (adjust based on your resistor divider)
battery.setVoltageDivider(2.0);

// Set min/max voltage range for 0%-100% calculation
battery.setCalibration(3000.0, 4200.0);  // 3.0V to 4.2V
```

## Notes

- The code uses ESP32-S3's ADC with 12-bit resolution
- Default ADC reference voltage: 4.3V (for ESP32-S3)
- Typical LiPo voltage range: 3.0V (empty) to 4.2V (full)
- Voltage readings are averaged over multiple samples for stability

## Future Enhancements

- [ ] Display battery status on TFT screen
- [ ] BMP280 temperature and pressure reading
- [ ] QMI8658C motion detection
- [ ] Battery discharge curve mapping
- [ ] Low battery warning/sleep mode

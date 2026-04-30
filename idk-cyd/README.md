# CYD Firmware - ESP32-2432S028R (ILI9341)

This is a lightweight firmware for the Cheap Yellow Display (CYD) ESP32 dev board with ILI9341 TFT driver.

## Features

- **Display Support**: 320x240 ILI9341 TFT
- **WiFi**: AP + STA mode
- **Web UI**: Modern control panel
- **3 Screens**: Customizable UI layouts
- **Button Controls**: Navigation between screens
- **Lightweight**: Minimal dependencies, efficient design

## Hardware

**Board**: ESP32-2432S028R (CYD)
- MCU: ESP32 (240MHz)
- Display: ILI9341 (320x240, 16-bit color)
- Touch: XPT2046
- Buttons: 2 (GPIO0, GPIO35)

## Pin Configuration

| Component | Pin |
|-----------|-----|
| TFT CS | GPIO15 |
| TFT DC | GPIO32 |
| TFT MOSI | GPIO13 |
| TFT MISO | GPIO12 |
| TFT SCLK | GPIO14 |
| TFT RST | GPIO27 |
| Button Down | GPIO0 |
| Button Up | GPIO35 |
| Battery ADC | GPIO34 |

## Building & Flashing

```bash
# Build
cd idk-cyd
pio run -e cyd-esp32-2432s028r

# Upload firmware
pio run -e cyd-esp32-2432s028r -t upload

# Monitor serial output
pio device monitor -b 115200
```

## WiFi Connection

### Access Point Mode (Default)
- SSID: `CYD_WiFi`
- Password: `12345678`
- URL: `http://192.168.4.1`

### Station Mode
Edit `src/main.cpp` and change WiFi credentials in `initWiFi()` function.

## Web UI

The control panel provides:
- Screen switching (1, 2, 3, or next)
- Device information display
- Status monitoring
- WiFi connection info

Access via: `http://192.168.4.1` (AP mode) or station IP

## Screen Layouts

**Screen 1**: Dashboard
- WiFi indicator
- Battery status
- SD card status
- Time, Date, Weather
- Note display box

**Screen 2**: Calendar/Schedule
- Weekly view
- Grid layout
- Time slots

**Screen 3**: Media Status
- GIF player status
- ASCII art player
- Screensaver info

## Customization

### Change UI Elements
Edit the `drawScreen1()`, `drawScreen2()`, `drawScreen3()` functions in `src/main.cpp`.

### Modify WiFi Settings
In `src/main.cpp`, update the `initWiFi()` function with your credentials.

### Add New Screens
1. Create `drawScreen4()` function
2. Add case in `updateDisplay()` switch statement
3. Update max screen count

## Libraries Used

- **TFT_eSPI**: TFT display driver
- **AsyncTCP**: TCP protocol library
- **ESPAsyncWebServer**: Web server library

## Troubleshooting

### Display Not Showing
- Check pin connections to ILI9341
- Verify GPIO pins in `include/pins.h`
- Ensure TFT_eSPI is properly installed

### WiFi Issues
- Check AP credentials in web UI
- Reset ESP32 if stuck
- Monitor serial output for errors

### Web UI Not Loading
- Confirm device IP address (check serial monitor)
- Try hard refresh (Ctrl+Shift+R)
- Check WiFi connection

## License

This project is part of the idk-test suite. Follow project guidelines for contributions.

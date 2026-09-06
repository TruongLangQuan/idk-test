# CYD Firmware Development Guide

This project is a custom firmware for the **Cheap Yellow Display (CYD)** / ESP32-2432S028.

## Hardware Configuration (Critical)
The following settings are applied in `platformio.ini` via `build_flags`. **Do not modify these unless you are using a different hardware revision.**

- **Driver**: ILI9341
- **Rotation**: 1 (Landscape, 320x240)
- **SPI Bus**: HSPI (Shared pins 12, 13, 14 for Display, Touch, and SD Card)
- **Touch Driver**: XPT2046 (CS Pin 33)
- **Backlight**: Pin 21 (PWM controlled)

## Build Environment
This project requires **ESP32 Arduino Core v3.x**.
- **Fixed Issue**: Switched to `mathieucarbou` forks of `ESPAsyncWebServer` and `AsyncTCP` for compatibility with Core v3.0 and `ArduinoJson 7`.
- **Dependency**: If you encounter a `ModuleNotFoundError: No module named 'yaml'`, run `pipx inject platformio pyyaml`.

## Features
1. **Home Screen**: Clock, Date, and Weather (requires WiFi).
2. **Timetable**: Customizable grid via Web UI.
3. **Media**: GIF Player (plays from `/media` on SD Card).
4. **Screensaver**: Matrix rain effect.
5. **Web UI**: Access via `http://cyd.local` or the device IP.

## Navigation
- **Touch**: Swipe left/right to change screens.
- **Button Fallback**: Press the **BOOT button** (GPIO 0) to cycle screens if touch is unavailable.
- **Long Press**: Cycle screen brightness.

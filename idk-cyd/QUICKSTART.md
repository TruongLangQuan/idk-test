# 🚀 Quick Start Guide - CYD Firmware

## Prerequisites

- Python 3.6+
- PlatformIO CLI
- ESP32 board (ESP32-2432S028R recommended)
- USB cable

## 1. Installation

```bash
# Install PlatformIO CLI if not already installed
pip install platformio

# Navigate to project
cd /home/truonglangquan/idk-code/idk-test/idk-cyd

# Install dependencies
pio pkg install -e cyd-esp32-2432s028r
```

## 2. Build

```bash
# Build firmware
pio run -e cyd-esp32-2432s028r

# Build output: `.pio/build/cyd-esp32-2432s028r/firmware.bin`
```

## 3. Flash to Device

### Option A: Direct Upload via USB

```bash
# Connect ESP32 via USB cable
# Identify USB port (usually /dev/ttyUSB0 on Linux or /dev/ttyACM0)

# Flash firmware
pio run -e cyd-esp32-2432s028r -t upload
```

### Option B: Manual Flash with esptool

```bash
# If direct upload fails, use esptool
esptool.py --port /dev/ttyUSB0 --baud 921600 --chip esp32 write_flash \
  0x1000 ~/.platformio/packages/framework-arduinoespressif32/tools/sdk/bin/bootloader_dio_40m.bin \
  0x8000 ~/.platformio/packages/framework-arduinoespressif32/tools/partition_table/partitions_two_ota.bin \
  0x10000 .pio/build/cyd-esp32-2432s028r/firmware.bin
```

## 4. Monitor Serial Output

```bash
# Open serial monitor
pio device monitor -b 115200 -p /dev/ttyUSB0
```

Look for:
```
CYD Firmware Starting...
Display initialized: 320x240
Driver: ILI9341
Starting WiFi AP...
AP IP address: 192.168.4.1
Web server started on port 80
Setup complete!
```

## 5. Connect to Device

### Via WiFi (Access Point)

1. **On your phone/computer:**
   - Open WiFi settings
   - Find network: `CYD_WiFi`
   - Password: `12345678`
   - Connect

2. **Access Web UI:**
   - Open browser
   - Go to: `http://192.168.4.1`
   - You should see the CYD Control Panel

### Via Serial Monitor

- Check displayed IP address
- Buttons control screen navigation

## 6. Control the Device

### Physical Buttons

| Button | Action |
|--------|--------|
| GPIO0 (DOWN) | Previous screen |
| GPIO35 (UP) | Next screen |

### Web UI Controls

- **Screen 1-3 Buttons**: Switch directly to any screen
- **Next Button**: Navigate forward through screens
- **Status Panel**: View device information

## 7. Customization

### Change WiFi Credentials

Edit `src/main.cpp`:

```cpp
const char* ssid = "CYD_WiFi";
const char* password = "12345678";
```

### Modify Display Content

Edit screen drawing functions in `src/main.cpp`:

```cpp
void drawScreen1() {
  // Add your UI code here
  tft.drawString("Hello", 10, 10);
}
```

### Add New Screens

1. Create new function: `void drawScreen4() { ... }`
2. Add case in `updateDisplay()`:
   ```cpp
   case 4:
     drawScreen4();
     break;
   ```
3. Update max screen check: `if (currentScreen > 4) currentScreen = 1;`

## 8. Troubleshooting

### Serial Port Not Found

```bash
# List available ports
pio device list

# Or manually check
ls /dev/ttyUSB*   # Linux
ls /dev/tty.*     # macOS
```

### Upload Fails

1. **Hold BOOT button** while uploading
2. Try different USB cable
3. Check USB driver installation
4. Use lower baud rate: Edit platformio.ini, change `upload_speed = 115200`

### Display Shows Garbage

1. Check pin connections match `include/pins.h`
2. Verify SPI bus wiring (CLK, MOSI, MISO)
3. Ensure CS and DC pins are correct
4. Try different TFT_eSPI user setup

### WiFi AP Not Appearing

1. Check serial monitor output for WiFi init messages
2. ESP32 may be in different mode - reset device
3. Try connecting via station mode (edit WiFi SSID/password)

### Web UI Not Loading

1. Ping device: `ping 192.168.4.1`
2. Check firewall settings
3. Hard refresh browser: `Ctrl+Shift+R`
4. Try different browser or device

## 9. Advanced Usage

### Change TFT Driver

If using different ILI driver, edit `platformio.ini`:

```ini
build_flags =
  -DILI9340_DRIVER  # Change this
  ...
```

### Enable Touch Support

Uncomment touch handling in `src/main.cpp` and configure XPT2046 pins.

### Add SD Card Support

Mount SD card and use AsyncWebServer to serve files.

## 10. Useful Commands

```bash
# Clean build
pio run -e cyd-esp32-2432s028r -t clean

# Full rebuild
pio run -e cyd-esp32-2432s028r --verbose

# Monitor with timestamps
pio device monitor -b 115200 --filter time

# List available environments
pio run --list-envs
```

---

**Need help?** Check [README.md](./README.md) or review `src/main.cpp` comments.

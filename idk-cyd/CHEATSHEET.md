# Quick Reference & Cheat Sheet

## Build Commands

```bash
# Navigate to project
cd /home/truonglangquan/idk-code/idk-test/idk-cyd

# Build only
pio run -e cyd-esp32-2432s028r

# Build + Upload
pio run -e cyd-esp32-2432s028r -t upload

# Monitor (serial console)
pio device monitor -b 115200

# Monitor with timestamps
pio device monitor -b 115200 --filter time

# Clean build directory
pio run -e cyd-esp32-2432s028r -t clean

# Verbose output
pio run -e cyd-esp32-2432s028r --verbose

# List environments
pio run --list-envs
```

## WiFi Connection

### Access Point Mode (Default)
```
SSID: CYD_WiFi
Password: 12345678
IP: 192.168.4.1
```

### Station Mode
Edit `include/config.h`:
```cpp
#define WIFI_SSID_STA     "YourNetwork"
#define WIFI_PASSWORD_STA "YourPassword"
```

## Pin Assignments

| Component | GPIO | Purpose |
|-----------|------|---------|
| TFT_CS | 15 | Display chip select |
| TFT_DC | 32 | Display data/command |
| TFT_MOSI | 13 | SPI data out |
| TFT_MISO | 12 | SPI data in |
| TFT_SCLK | 14 | SPI clock |
| TFT_RST | 27 | Display reset |
| BTN_DOWN | 0 | Navigation down/prev |
| BTN_UP | 35 | Navigation up/next |

## Colors (RGB565, 16-bit)

```cpp
#define COLOR_WHITE       0xFFFF  // White
#define COLOR_BLACK       0x0000  // Black
#define COLOR_RED         0xF800  // Red
#define COLOR_GREEN       0x07E0  // Green
#define COLOR_BLUE        0x001F  // Blue
#define COLOR_YELLOW      0xFFE0  // Yellow
#define COLOR_CYAN        0x07FF  // Cyan
#define COLOR_MAGENTA     0xF81F  // Magenta
#define COLOR_GRAY        0x8410  // Gray
```

## Common TFT Operations

```cpp
// Initialize
tft.init();
tft.setRotation(0);  // 0, 1, 2, 3

// Clear screen
tft.fillScreen(TFT_BLACK);

// Draw rectangle
tft.drawRect(x, y, width, height, color);

// Fill rectangle
tft.fillRect(x, y, width, height, color);

// Draw line
tft.drawLine(x1, y1, x2, y2, color);

// Draw circle
tft.drawCircle(x, y, radius, color);

// Text
tft.setTextColor(color);
tft.setTextSize(size);  // 1-7
tft.setFreeFont(font);  // NULL for default
tft.drawString("Text", x, y);
```

## Web API

### Switch Screen
```
GET /screen?n=1
GET /screen?n=2
GET /screen?n=3
```

Response:
```json
{"status":"ok","screen":1}
```

### Next Screen
```
GET /next
```

Response:
```json
{"status":"ok","screen":2}
```

### Get Status
```
GET /status
```

Response:
```json
{"screen":1,"uptime":1234}
```

### Web UI
```
GET /
```

Returns HTML control panel (index.html)

## Serial Output Format

```
CYD Firmware Starting...
Display initialized: 320x240
Driver: ILI9341
Starting WiFi AP...
AP IP address: 192.168.4.1
Web server started on port 80
Setup complete!
```

## Troubleshooting Checklist

- [ ] Device powers up (LED indicators visible)
- [ ] Display backlight turns on
- [ ] Serial output visible in monitor
- [ ] WiFi AP appears in WiFi list
- [ ] Can connect to AP with password
- [ ] Web UI loads at 192.168.4.1
- [ ] Screen changes with button press
- [ ] Screen changes with web UI
- [ ] All 3 screens display correctly

## File Locations

| File | Purpose |
|------|---------|
| `src/main.cpp` | Main firmware code |
| `include/pins.h` | GPIO definitions |
| `include/config.h` | Configuration |
| `data/index.html` | Web UI |
| `platformio.ini` | Build configuration |
| `README.md` | Full documentation |
| `QUICKSTART.md` | Build guide |
| `HARDWARE.md` | Hardware reference |

## Key Configuration Values

```cpp
// src/main.cpp
#define DISPLAY_UPDATE_INTERVAL  1000  // ms
#define BUTTON_DEBOUNCE          50    // ms
#define LONG_PRESS_TIME          1000  // ms

// WiFi
const char* ssid = "CYD_WiFi";
const char* password = "12345678";

// Web server
AsyncWebServer server(80);
```

## Common Modifications

### Change Brightness
```cpp
// Add in setup():
ledcSetup(0, 5000, 8);
ledcAttachPin(GPIO_BACKLIGHT, 0);
ledcWrite(0, 200);  // 0-255
```

### Add Touch Support
```cpp
// In config.h
#define ENABLE_TOUCH 1

// In main.cpp
#include "TouchScreen.h"
// Add touch initialization
```

### Enable SD Card
```cpp
// In platformio.ini
lib_deps = 
  ...
  SD_MMC

// In main.cpp
#include "SD_MMC.h"
SD_MMC.begin();
```

### Customize WiFi SSID/Password
Edit `include/config.h`:
```cpp
#define WIFI_SSID_AP      "MyDevice"
#define WIFI_PASSWORD_AP  "MyPassword123"
```

## Performance Metrics

| Metric | Value |
|--------|-------|
| ESP32 Speed | 240 MHz dual-core |
| Display Refresh | 1 Hz (1000ms) |
| Web Response | <10ms |
| WiFi Connection | <2 seconds |
| Flash Size | 4MB |
| RAM Available | ~200KB heap |

## GPIO Summary

```
Used (Occupied)      : GPIO 0, 2, 3, 4, 12-15, 25, 27, 32, 34-36
Available for Use    : GPIO 16-23, 26, 33
Reserved/Bootloader  : GPIO 0 (boot select), GPIO 1-3 (UART)
```

## Common Issues & Quick Fixes

| Issue | Fix |
|-------|-----|
| Serial not showing | Check USB cable, press RESET |
| Display black | Verify RST pin connection |
| WiFi AP missing | Check serial output, reset device |
| Upload fails | Hold BOOT, try lower baud rate |
| Web UI slow | Check WiFi signal, restart device |

## Next Steps After Build

1. Verify serial output matches expected log
2. Test WiFi connection and web UI
3. Press buttons to test screen navigation
4. Customize display content in drawScreen functions
5. Modify configuration as needed
6. Build final version for deployment

## Documentation Navigation

- **Quick Start**: [QUICKSTART.md](QUICKSTART.md)
- **Full Guide**: [README.md](README.md)
- **Hardware Setup**: [HARDWARE.md](HARDWARE.md)
- **Project Structure**: [STRUCTURE.md](STRUCTURE.md)
- **Dependencies**: [DEPENDENCIES.md](DEPENDENCIES.md)

---

**Need more details?** Check the full documentation in the docs folder.


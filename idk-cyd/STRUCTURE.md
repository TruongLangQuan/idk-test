# Project Structure - idk-cyd

```
idk-cyd/
├── platformio.ini          # PlatformIO project configuration
├── .gitignore              # Git ignore rules
│
├── README.md               # Main documentation
├── QUICKSTART.md           # Quick start guide
├── HARDWARE.md             # Hardware setup & pinout
│
├── src/
│   └── main.cpp            # Main firmware code
│       ├── WiFi setup
│       ├── Display init (ILI9341)
│       ├── Web server
│       └── Screen rendering (3 screens)
│
├── include/
│   ├── pins.h              # Pin definitions
│   ├── config.h            # Configuration constants
│   └── colors.h            # (optional) Color definitions
│
├── data/
│   ├── index.html          # Web UI (served via HTTP)
│   └── style.css           # (optional) External CSS
│
└── .pio/
    └── build/              # Build artifacts (auto-generated)
        └── cyd-esp32-2432s028r/
            └── firmware.bin # Compiled binary
```

## Key Files

### `src/main.cpp`
Main firmware entry point containing:
- Hardware initialization
- WiFi AP setup
- Web server endpoints
- Display rendering code
- Button handling

### `platformio.ini`
PlatformIO configuration:
- Environment: `cyd-esp32-2432s028r`
- Platform: esp32
- Libraries: TFT_eSPI, AsyncWebServer, etc.
- Build flags for ILI9341 driver

### `include/pins.h`
GPIO pin mappings:
- TFT display pins (SPI)
- Touch controller pins
- Button pins
- ADC pins

### `include/config.h`
Configuration constants:
- WiFi SSID/password
- Feature flags
- Color definitions
- Update intervals

### `data/index.html`
Web UI interface:
- Modern, responsive design
- Screen control buttons
- Device status display
- Real-time uptime tracking

## Build Output

After building:
```
.pio/build/cyd-esp32-2432s028r/
├── firmware.bin            # Main firmware binary
├── firmware.elf            # ELF file with debug symbols
├── bootloader.bin          # ESP32 bootloader
└── partitions.bin          # Partition table
```

## Build & Flash Workflow

```
src/main.cpp ──┐
include/*.h ───┼──→ [PlatformIO] ──→ firmware.bin ──→ [esptool] ──→ ESP32
platformio.ini─┘
data/*.html ───┘ (optional: uploadfs)
```

## File Modification Guide

### Safe to Edit
- ✅ `src/main.cpp` - Main logic
- ✅ `include/config.h` - Configuration
- ✅ `data/index.html` - Web UI
- ✅ Documentation files (*.md)

### Edit with Caution
- ⚠️ `include/pins.h` - Hardware dependent
- ⚠️ `platformio.ini` - Build configuration

### Do Not Edit
- ❌ `.pio/` - Auto-generated build directory
- ❌ Build artifacts

## Adding New Features

### Add Screen 4
1. Add `void drawScreen4()` in `src/main.cpp`
2. Update `NUM_SCREENS` in `include/config.h`
3. Add case in `updateDisplay()` switch
4. Update condition in button handler

### Add Web API Endpoint
1. Add handler in `setupWebServer()`:
   ```cpp
   server.on("/api/endpoint", HTTP_GET, [](AsyncWebServerRequest *request) {
     request->send(200, "application/json", "{\"status\":\"ok\"}");
   });
   ```

### Change WiFi Credentials
1. Update `WIFI_SSID_AP` and `WIFI_PASSWORD_AP` in `include/config.h`
2. Rebuild and flash

## Dependencies

### External Libraries (auto-installed by PlatformIO)
- **TFT_eSPI** - Display driver for ILI9341
- **AsyncTCP** - Async TCP protocol
- **ESPAsyncWebServer** - Non-blocking web server

### Framework
- **Arduino** - Arduino core for ESP32

## Compilation Targets

```bash
# Build only
pio run -e cyd-esp32-2432s028r

# Build + Upload
pio run -e cyd-esp32-2432s028r -t upload

# Upload filesystem (data/ folder)
pio run -e cyd-esp32-2432s028r -t uploadfs

# Clean build directory
pio run -e cyd-esp32-2432s028r -t clean

# Build with verbose output
pio run -e cyd-esp32-2432s028r --verbose
```

## Memory Layout

**Flash Memory**:
- 0x0000-0x1000: Bootloader
- 0x1000-0x8000: Partition table
- 0x8000-0x100000: SPIFFS (file system, optional)
- 0x100000+: Application code

**RAM**:
- Total: 520KB available
- Typical firmware: ~300KB
- Runtime: ~100KB (variables, buffers)

---

**Get started**: See [QUICKSTART.md](./QUICKSTART.md)

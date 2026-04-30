# Dependencies & Libraries Reference

## External Libraries

This project uses PlatformIO-managed libraries. They are automatically installed when you first build.

### TFT_eSPI
- **Author**: Bodmer
- **Repository**: https://github.com/Bodmer/TFT_eSPI
- **Version**: Latest stable
- **Purpose**: Display driver for ILI9341 TFT
- **Features**:
  - Hardware SPI support
  - Sprite buffering
  - Font rendering
  - Anti-aliased graphics
  - 16-bit color (RGB565)

### AsyncTCP
- **Author**: Me-no-dev
- **Repository**: https://github.com/me-no-dev/AsyncTCP
- **Purpose**: Non-blocking TCP protocol support
- **Features**:
  - Asynchronous socket operations
  - Event-driven architecture
  - Memory efficient

### ESPAsyncWebServer
- **Author**: Me-no-dev
- **Repository**: https://github.com/me-no-dev/ESPAsyncWebServer
- **Purpose**: High-performance web server
- **Features**:
  - Non-blocking HTTP/HTTPS
  - WebSocket support
  - Multipart form handling
  - Request routing
  - Minimal memory footprint

## Framework & Core

### Arduino Core for ESP32
- **Embedded in**: PlatformIO ESP32 platform
- **Version**: 3.20016 (as specified)
- **Components**:
  - Serial communication (UART)
  - Digital I/O (GPIO)
  - Analog input (ADC)
  - SPI bus communication
  - WiFi stack (lwip, wpa2)
  - FreeRTOS kernel

## Platform Configuration

```ini
platform = espressif32
framework = arduino
```

**Espressif32 Platform**:
- ESP32 SDK v4.4 series
- IDF tools integration
- OTA support
- Partition management

## Build Flags

```ini
-DUSER_SETUP_LOADED        # Use custom TFT_eSPI setup
-DILI9341_DRIVER           # Enable ILI9341 driver
-DTFT_WIDTH=320            # Display width
-DTFT_HEIGHT=240           # Display height
-DTFT_CS=15                # Chip select pin
-DTFT_DC=32                # Data/command pin
-DTFT_MOSI=13              # SPI MOSI pin
-DTFT_MISO=12              # SPI MISO pin
-DTFT_SCLK=14              # SPI clock pin
-DTFT_RST=27               # Reset pin
-DLOAD_FONT2               # Load font 2
-DLOAD_FONT4               # Load font 4
-DLOAD_FONT6               # Load font 6
-DLOAD_FONT7               # Load font 7
-DLOAD_FONT8               # Load font 8
```

## Memory Layout

### Flash Memory (4MB typical)
```
0x000000 - 0x001000   : Bootloader (4KB)
0x001000 - 0x008000   : Partition table (28KB)
0x008000 - 0x100000   : SPIFFS file system (992KB, optional)
0x100000 - End        : Application firmware
```

### RAM Memory (520KB total)
```
IRAM (Instruction RAM)    : ~160KB
DRAM (Data RAM)           : ~160KB
Available for heap        : ~200KB
```

**Typical Allocation**:
- Firmware code: ~300KB (flash)
- Global variables: ~10KB (RAM)
- Heap (runtime): ~100KB (RAM)
- Stack: ~10KB (RAM)

## Compilation Options

### Optimization Level
```ini
build_flags = -O2              # Default: Balanced optimization
                      # -O0: No optimization (debugging)
                      # -O3: Maximum optimization (larger binary)
```

### Serial Monitor
```ini
monitor_speed = 115200         # Baud rate for serial output
monitor_filters = esp32_exception_decoder  # Parse crash info
```

## Build Time

Typical build times:
- Clean build: 30-45 seconds
- Incremental build: 5-10 seconds
- Compilation: 20-30 seconds
- Linking: 5-10 seconds

## Upload Speed

| Method | Speed | Time |
|--------|-------|------|
| 115200 baud | ~14.4 KB/s | 20-30s |
| 460800 baud | ~57.6 KB/s | 5-10s |
| 921600 baud | ~115.2 KB/s | 3-5s (default) |

## Library Size Impact

| Library | Flash | RAM |
|---------|-------|-----|
| TFT_eSPI | ~50KB | ~2KB |
| AsyncTCP | ~15KB | ~4KB |
| ESPAsyncWebServer | ~30KB | ~6KB |
| WiFi (core) | ~80KB | ~20KB |
| **Total** | **~175KB** | **~32KB** |

**Available for app code**: ~125KB flash, ~60KB RAM

## Dependency Graph

```
main.cpp
├── Arduino.h (ESP32 core)
├── TFT_eSPI.h (Display)
│   ├── SPI.h (Hardware SPI)
│   └── pins.h (GPIO definitions)
├── WiFi.h (Network)
│   └── lwip (TCP/IP stack)
├── AsyncTCP.h (Async TCP)
│   └── lwip
└── ESPAsyncWebServer.h (Web server)
    ├── AsyncTCP.h
    └── TCP/IP stack
```

## Configuration Headers

### pins.h
Defines all GPIO pins for:
- TFT display (SPI)
- Touch controller (SPI)
- Input buttons
- ADC inputs

### config.h
Configuration constants for:
- WiFi credentials
- Feature flags
- Update intervals
- Color definitions
- Debug settings

## Optional Enhancements

### SD Card Support
Add library:
```ini
lib_deps = SD_MMC  # Or SDMMC for ESP32
```

### Touch Input
Already configured but disabled:
```cpp
#define ENABLE_TOUCH 0  // Change to 1 in config.h
```

### MQTT Support
```ini
lib_deps = PubSubClient
```

### Over-The-Air Updates
```ini
build_flags = -D OTA_UPDATE_ENABLED
```

## Troubleshooting Library Issues

### Compile Errors
- Ensure `USER_SETUP_LOADED` is defined
- Check TFT_eSPI user setup in include/pins.h
- Verify library versions match

### Memory Issues
- If "not enough memory" error: Use -O2 optimization
- Disable unused features in config.h
- Use SPIFFS to move files to flash

### Library Conflicts
- Only one web server (AsyncWebServer) recommended
- WiFi and AsyncTCP must be compatible versions
- Arduino core version must match PlatformIO version

## Update Libraries

```bash
# Check for updates
pio lib update

# Force specific library version
pio pkg install "TFT_eSPI@2.4.0"

# List installed versions
pio pkg list
```

## Performance Tuning

### Faster Compilation
```ini
build_flags = -O1           # Faster compile time
upload_speed = 921600       # Max USB speed
```

### Better Performance
```ini
build_flags = -O3           # Optimize for speed
                  -funroll-loops
                  -fno-exceptions
```

### Lower Memory Usage
```ini
build_flags = -Os           # Optimize for size
                  -fno-rtti
                  -fno-exceptions
```

## Version Reference

| Component | Version |
|-----------|---------|
| Arduino Core | 3.20016.x |
| PlatformIO | Latest |
| TFT_eSPI | 2.4.x |
| AsyncTCP | 1.2.x |
| ESPAsyncWebServer | 1.2.x |
| ESP32 SDK | v4.4.x |

---

**Need specific version info?** Run:
```bash
cd idk-cyd
pio pkg list --json-output
```

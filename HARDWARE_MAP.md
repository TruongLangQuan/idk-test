# HARDWARE_MAP

## TARGET 1: M5StickC Plus 2 (PRIMARY)
**MCU:** ESP32-PICO-V3-02 (240MHz dual-core, Wi-Fi, BT)
**Memory:** 8MB Flash, 2MB PSRAM
**Projects:** `idk-screensaver`, `idk-worldgen*`, `idk-math/*`, `idk-physics/*`, `idk-chemistry/*`, `idk-biology`, `idk-maze`, `idk-dice`, `idk-audio`, `idk-miner`, `idk-translator`, `idk-english-wordform`, `idk-geometry-dash`, `idk-motion-ascii`, `idk-sanctuaryrpg`, `idk-ascii-doom`, `idk-draw`, `idk-block-builder-3d`, `idk-ai`, `idk-gamerom`, `idk-bitnet`

| Component | Chip/Type | Protocol | Pins | Managed By |
|---|---|---|---|---|
| Display | ST7789V2 135×240 | SPI | Internal | M5Unified |
| IMU (6-axis) | QMI8658 | I2C | Internal | M5Unified |
| PMIC | AXP2101 | I2C | Internal | M5Unified |
| RTC | BM8563 | I2C | Internal | M5Unified |
| Microphone | SPM1423 | I2S | Internal | M5Unified |
| Buzzer | Passive | PWM | Internal | M5Unified |
| IR Transmitter | IR diode | GPIO | Internal | M5Unified |
| Button A (Front) | Tactile | GPIO | `G37` | M5.BtnA |
| Button B (Side) | Tactile | GPIO | `G39` | M5.BtnB |
| Button C (Power) | Tactile | GPIO | `G35` | M5.BtnPWR |
| Red LED | LED | GPIO | `G19` (inverted) | Direct |
| Grove Port | HY2.0-4P | I2C/UART/GPIO | `G32`, `G33` | Direct |

### 5-Way Tactile Switch (External — Used in most Plus2 projects)
| Direction | GPIO | Connector |
|---|---|---|
| UP | `G32` | Grove HY2.0-4 |
| DOWN | `G33` | Grove HY2.0-4 |
| LEFT | `G25` | Top header |
| RIGHT | `G26` | Top header |
| CENTER | `G0` | Top header (⚠️ boot strap pin) |

**Debounce:** Directional 100–200ms, Center 300–400ms, Boot grace 400ms (GPIO0 jitter)

---

## TARGET 2: M5StickC (Original)
**MCU:** ESP32-PICO-D4
**Memory:** 4MB Flash, no PSRAM (some variants 8MB + 2MB PSRAM)
**Projects:** `idk-gif`, `idk-txt`, `idk-img`, `idk-chess`, `idk-clock`, `idk-ir`, `idk-blackhole`, `idk-atom`, `idk-video`, `idk-portal`, `idk-linux`

| Component | Chip/Type | Protocol | Notes |
|---|---|---|---|
| Display | ST7735S 80×160 | SPI | Different from Plus2 |
| IMU | MPU6886 | I2C | Different from Plus2 |
| PMIC | AXP192 | I2C | Different from Plus2 |

⚠️ **WARNING:** Firmware for M5StickC and M5StickC Plus2 are NOT interchangeable. Wrong firmware = white/blank screen.

---

## TARGET 3: CYD (Cheap Yellow Display) — ESP32-2432S028
**MCU:** ESP32-WROOM-32
**Projects:** `idk-cyd`, `CYD_Firmware`, `idk-mine/cyd`

| Component | Chip/Type | Protocol | Pins |
|---|---|---|---|
| Display | ILI9341 320×240 | SPI | See `CYD_Firmware/User_Setup.h` |
| Touch | XPT2046 | SPI | Shared SPI bus |
| SD Card | MicroSD | SPI | Shared SPI bus |
| Audio | DAC / I2S | — | Board-specific |
| RGB LED | WS2812 | GPIO | `G16` (ASSUMPTION) |

---

## TARGET 4: Tenstar TS-ESP32-S3 + ST7789 1.14" TFT
**MCU:** ESP32-S3
**Memory:** 4MB Flash (varies)
**Projects:** `idk-ts-*`, `idk-subtitle`, `idk-gif-s3`, `idk-bat`, `idk-tenstar-remote`, `idk-s3-test`

| Component | Chip/Type | Protocol | Pin |
|---|---|---|---|
| TFT CS | — | SPI | `GPIO 6` |
| TFT DC | — | SPI | `GPIO 7` |
| TFT RST | — | — | `GPIO 8` |
| TFT BL | — | PWM | `GPIO 9` |
| TFT MOSI | — | SPI (HSPI) | `GPIO 35` |
| TFT SCLK | — | SPI (HSPI) | `GPIO 36` |
| SD CS | — | SPI | Project-specific |
| 5-Way Switch | — | GPIO | Project-specific |
| USB HID | Native USB | — | Built-in (BadUSB projects) |

**Config:** `User_Setup.h` at repo root defines TFT_eSPI pin mapping.
SPI frequency: 27MHz write, 20MHz read.

---

## TARGET 5: ESP32-WROOM-32U (Mining)
**Projects:** `idk-mine/esp32-wroom-32u`
- Bare ESP32 module for crypto/lottery operations
- No display (serial output only)

---

## ⚠️ CRITICAL DEPENDENCIES

1. **M5Unified** abstracts ALL M5StickC/Plus2 hardware. **NEVER** use raw `pinMode`/`digitalWrite` for internal peripherals.
2. **GPIO0** is a boot strap pin — needs 400ms grace period after boot. Using it for CENTER button is intentional but fragile.
3. **PSRAM** (`-DBOARD_HAS_PSRAM`) required for large sprite buffers on Plus2. Allocation failures crash the device silently.
4. **TFT_eSPI** `User_Setup.h` is target-specific. Wrong config = dead display. Each Tenstar/CYD project has its own copy.
5. **SPI bus sharing** on CYD: Display + Touch + SD share SPI. Concurrent access without mutex = corruption.

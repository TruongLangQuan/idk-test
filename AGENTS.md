# AGENTS.md

## BUILD FIRMWARE
- Each `idk-*` folder is an independent PlatformIO project.
- Run: `cd <project_dir> && pio run -e <env_name>`
- Check `platformio.ini` for exact `<env_name>`.

## COMMON ENVIRONMENTS
| Env Name | Board | Target Hardware |
|---|---|---|
| `firmware` | `m5stick-c` | M5StickC (original) |
| `m5stickc_plus2` | `m5stick-c` | M5StickC Plus2 |
| `esp32s3_114tft` | `esp32-s3-devkitc-1` | Tenstar/ESP32-S3 + ST7789 |
| `tenstar_ts_esp32s3` | `esp32-s3-devkitc-1` | Tenstar desktop firmware |
| `tenstar-esp32s3` | `esp32-s3-devkitc-1` | Tenstar battery monitor |
| `cyd` | `esp32dev` | CYD (Cheap Yellow Display) |

## FLASH DEVICE
- USB: `cd <project_dir> && pio run -e <env> --target upload`
- Filesystem: `pio run -e <env> --target uploadfs`
- OTA merge: `esptool.py --chip esp32 merge_bin -o out.bin 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin`

## TEST & MONITOR
- `pio device monitor --baud 115200`
- Exit: `Ctrl+C`

## SAFE EDIT ZONES (GREEN)
- `src/animations/*` — rendering logic
- `src/menu/*` — UI menus
- Game logic, math formulas, UI layouts
- App-level logic inside `loop()`
- Python scripts at repo root (`gen_*.py`, `convert_*.py`, `process_*.py`)
- `shared/idk_ui/` — shared UI components

## EDIT WITH CAUTION (YELLOW)
- `platformio.ini` — partition/board changes break flashing
- `tools/asset_compiler.py` — affects all asset pipelines
- `build_merge_copy.fish` — OTA build script
- `shared/idk_vi_font.h` — shared Vietnamese font (used by multiple projects)
- `idk-corey/` — third-party ports, original code conventions

## FORBIDDEN EDITS (RED — HARDWARE-CRITICAL)
- `M5Unified` / `M5GFX` library internals — use M5 API only
- Direct GPIO for display, IMU, PMIC, RTC pins — use M5.Display, M5.Imu, etc.
- `setup()` hardware init order — peripheral power must init before I2C
- `User_Setup.h` (TFT_eSPI pin mapping) — wrong pins = white/dead screen
- `idk-bin/*.bin` — generated output, never edit

## RULES FOR MODIFYING DRIVERS
- DO NOT modify third-party driver code.
- Create a wrapper class or use inheritance for custom behavior.
- Always check `HARDWARE_MAP.md` before assigning GPIO pins.
- 5-way GPIO (32,33,25,26,0) is shared across projects — do not reassign.

# AGENTS.md

## BUILD FIRMWARE
- Identify project folder containing `platformio.ini`.
- Run: `cd <project_dir> && pio run -e <env_name>`
- *Note: Check `platformio.ini` for exact `<env_name>` (e.g., `m5stickc_plus2`, `cyd`).*

## FLASH DEVICE
- Connect device via USB.
- Run: `cd <project_dir> && pio run -e <env_name> --target upload`

## TEST & MONITOR
- Run: `pio device monitor --baud 115200`
- Exit monitor: `Ctrl+C`

## SAFE EDIT ZONES (GREEN)
- `src/animations/*`
- `src/menu/*`
- Game logic, math formulas, UI layouts
- App-level logic inside `loop()`

## FORBIDDEN EDITS (HARDWARE-CRITICAL - RED)
- `platformio.ini` (Do not change board configs/partitions without explicit instruction)
- `M5Unified` or `M5GFX` library internals
- Direct GPIO manipulation for display, IMU, PMIC, RTC (Use M5 API instead)
- `setup()` block hardware initialization order

## RULES FOR MODIFYING DRIVERS
- DO NOT modify third-party driver code.
- If custom behavior is needed, create a wrapper class or use inheritance.
- Always check `HARDWARE_MAP.md` before assigning new GPIO pins.

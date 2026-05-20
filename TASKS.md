# TASKS

## Common Development Tasks & AI Instructions

### Task: Add a New ASCII Animation (`idk-ascii-idk`)
- **Files to edit:** `gen_distinct.py` or `gen_missing.py` (to generate math logic), `src/animations/` (to manually tweak).
- **Risk level:** **LOW**. 
- **Instructions:** Ensure the logic maps coordinates correctly to the 40x16 ASCII grid. Check memory usage if allocating large arrays.

### Task: Modify UI Layouts
- **Files to edit:** `shared/idk_ui/src/idk_ui.cpp` or specific project `src/ui.cpp` / `src/menu/`.
- **Risk level:** **MEDIUM**. 
- **Instructions:** Changes in `shared/idk_ui/` affect multiple projects (`idk-ai`, `idk-math`, etc.). Test thoroughly. Use `M5.Display` drawing methods.

### Task: Add Hardware Sensor / Direct GPIO Support
- **Files to edit:** `platformio.ini` (for new driver libraries), `src/main.cpp`.
- **Risk level:** **HIGH**. 
- **Instructions:** Always consult `HARDWARE_MAP.md` first. Ensure new GPIO pins do not conflict with internal M5 buses (I2C, I2S, PMIC). Do not use GPIO 0 for active-low interrupts (boot strap pin).

### Task: Port an App to Tenstar ESP32-S3
- **Files to edit:** `platformio.ini` (change env to `esp32s3_114tft`), `User_Setup.h`.
- **Risk level:** **HIGH**. 
- **Instructions:** M5Unified cannot be used on Tenstar. You must rewrite M5 API calls (`M5.Display`) to use `TFT_eSPI` directly, and handle buttons manually via `digitalRead`.

### Task: Compile New Image/GIF Assets
- **Files to edit:** Add files to `tools/assets/`, run `tools/asset_compiler.py`.
- **Risk level:** **LOW**. 
- **Instructions:** Check flash memory limits. `idk-txt` and `idk-gif` may exceed standard 4MB partitions if too many assets are compiled. Ensure `huge_app.csv` or `default_8MB.csv` is set in `platformio.ini`.

### Task: Fix Watchdog Timer (WDT) Resets
- **Files to edit:** Main calculation loops in `src/main.cpp` or engine classes.
- **Risk level:** **MEDIUM**.
- **Instructions:** Insert `vTaskDelay(1)` or `yield()` in `while` or heavy `for` loops.

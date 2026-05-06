# TASKS

## Example: Add New Sensor
- **Files to edit:** `platformio.ini` (to add library), `src/main.cpp` (to init sensor).
- **Risk level:** **MEDIUM**. Ensure I2C pins do not conflict with internal M5StickC buses.

## Example: Change Display Logic / ASCII Renderer
- **Files to edit:** `src/engine/ASCIIRenderer.cpp`, `src/engine/ASCIIRenderer.h`.
- **Risk level:** **HIGH**. Affects all animations. Frame rate drops or PSRAM allocation failures can crash the device.

## Example: Modify WiFi Config (If applicable)
- **Files to edit:** `src/network.cpp` or `src/main.cpp`.
- **Risk level:** **LOW**. Usually isolated logic.

## Example: Debug Boot Issue (Device resets immediately)
- **Files to edit:** Check `setup()` in `src/main.cpp`.
- **Risk level:** **HIGH**. Likely caused by a Watchdog Timer (WDT) timeout during heavy initialization or a null pointer dereference.

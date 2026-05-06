# WORKFLOWS

## 1. Build Firmware
1. Open terminal and navigate to the specific project directory (e.g., `cd idk-ascii-idk`).
2. Identify the environment in `platformio.ini` (e.g., `[env:m5stickc_plus2]`).
3. Run the build command:
   ```bash
   pio run -e m5stickc_plus2
   ```

## 2. Flash Process (USB)
1. Ensure the device is connected via USB.
2. Run the upload command:
   ```bash
   pio run -e m5stickc_plus2 --target upload
   ```
3. To monitor serial output immediately after:
   ```bash
   pio device monitor --baud 115200
   ```

## 3. Merge for OTA (Bmorcelli Launcher)
1. Use the `build_merge_copy.fish` script or run `esptool.py` manually to merge bootloader, partitions, and firmware:
   ```bash
   esptool.py --chip esp32 merge_bin -o output.bin 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
   ```
2. Upload `output.bin` to the SD card or flash via OTA WebUI.

## 4. Debugging Hardware
- **Display blank:** Verify `M5.begin()` is called and brightness is set (`M5.Display.setBrightness()`).
- **I2C failures:** Check if peripheral power is enabled (AXP2101 PMIC needs to turn on power rails for internal I2C devices). M5Unified handles this automatically.

## 5. Adding a New Module / Animation
1. Create a new `.h` file in the appropriate directory (e.g., `src/animations/`).
2. Inherit from the base class (e.g., `IAnimation`).
3. Implement `init()` and `update()`.
4. Register the module in the main factory loop in `main.cpp`.

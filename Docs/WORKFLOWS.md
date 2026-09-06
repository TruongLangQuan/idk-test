# WORKFLOWS

## 1. Build Firmware (PlatformIO)
1. Open terminal and navigate to the specific project directory:
   ```bash
   cd /home/truonglangquan/idk-code/idk-test/idk-maze
   ```
2. Identify the environment from the project's `platformio.ini` (e.g., `m5stickc_plus2`, `firmware`, `esp32s3_114tft`).
3. Run the build command:
   ```bash
   pio run -e <env_name>
   ```

## 2. Flash Process (USB)
1. Connect device via USB (ensure cable supports data).
2. Upload firmware:
   ```bash
   pio run -e <env_name> --target upload
   ```
3. Upload filesystem (SPIFFS/LittleFS) if the project uses data (e.g., `idk-ai`, `idk-badapple`):
   ```bash
   pio run -e <env_name> --target uploadfs
   ```

## 3. Merge for OTA (Bmorcelli Launcher)
1. To merge bootloader, partitions, and firmware into a single `.bin` for OTA or SD card booting:
   ```bash
   cd /home/truonglangquan/idk-code/idk-test
   ./build_merge_copy.fish
   ```
2. Or use `esptool.py` manually:
   ```bash
   esptool.py --chip esp32 merge_bin -o out.bin 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
   ```

## 4. Debugging Hardware
- **Blank Display:** Verify `M5.begin()` is called. PMIC (AXP192/AXP2101) must enable display power rails. M5Unified handles this automatically.
- **Boot Loop (Guru Meditation / WDT):** Ensure heavy loops yield to the RTOS using `vTaskDelay(1)` or `delay(1)`.
- **Serial Monitor:** 
  ```bash
  pio device monitor -b 115200
  ```

## 5. Compiling Assets (Images, GIFs, Text, IR)
1. Place raw assets in `/tools/assets/` subdirectories.
2. Run the compiler script:
   ```bash
   cd /home/truonglangquan/idk-code/idk-test
   source .venv/bin/activate
   python tools/asset_compiler.py all
   ```
3. The script generates C++ header files in `generated/` folders (e.g., `idk_vi_font.h`, `gif_asset.h`).

## 6. Adding a New ASCII Animation
1. Edit `gen_distinct.py` or `gen_missing.py` to add new math/logic patterns.
2. Run `python gen_distinct.py`.
3. Fix up the generated headers if needed: `python fix_gen.py`.
4. Rebuild the `idk-ascii-idk` project.

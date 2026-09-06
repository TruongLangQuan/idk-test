# TROUBLESHOOTING

## 1. White / Blank / Noisy Screen After Flash
- **Cause:** Flashing firmware built for the wrong target board (e.g., M5StickC vs M5StickC Plus2), or incorrect `User_Setup.h` pin mapping for TFT_eSPI.
- **Fix:** Check `platformio.ini` for the correct environment (`m5stickc_plus2` vs `firmware`). For Tenstar/CYD, verify MOSI, SCLK, CS, DC, and RST pins in `User_Setup.h`.

## 2. Boot Loop / Guru Meditation Error / Watchdog Reset
- **Cause:** 
  1. A `while()` loop or heavy math function took too long without yielding (WDT triggered).
  2. Null pointer dereference or array out-of-bounds (LoadProhibited).
  3. Stack overflow from allocating massive arrays inside local functions.
- **Fix:** Add `vTaskDelay(1)` to tight loops. Move large arrays to global scope or allocate them dynamically in PSRAM.

## 3. PSRAM Allocation Fails / Device Crashes on Sprite Creation
- **Cause:** The project requires PSRAM but the board doesn't have it (or it's disabled).
- **Fix:** Ensure `-DBOARD_HAS_PSRAM` is in `platformio.ini` build flags. Use `ESP.getFreePsram()` to check availability. If PSRAM is absent, reduce `LGFX_Sprite` sizes or use direct screen drawing.

## 4. Flash Error: `Timed out waiting for packet header`
- **Cause:** The ESP32 is not in bootloader mode, or the USB cable lacks data lines.
- **Fix:** Check the cable. On some boards, you may need to hold the BOOT button (GPIO 0) while plugging it in or right when esptool says "Connecting...".

## 5. Serial Port Not Detected
- **Cause:** Missing CH9102/CH34x drivers (M5StickC Plus2) or CP210x drivers.
- **Fix:** Install the correct VCP drivers for your OS.

## 6. SD Card / SPIFFS Mount Fails
- **Cause:** SPI pins conflict, or the filesystem isn't formatted/uploaded.
- **Fix:** Run `pio run -t uploadfs` to upload data. On CYD/Tenstar, verify the SD CS pin and ensure SPI frequency isn't too high (try 10MHz-20MHz for SD).

## 7. 5-Way Switch Unresponsive
- **Cause:** Incorrect wiring or button logic.
- **Fix:** Verify standard mapping (UP=32, DOWN=33, LEFT=25, RIGHT=26, CENTER=0). Note that GPIO 0 requires a 400ms delay after boot before reading, otherwise it reads jitter from the boot sequence.

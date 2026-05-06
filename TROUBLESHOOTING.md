# TROUBLESHOOTING

## 1. Flash Errors
- **Error:** `A fatal error occurred: Failed to connect to ESP32: Timed out waiting for packet header`
- **Fix:** Ensure the M5StickC is turned on and connected. You may need to hold the Power button for 6 seconds to force power off, then plug it in again. Check the USB cable (must support data).

## 2. Boot Loop / Watchdog Reset
- **Error:** Constant rebooting, serial output shows `Task watchdog got triggered` or `Guru Meditation Error: Core 1 panic'ed (LoadProhibited)`.
- **Fix:** 
  - If WDT triggered: A `while()` loop or heavy math function took too long without yielding. Add `vTaskDelay(1)` inside the loop.
  - If LoadProhibited: Null pointer dereference. Check array bounds and uninitialized objects.

## 3. Device Not Detected (Serial)
- **Error:** `/dev/ttyUSB0` or `COM port` not showing up.
- **Fix:** M5StickC Plus 2 uses a CH9102 serial chip. Ensure the CH9102/CH34x drivers are installed on the host OS.

## 4. Hardware Mismatch / White Screen
- **Error:** Display is entirely white, noisy, or blank after flash.
- **Fix:** You likely flashed firmware meant for a different board. Verify `[env:m5stickc_plus2]` is selected. M5StickC (old) and Plus 2 have different LCD controllers and PMICs.

## 5. PSRAM Allocation Fails
- **Error:** Sprites fail to create, or device crashes when allocating `M5Canvas`.
- **Fix:** Ensure `-DBOARD_HAS_PSRAM` is in `platformio.ini` build flags and that the board supports it. Check `ESP.getFreePsram()` to ensure memory isn't leaking.

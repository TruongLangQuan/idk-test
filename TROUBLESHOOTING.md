# TROUBLESHOOTING

## Flash errors
- Confirm correct `platformio.ini` env.
- Close serial monitor before upload.
- Retry with lower USB contention and correct port.

## Boot loop / jumps into app
- Project: `Launcher`
- Check `bootToApp` state in config/NVS.
- Verify startup button path still enters launcher.
- If recently changed boot flow, test with clean config and serial logs.

## Device not detected
- Verify USB cable/data path.
- Verify correct board family and drivers.
- Retry `pio device list` and monitor.

## SD card mount fails
- Verify SPI pins and card formatting.
- Cold boot and remount.
- Compare with known-good SD logic in `idk-video` or `idk-audio`.
- In Launcher, re-check `src/sd_functions.cpp` retry logic.

## Subtitle missing or wrong timing
- Verify `name.mjpg` and `name.srt` basename match.
- Verify SRT format is standard `HH:MM:SS,mmm`.
- Verify video frame cadence matches subtitle timebase.
- Verify Tenstar heartbeat/UDP on port `4210`.

## WiFi / UDP remote not working
- Verify AP/STA mode for that firmware.
- Verify SSID/password expected by peer device.
- Verify UDP ports `4210`, `4211`, `4212` are unchanged.

## Hardware mismatch
- Check target board against `platformio.ini` env.
- For Launcher, check `boards/pinouts/*` and `boards/*/interface.cpp`.
- Mark unresolved pin assumptions before editing further.

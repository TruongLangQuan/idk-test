# idk-subtitle

Subtitle receiver firmware for Tenstar TS-ESP32-S3 1.14" TFT.

## Notes
- Connects to the M5StickC Plus2 AP: SSID `dhe` / pass `dhe`
- Listens for subtitle text via UDP port `4210`
- Displays the latest subtitle line on the screen

## Build
```
pio run
```

## Flash
```
pio run -t upload
```

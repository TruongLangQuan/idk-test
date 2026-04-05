# idk-video

M5StickC Plus2 MJPEG player that reads `/video.mjpg` from SD card.

## Build & Upload
```bash
pio run
pio run -t upload
```

## Encode the video
A helper script is included to resize your source MP4 to 240x135 MJPEG.
```bash
cd tools
./encode_video.sh
```

Default settings:
- Full duration
- 10 fps
- MJPEG quality 15

You can override:
```bash
DURATION=20 FPS=12 QUALITY=12 ./encode_video.sh /path/to/video.mp4 ../data/video.mjpg
```

## Controls
Menu:
- `BtnA` select/play
- `BtnB` next
- `BtnPWR` previous (hold to go back)

Playback:
- `BtnA` restart
- `BtnB` pause/resume
- `BtnPWR` back to menu

## Notes
- Display rotation is set to left landscape (`setRotation(3)`). Change to `1` if your orientation is flipped.
- Place `video.mjpg` in SD root or upload to LittleFS (use `pio run -t uploadfs`).
- SD pins are aligned with the Bruce firmware M5StickC Plus2 config.

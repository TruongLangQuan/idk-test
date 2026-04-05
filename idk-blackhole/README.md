# idk-blackhole

M5StickC Plus2 black-hole themed visualization inspired by kavan010/black_hole.

## Controls
- `BtnA` reseeds the starfield
- `BtnB` toggles pause

## Build & Upload
```bash
pio run
pio run -t upload
```

## Notes
- Display rotation is set to left landscape (`setRotation(3)`). Change to `1` if your orientation is flipped.
- This is a lightweight real-time visual inspired by the original desktop project rather than a direct port.

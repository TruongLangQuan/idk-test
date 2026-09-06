# idk-gamerom

Console ROM browser/player for M5StickC Plus 2 with 5-way tactile switch controls.

## Features

- Load ROM files from SD card
- Display ROM info and thumbnails
- 5-way tactile switch controls (UP/DOWN: navigate, LEFT/RIGHT: select action, CENTER: start game)
- Landscape display (rotation = 3)
- Game Boy/Game Boy Color best-effort playback via PeanutGB
- SNES/SFC playback via embedded Snes9x core, video only

## ROM Storage

- Place ROM files in `/roms/` on SD card.
- Supported extensions: `.gb`, `.gbc`, `.nes`, `.sfc`, `.smc`, `.md`, `.gen`, `.gba`.
- Runtime max ROM sizes shown on the device:
  - Game Boy: 1536 KB, playable.
  - Game Boy Color: 1536 KB, best-effort playable for DMG-compatible carts.
  - NES/Famicom: 512 KB, browser only until an NES core is added.
  - SNES: 4096 KB, experimental playable.
  - Sega Genesis/Mega Drive: 4096 KB, browser only.
  - GBA: 8192 KB, browser only.

## 5-way Button Mapping

```
UP    = Navigate up in ROM list
DOWN  = Navigate down in ROM list
LEFT  = Previous ROM page
RIGHT = Next ROM page  
CENTER= Load/Start selected ROM
```

## Supported Emulators

### Current playable core
- Game Boy through `peanut_gb.h`.
- SNES/SFC through the bundled Snes9x core. Audio is disabled to fit the device.
- Other console formats are detected and documented in-app, but need emulator cores before real playback.

## Build

```bash
cd idk-gamerom
pio run -e m5stickc_plus2 -t upload
pio run -e m5stickc_plus2 -t uploadfs
pio device monitor -b 115200
```

## Hardware Requirements

- M5StickC Plus 2
- SD card (256MB minimum)
- 5-way tactile switch module (GPIO 32, 33, 25, 26, 0)
- 4MB+ Flash (uses PSRAM for emulation state)

## Notes

- `.sfc` / `.smc` files are detected as Super Nintendo ROMs and can be launched.
- SNES needs much more RAM/CPU than Game Boy. Small LoROM games have the best chance; enhancement-chip games may fail or run slowly.
- SNES audio is currently disabled.
- The bundled Snes9x-derived core is GPLv2; see `src/snes9x/src/LICENSE`.

# idk-gamerom

**NES ROM player for M5StickC Plus 2** with 5-way tactile switch controls.

## Features

- Load NES ROM files from SD card
- Display ROM info and thumbnails
- 5-way tactile switch controls (UP/DOWN: navigate, LEFT/RIGHT: select action, CENTER: start game)
- Landscape display (rotation = 3)
- Memory-efficient emulation framework (256KB+ code space)

## ROM Storage

- Place `.nes` files in `/roms/` on SD card
- ROM info stored in `/rom_metadata/` (auto-generated)

## 5-way Button Mapping

```
UP    = Navigate up in ROM list
DOWN  = Navigate down in ROM list
LEFT  = Previous ROM page
RIGHT = Next ROM page  
CENTER= Load/Start selected ROM
```

## Supported Emulators

### Phase 1 (Current): NES Support
- Uses lightweight NES emulator framework
- Planned: Integration with PicoDrive or custom minimal emulator

### Future Phases
- Game Boy (GB/GBC)
- Atari 8-bit
- Sega Master System

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

- ROM files should be authentic NES dumps (iNES format .nes)
- Large ROMs benefit from PSRAM availability
- Emulation accuracy varies based on ROM complexity

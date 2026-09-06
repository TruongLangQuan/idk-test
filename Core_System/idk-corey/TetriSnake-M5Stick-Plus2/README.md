# TetriSnake - Release Files

## Version 1.2 - Final Release

**Download**: [`TetriSnake-v1.2-MERGED.bin`](TetriSnake-v1.2-MERGED.bin) (490 KB)

### Quick Install

1. Download the merged binary above
2. Open **M5Burner**
3. Select **"M5StickC Plus2"**
4. Load binary and flash to address **`0x0`**
5. Play!

---

## Files Included

### Complete Merged Binary (Recommended)
- **`TetriSnake-v1.2-MERGED.bin`** (490 KB)
  - MD5: `b1ae66c60b75a5ab6e26861381644599`
  - Contains: Bootloader + Partition Table + Application
  - Flash to address: **0x0**
  - Ready for M5Burner

### Component Binaries
- **`TetriSnake-v1.2-APP.bin`** (426 KB) - Application only
- **`bootloader.bin`** (17 KB) - ESP32 bootloader
- **`partitions.bin`** (3 KB) - Partition table

### Metadata
- **`TetriSnake-v1.2.json`** - M5Burner configuration file
- **`checksums.md5`** - File verification checksums

---

## Installation Methods

### Method 1: M5Burner (Easiest)
```
1. Download TetriSnake-v1.2-MERGED.bin
2. Open M5Burner
3. Select M5StickC Plus2
4. Load binary → Flash to 0x0
5. Done!
```

### Method 2: esptool (Command Line)
```bash
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x0 TetriSnake-v1.2-MERGED.bin
```

**Windows**: Replace `/dev/ttyACM0` with your COM port (e.g., `COM3`)

### Method 3: PlatformIO (Developers)
```bash
cd TetriSnake-GitHub
pio run -e m5stick-c-plus2 -t upload
```

### Method 4: Manual Component Flash
```bash
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 TetriSnake-v1.2-APP.bin
```

---

## Version 1.2 Features

### New in v1.2
- ✅ **White food** - No longer blends with red Z-pieces
- ✅ **Flicker-free** - Smooth differential rendering
- ✅ **Better food spawn** - Improved algorithm with fallback

### Features from v1.1
- ✅ **Colored tetriminos** - Each type has unique color
- ✅ **Blue border** - Clear visual boundaries
- ✅ **Extended play area** - More room at bottom

### Core Features
- ✅ **Classic Snake** with Tetris obstacles
- ✅ **7 random tetriminos** each game
- ✅ **Progressive speed** - Gets faster as you eat
- ✅ **High score** tracking
- ✅ **God Mode** - Practice mode

---

## Game Controls

### Mini JoyC Hat
- **Up/Down/Left/Right**: Move snake
- Cannot reverse 180° (just like classic Snake)

### M5StickC Buttons
- **BtnA**: Restart (game over screen)
- **BtnB**: Toggle God Mode

---

## Technical Specifications

### Hardware
- **Device**: M5StickC Plus2
- **Chip**: ESP32-PICO-V3-02
- **Controller**: Mini JoyC Hat (I2C at 0x54)
- **Display**: 135×240 TFT LCD

### Software
- **Framework**: Arduino
- **Build**: PlatformIO
- **RAM**: 8.8% (28,892 bytes)
- **Flash**: 33.2% (434,889 bytes)

### Game Grid
- **Columns**: 22
- **Rows**: 35
- **Block Size**: 6×6 pixels
- **Frame Rate**: ~50 FPS

---

## File Verification

MD5 checksums for integrity checking:

```
b1ae66c60b75a5ab6e26861381644599  TetriSnake-v1.2-MERGED.bin
92c648bb1ccc1ef6d4e9d1147d90cbe8  TetriSnake-v1.2-APP.bin
6bf55d2acdd830c317f57426ce8dcfd9  bootloader.bin
a039c66cd3488176037b616b7595fe72  partitions.bin
```

Verify with: `md5sum -c checksums.md5`

---

## Troubleshooting

### Device Not Detected
- Try different USB cable (must support data)
- Check drivers (ESP32 USB drivers)
- Hold BtnA while connecting USB

### Flash Fails
- Lower baud rate: `--baud 115200`
- Verify file checksum
- Try different USB port

### Game Won't Start
- Press BtnA on title screen
- Power cycle device
- Reflash binary

### Joystick Issues
- Check I2C connection (pins 0, 26)
- Verify JoyC Hat is seated properly
- Test in God Mode (BtnB)

---

## Version History

### v1.2.0 (2025-11-08) - Final ✅
- White food for visibility
- Flicker-free rendering
- Improved food spawning

### v1.1.0 (2025-11-08) - Visual Polish ✨
- Colored tetriminos
- Blue border
- Extended play area

### v1.0.0 (2025-11-08) - Initial Release 🎉
- Core Snake gameplay
- Random tetrimino obstacles
- Progressive difficulty
- God Mode

---

## Support

- 📖 [Main README](../README.md)
- 📝 [Changelog](../CHANGELOG.md)
- 🐛 [Report Issues](https://github.com/yourusername/TetriSnake/issues)

---

**Ready to play!** 🐍✨

Flash the merged binary and navigate through the tetrimino maze!

# 🎮 Columns M5 - M5Burner Release Package v1.0.0

**Classic Sega Columns match-3 puzzle game for M5StickC Plus2**

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Device](https://img.shields.io/badge/device-M5StickC%20Plus2-orange)
![Status](https://img.shields.io/badge/status-stable-green)

---

## 📦 Package Contents

This release package contains everything needed for M5Burner installation:

| File | Size | Purpose |
|------|------|---------|
| **ColumnsM5-v1.0-MERGED.bin** | 495KB | **👈 USE THIS FILE in M5Burner** |
| ColumnsM5-v1.0-FINAL.bin | 431KB | Firmware only (for PlatformIO upload) |
| bootloader.bin | 17KB | ESP32 bootloader (included in merged) |
| partitions.bin | 3KB | Partition table (included in merged) |
| m5burner_config.json | 7.3KB | M5Burner metadata |
| README.md | - | This file |

---

## 🚀 Quick Start (M5Burner Installation)

### Requirements
- **M5StickC Plus2** device
- **Mini Joy C Hat** (REQUIRED - game won't work without it)
- **M5Burner** software ([Download here](https://m5burner.m5stack.com/))

### Installation Steps
1. Download **`ColumnsM5-v1.0-MERGED.bin`** from this release
2. Open M5Burner
3. Connect your M5StickC Plus2 via USB
4. Click "Custom" or "Burn" 
5. Select the **MERGED.bin** file
6. Flash address: **0x0** (important!)
7. Click "Burn" and wait for completion
8. Disconnect and power on - game starts immediately!

---

## 🎮 Game Features

### Core Gameplay
- **Classic Columns** - Match 3+ gems in any direction
- **5 Colorblind-Friendly Colors** with unique markers:
  - 🔴 Red - Solid
  - 🟡 Yellow - Solid
  - 🔵 Blue - Solid
  - ⚪ White - Black dot + X
  - 🟢 Green - Black X
- **Ghost Piece** - See exactly where pieces will land
- **400ms Lock Delay** - Time to adjust placement (like Tetris)
- **All-Direction Matching** - Horizontal, vertical, both diagonals
- **Chain Reactions** - Gravity creates combos automatically
- **Progressive Difficulty** - 3% faster each level

### Advanced Features
- Lock delay resets when moving left/right (smooth gameplay)
- Ghost pieces have matching markers (white dot, white X)
- Score multiplier for chain reactions (100 × chain number)
- Level up every 1000 points
- Maximum 20 levels, speeds from 500ms to 261ms

---

## 🕹️ Controls

| Input | Action |
|-------|--------|
| **Joystick Left/Right** | Move piece horizontally |
| **Joystick Button** | Cycle gem colors in column |
| **Joystick Up** | Cycle gems (alternative) |
| **Joystick Down** | Soft drop (faster falling) |
| **BtnA (side button)** | Cycle gems (alternative) |
| **BtnB (side button)** | Hard drop (instant placement) |
| **Power Button** | Restart after game over |

### Pro Tips
- Use lock delay to slide pieces last-second
- Watch ghost piece for perfect placement
- Moving left/right extends lock delay timer
- Set up diagonal matches for style points
- Create cascades for massive chain combos

---

## 📊 Technical Specifications

| Spec | Value |
|------|-------|
| **Firmware Size** | 441KB (33.2% of flash) |
| **RAM Usage** | ~8.5% |
| **Grid Size** | 10 columns × 20 rows |
| **Colors** | 5 distinct gems |
| **Match Directions** | 4 (H, V, /, \) |
| **Lock Delay** | 400ms (resets on move) |
| **Starting Speed** | 500ms per drop |
| **Max Speed** | 261ms at level 20 |
| **Level Progression** | Every 1000 points |
| **Speed Increase** | 3% per level |

---

## 🎯 Scoring System

### Basic Scoring
- **First match**: 100 points
- **Second match (chain)**: 200 points
- **Third match (chain)**: 300 points
- **Formula**: `100 × chain_number`

### Level Progression
- **Level up**: Every 1000 points
- **Speed formula**: `500ms × (0.97 ^ (level-1))`
- **Max level**: 20

---

## 🔧 Developer Info

### Flash Addresses (in MERGED.bin)
```
0x0000   - Start of file (padding)
0x1000   - Bootloader (17KB)
0x8000   - Partition table (3KB)  
0x10000  - Firmware application (441KB)
```

### Alternative Installation (PlatformIO)
If you want to build from source:
```bash
cd ColumnsM5
pio run --target upload
```

Or flash individual files:
```bash
esptool.py --chip esp32 --port /dev/ttyACM0 \
  --baud 460800 --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 40m --flash_size detect \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 ColumnsM5-v1.0-FINAL.bin
```

---

## 📝 Version History

### v1.0.0 (2024-11-07)
- ✅ Initial release
- ✅ Full Columns gameplay with match-3 in all directions
- ✅ 5 colorblind-friendly colors with visual markers
- ✅ Ghost piece with matching markers
- ✅ 400ms lock delay (Tetris-style)
- ✅ Chain reaction system with score multipliers
- ✅ Progressive 3% difficulty curve
- ✅ Smooth controls with lock delay reset
- ✅ Portrait orientation (135×240)
- ✅ Mini Joy C hat support

---

## 🙏 Credits

- **Developer**: coreymillia + GitHub Copilot CLI
- **Inspired by**: Sega Columns (1990)
- **Architecture**: Based on Tetris-2P-Battle-M5StickC-Plus2
- **Hardware**: M5Stack M5StickC Plus2 + Mini Joy C Hat

---

## 📄 License

MIT License - See main project LICENSE file

---

## 🐛 Troubleshooting

**Game won't start after flashing:**
- Ensure Mini Joy C hat is properly connected
- Check I2C connections: SDA=0, SCL=26
- Try power cycling the device

**Joystick not responding:**
- Verify Joy C hat is seated correctly
- Check cable connections
- Hat should light up when powered

**Colors hard to distinguish:**
- White has dot + X marker
- Green has X marker
- Ghost pieces have white markers
- All colors are high contrast

**Game too fast/slow:**
- Starting speed: 500ms (level 1)
- Fastest: 261ms (level 20)
- Speed increases 3% per level
- Level up every 1000 points

---

**Made with ❤️ for the M5Stack community**

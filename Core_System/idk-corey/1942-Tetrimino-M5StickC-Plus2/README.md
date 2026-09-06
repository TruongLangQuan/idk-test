# 🎮 1942 Tetrimino Edition

[![Platform](https://img.shields.io/badge/platform-M5StickC%20Plus2-blue)](https://shop.m5stack.com/products/m5stickc-plus2-esp32-pico-mini-iot-development-kit)
[![Version](https://img.shields.io/badge/version-2.3-green)](https://github.com/yourusername/1942-tetrimino/releases)
[![License](https://img.shields.io/badge/license-MIT-orange)](LICENSE)

A modern tribute to Capcom's classic 1942 arcade shooter, reimagined with Tetris-inspired visuals for the M5StickC Plus2. Pilot your T-piece fighter through endless waves of Tetrimino enemies, collect power-ups, battle massive bosses, and see how long you can survive!

![Game Screenshot](docs/screenshot-gameplay.png)
*Intense arcade action with Tetrimino-style enemies*

---

## ✨ Features

### 🎬 Animated Title Screen
- Flying Tetrimino formations with scrolling starfield
- Player plane that oscillates and shoots randomly
- Polished arcade-style presentation

### 🎯 Auto-Fire System
- **Toggle with Button A** - No more button mashing!
- Accessibility feature for extended play sessions
- Visual "AUTO" indicator on HUD

### 👾 20 Different Enemy Types
- **6 Tetriminos**: I, O, L, J, S, Z pieces
- **6 TETRIS Letters**: T-E-T-R-I-S spelling out in sequence
- **8 Strange Symbols**: Plus, Diamond, Arrow, Heart, and more
- Large variants for extra challenge

### 🔥 Power-Up System
- Collect "P" pickups spawning every 20-45 seconds
- Stack up to **3 levels** (6 wing guns total!)
- Power-ups act as **shields** before losing lives
- Wing guns fire in perfect sync

### 🦾 Progressive Boss Scaling
- Boss battle every 60 seconds
- **Size scales** from 2x to 8x (screen-filling!)
- **Health scales** infinitely (+5 HP per wave)
- Fire rate maxes at Wave 4 (bullet hell mode!)

### 📊 End-Game Statistics
Detailed performance tracking:
- Total score
- Wave reached
- Shots fired
- Enemies killed
- Accuracy percentage

### 📈 Smooth Difficulty Progression
All metrics scale smoothly with safety caps:
- Enemy speed: 1.0x → 3.0x (maxed at Wave 14)
- Spawn rate: 1200ms → 600ms (maxed at Wave 7)
- Enemy fire: 1000ms → 400ms (maxed at Wave 13)
- Boss fire: 500ms → 200ms (maxed at Wave 4)
- Boss size: 2.0x → 8.0x (maxed at Wave 13)
- Boss health: 20 HP → ∞ (never caps!)

---

## 🎮 Gameplay

### Wave Structure
- **Waves 1-5**: Regular Tetriminos with progressive difficulty
- **Wave 6**: TETRIS letters + strange symbols appear
- **Wave 7+**: Mix of everything - Tetriminos, letters, and symbols

### Controls
- **Joystick**: 8-direction movement
- **Joystick Button**: Fire bullets
- **Button A**: Toggle Auto-Fire
- **Button B**: Toggle God Mode (for practice)

### Scoring
- Regular enemy: 10 points
- Large enemy: 15 points
- Boss hit: 5 points
- Boss defeated: 100 points bonus

---

## 🚀 Quick Start

### Hardware Requirements
- **M5StickC Plus2** (ESP32-PICO)
- **Mini JoyC Hat** (I2C controller)
- USB-C cable for programming

### Installation

#### Option 1: Flash Pre-built Binary (Easiest)
```bash
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x0 \
  releases/1942_Tetrimino-v2.3-MERGED.bin
```

#### Option 2: Build from Source
```bash
# Install PlatformIO
pip install platformio

# Clone repository
git clone https://github.com/yourusername/1942-tetrimino.git
cd 1942-tetrimino

# Build and upload
pio run --target upload
```

---

## 📁 Repository Structure

```
1942-tetrimino/
├── src/
│   ├── main.cpp                    # Main game code (1240 lines)
│   ├── UNIT_MiniJoyC.cpp          # JoyC Hat driver
│   └── UNIT_MiniJoyC.h            # Driver header
├── include/                        # Arduino headers
├── releases/
│   ├── 1942_Tetrimino-v2.3-MERGED.bin    # Flash this! (506 KB)
│   ├── 1942_Tetrimino-v2.3-RELEASE.json  # Metadata
│   └── development-builds/               # All checkpoint versions
├── docs/
│   ├── RELEASE_NOTES.md           # User release notes
│   ├── DIFFICULTY_PROGRESSION.md  # Scaling system details
│   └── QUICKSTART.md              # Setup guide
├── platformio.ini                 # Build configuration
├── README.md                      # This file
└── LICENSE                        # MIT License
```

---

## 📊 Technical Details

### Performance
- **Frame Rate**: 60 FPS locked
- **RAM Usage**: 8.7% (28,500 / 327,680 bytes)
- **Flash Usage**: 34.5% (452,240 / 1,310,720 bytes)

### Memory Allocation
- Max 15 enemies simultaneously
- 20 player bullets + 30 wing bullets
- 30 enemy bullets
- 50 particle effects
- Boss health bar and HUD

### Code Statistics
- **1240 lines** of C++ code
- Single-file architecture for easy understanding
- All sprites procedurally generated
- Zero external dependencies (except M5 libraries)

---

## 🎯 Difficulty Progression Details

| Wave | Enemy Speed | Spawn Rate | Enemy Fire | Boss Size | Boss Health |
|------|-------------|------------|------------|-----------|-------------|
| 1    | 1.0x        | 1200ms     | 1000ms     | 2.0x      | 20 HP       |
| 5    | 1.6x        | 800ms      | 800ms      | 4.0x      | 40 HP       |
| 10   | 2.35x       | 650ms      | 550ms      | 6.5x      | 65 HP       |
| 14+  | 3.0x ⚠️     | 600ms ⚠️   | 400ms ⚠️   | 8.0x ⚠️   | ∞           |

⚠️ = Maxed difficulty cap

**After Wave 14-15**: All metrics maxed except boss health which scales infinitely!

---

## 💡 Gameplay Tips

1. **Use Auto-Fire** - Toggle with Button A to save your thumb during long sessions
2. **Prioritize Power-Ups** - They're both firepower AND shields
3. **Dodge First** - Bullets are infinite, lives aren't
4. **Learn Boss Patterns** - They move predictably side-to-side
5. **Stay Mobile** - Don't camp corners in later waves
6. **Accuracy Matters** - Better hit rate = higher scores
7. **Practice with God Mode** - Button B enables invincibility for learning

---

## 🔧 Development

### Build Requirements
- PlatformIO Core
- ESP32 Arduino framework
- M5StickCPlus2 board support

### Compiling
```bash
# Install dependencies
pio lib install

# Build
pio run

# Build and upload
pio run --target upload

# Monitor serial output
pio device monitor
```

### Configuration
Edit `platformio.ini` to change:
- Upload port
- Baud rate
- Board settings

---

## 📝 Version History

- **v2.3 FINAL** (Nov 2025) - Smooth difficulty progression, release ready
- **v2.2** - Progressive boss scaling up to 8x
- **v2.1** - Animated title screen with shooting
- **v2.0** - Endless mode with letters/symbols
- **v1.6** - TETRIS letters + strange symbols
- **v1.5** - Five waves working
- **v1.4** - Wave progression system
- **v1.3** - Fixed wing gun positioning
- **v1.2** - Power-up system
- **v1.1** - Fixed controls + score display
- **v1.0** - First working version

See [CHANGELOG.md](CHANGELOG.md) for detailed changes.

---

## 🙏 Credits

### Inspired By
- **Capcom's 1942** (1984) - Classic vertical scrolling shooter gameplay
- **Tetris** (1984) - Block-based visual style and enemy shapes

### Built With
- [PlatformIO](https://platformio.org/) - Development platform
- [Arduino Framework](https://www.arduino.cc/) - Core framework
- [M5GFX](https://github.com/m5stack/M5GFX) - Graphics library
- Custom UNIT_MiniJoyC driver for joystick input

### Development
- **Platform**: M5StickC Plus2 (ESP32-PICO)
- **Development Time**: ~6 hours (single night)
- **Language**: C++ (Arduino)
- **Tools**: PlatformIO, VSCode

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

Contributions are welcome! Here are some ideas:

### Enhancement Ideas
- [ ] Sound effects and music
- [ ] High score persistence (EEPROM)
- [ ] Multiple difficulty modes
- [ ] Different player ships
- [ ] Special weapons (bombs, lasers, spread shot)
- [ ] Achievement system
- [ ] Multiplayer co-op
- [ ] More boss patterns
- [ ] Environmental hazards

### How to Contribute
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📞 Support

### Issues?
- Check [Issues](https://github.com/yourusername/1942-tetrimino/issues) for known problems
- Create a new issue with detailed description and steps to reproduce

### Questions?
- See [docs/QUICKSTART.md](docs/QUICKSTART.md) for setup help
- Check [docs/DIFFICULTY_PROGRESSION.md](docs/DIFFICULTY_PROGRESSION.md) for game mechanics
- Review [docs/RELEASE_NOTES.md](docs/RELEASE_NOTES.md) for features

---

## 🎮 Ready to Play!

Flash the binary and prepare for action-packed arcade gameplay!

**Good luck, pilot! The Tetrimino air force needs you!** ✈️🎯

---

## ⭐ Star This Project!

If you enjoy this game, please consider giving it a star! It helps others discover it.

---

*1942 Tetrimino Edition v2.3 FINAL - Built with ❤️ for M5StickC Plus2*

# 🚀 TetroidsM5 - Tetris meets Asteroids

**A space shooter for M5StickC Plus2 where you pilot a T-piece ship through waves of Tetris asteroid attacks!**

<p align="center">
  <img src="https://img.shields.io/badge/Platform-M5StickC%20Plus2-orange" />
  <img src="https://img.shields.io/badge/Framework-Arduino-blue" />
  <img src="https://img.shields.io/badge/License-MIT-green" />
</p>

## 🎮 Game Description

Control a **T-piece spaceship** that rotates 360° and fires bullets to destroy incoming **Tetris-shaped asteroids**. Survive waves of falling tetromino enemies, break them apart, and rack up your high score! Classic arcade action meets block-puzzle aesthetics in this retro-inspired shooter.

### 🌟 Key Features
- **360° Ship Rotation** - 8-direction smooth rotation with momentum physics
- **Asteroid Destruction** - Shoot Tetris pieces that break into smaller blocks
- **Progressive Waves** - Increasing difficulty with more asteroids
- **Tetris Aesthetics** - Classic piece colors (I=Cyan, O=Yellow, L=Orange, J=Blue, S=Green, Z=Red)
- **Screen Wrapping** - True Asteroids-style edge teleportation
- **God Mode** - Invincibility toggle for testing/practice
- **Smooth 60 FPS** - Buttery gameplay on ESP32

## 🕹️ Controls

| Input | Action |
|-------|--------|
| **Joystick Left/Right** | Rotate ship counter-clockwise/clockwise |
| **Joystick Up** | Thrust forward (momentum physics) |
| **Joy Button** | Fire bullet (max 4 on screen) |
| **Button A** | Fire bullet (alternative) |
| **Button B** | Toggle God Mode (invincibility) |

## 📦 Hardware Required

- **M5StickC Plus2** - ESP32-S3 development board
- **Mini JoyC Hat** - I2C joystick controller (address 0x54)

## 🚀 Quick Start

### Method 1: Flash Pre-built Binary (Easiest)

1. Download `TetroidsM5-v1.0-MERGED.bin` from this repo
2. Use [M5Burner](https://docs.m5stack.com/en/download) or esptool:
   ```bash
   esptool.py --chip esp32s3 --port /dev/ttyACM0 write_flash 0x0 TetroidsM5-v1.0-MERGED.bin
   ```
3. Attach Mini JoyC Hat to M5StickC Plus2
4. Power on and play!

### Method 2: Build from Source

#### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- USB cable for M5StickC Plus2

#### Build Steps
```bash
# Clone repository
git clone https://github.com/yourusername/TetroidsM5.git
cd TetroidsM5

# Build and upload
pio run --target upload

# Or just build
pio run
```

## 🎯 How to Play

1. **Survive the Waves** - Each wave spawns 3-8 Tetris-shaped asteroids
2. **Shoot to Break** - Hit large asteroids to break them into 4 smaller blocks
3. **Destroy All** - Clear all asteroids to advance to next wave
4. **Avoid Collisions** - Touching asteroids damages you (unless God Mode is on)
5. **Score Points**:
   - Small block destroyed: **10 pts**
   - Large asteroid broken: **50 pts**
   - Wave completed: **100 × wave number**
   - Survival time: **1 pt/second**

## 🛠️ Technical Details

### Display Configuration
- **Resolution**: 135×240 pixels (portrait mode)
- **Play Area**: 12-220px vertical
- **Block Size**: 8×8 pixels
- **Frame Rate**: ~60 FPS

### Game Physics
- **Thrust Power**: 0.4 pixels/frame²
- **Max Speed**: 4.0 pixels/frame
- **Friction**: 0.98 (slight slowdown)
- **Bullet Speed**: 6.0 pixels/frame
- **Asteroid Speed**: 1.2 pixels/frame

### Asteroid Types
| Type | Shape | Color | Blocks |
|------|-------|-------|--------|
| **I** | Line | Cyan | 4 |
| **O** | Square | Yellow | 4 |
| **L** | L-shape | Orange | 4 |
| **J** | J-shape | Blue | 4 |
| **S** | S-shape | Green | 4 |
| **Z** | Z-shape | Red | 4 |

**Note**: T-piece is reserved for the player ship!

## 📁 Project Structure

```
TetroidsM5/
├── src/
│   ├── main.cpp              # Main game logic
│   ├── UNIT_MiniJoyC.cpp/h   # JoyC controller driver
│   └── [sprite headers]      # Visual assets
├── include/
├── platformio.ini            # PlatformIO config
├── TetroidsM5-v1.0-MERGED.bin  # Pre-built binary
├── README.md                 # This file
├── TETROIDS_DESIGN.md        # Design document
└── LICENSE                   # MIT License
```

## 🎨 Design Philosophy

This game combines the best of two arcade classics:
- **Asteroids (1979)**: Rotation, thrust, screen wrap, shooting
- **Tetris (1984)**: Block aesthetics, piece types, colors

See [TETROIDS_DESIGN.md](TETROIDS_DESIGN.md) for full design document.

## 🐛 Known Issues / Future Enhancements

- [ ] Sound effects (buzzer feedback)
- [ ] High score EEPROM saving
- [ ] Particle effects on destruction
- [ ] Screen shake on death
- [ ] Scrolling starfield background
- [ ] Power-ups (shield, rapid fire, etc)

## 📜 License

MIT License - See [LICENSE](LICENSE) file for details.

## 🙏 Credits

- **Game Design & Code**: Built with PlatformIO and M5StickCPlus2 library
- **Inspired by**: Atari's Asteroids & Alexey Pajitnov's Tetris
- **Hardware**: M5Stack M5StickC Plus2 ecosystem

## 🤝 Contributing

Pull requests welcome! Please open an issue first to discuss major changes.

## 📞 Support

- **Issues**: Use GitHub Issues for bugs/features
- **Hardware**: [M5Stack Official Docs](https://docs.m5stack.com/)
- **PlatformIO**: [PlatformIO Docs](https://docs.platformio.org/)

---

**Ready to blast some blocks?** 🚀💥🎮

*Made with ❤️ for retro arcade fans and M5Stack enthusiasts*

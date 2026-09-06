# Changelog

All notable changes to M5Stick Dodge will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-11-03

### Added
- 🎮 Initial release with 4-wave difficulty system
- 🔤 Sequential TETRIS letters (T→E→T→R→I→S) with backwards R
- 🎨 7 Tetris piece types with rotation (I, O, T, S, Z, L, J)
- 💎 8 creative block patterns (plus, diamond, arrow, heart, zig-zag, cross, stairs, dot)
- ♾️ Endless Wave 4+ with random mix of all obstacle types
- 🎯 Pixel-perfect collision detection system
- 🔄 Player rotation with joystick button
- 📊 Score and level progression system (+10 per obstacle, level up every 100)
- ⚡ 60 FPS gameplay with <16ms response time
- 🎨 Colorful menu with scattered Tetris pieces and TETRIS letters
- 📱 Portrait mode display (135×240)
- 🕹️ Full Mini Joy C Hat support with analog joystick control
- 🎮 Dual game support (Dodge + Tetris)

### Wave System
- **Wave 1 (0-30s):** Classic Tetris pieces falling
- **Wave 2 (30-60s):** Sequential TETRIS letters spelling
- **Wave 3 (60-90s):** Creative block pattern shapes
- **Wave 4+ (90s+):** Random mix with progressive difficulty

### Technical
- ESP32-PICO-V3-02 chip support
- M5StickCPlus2 library integration
- I2C joystick control (SDA=0, SCL=26)
- Memory optimization (8.3% RAM, 33.8% Flash)
- Smooth interpolated movement
- Block-to-block collision algorithm

### Documentation
- Complete README with installation guide
- Contributing guidelines
- MIT License
- GitHub Actions CI workflow
- M5Burner configuration
- Installation guide with multiple methods

---

## [Unreleased]

### Ideas for Future Versions
- 🎵 Sound effects and music
- 💾 High score persistence (EEPROM)
- 🎨 Custom color themes
- 🏆 Achievement system
- 👥 Two-player mode
- 🌟 Power-ups and bonuses
- 📈 Statistics tracking
- 🎯 Challenge modes

---

[1.0.0]: https://github.com/yourusername/M5StickDodge4Wave/releases/tag/v1.0.0

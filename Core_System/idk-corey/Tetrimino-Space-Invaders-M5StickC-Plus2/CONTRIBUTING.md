# Contributing to Tetrimino Space Invaders

Thank you for your interest in contributing! 🎮

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue with:
- **Description** of the bug
- **Steps to reproduce** the issue
- **Expected behavior**
- **Actual behavior**
- **Hardware** (M5StickC Plus2, Mini JoyC Hat)
- **Version** of the game
- **Build environment** (PlatformIO version, OS)

### Suggesting Features

Feature requests are welcome! Please include:
- **Clear description** of the feature
- **Use case** - why is this feature needed?
- **Proposed implementation** (if you have ideas)
- **Alternatives considered**

### Pull Requests

1. **Fork** the repository
2. **Create a branch** for your feature
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. **Make your changes**
4. **Test thoroughly** on actual hardware
5. **Commit** with clear messages
   ```bash
   git commit -m "Add amazing feature"
   ```
6. **Push** to your fork
   ```bash
   git push origin feature/amazing-feature
   ```
7. **Open a Pull Request**

### Code Style

- Use descriptive variable names
- Comment complex logic
- Keep functions focused and modular
- Follow existing code formatting
- Test on hardware before submitting

### Testing Checklist

Before submitting, verify:
- [ ] Compiles without errors or warnings
- [ ] Works on M5StickC Plus2 with Mini JoyC Hat
- [ ] No regression in existing features
- [ ] God Mode works for testing
- [ ] Frame rate remains smooth (~50 FPS)
- [ ] Memory usage is acceptable
- [ ] Controls are responsive

### Areas for Contribution

**Gameplay:**
- New enemy patterns
- Power-up systems
- Bonus stages
- Difficulty modes
- Sound effects

**Technical:**
- Performance optimizations
- Better collision detection
- Sprite system
- EEPROM high score storage
- WiFi leaderboard

**Documentation:**
- Tutorial videos
- Gameplay guides
- Code documentation
- Translation to other languages

## Development Setup

### Prerequisites
- [PlatformIO](https://platformio.org/)
- M5StickC Plus2
- Mini JoyC Hat
- USB-C cable

### Build & Test
```bash
# Clone repo
git clone <your-fork-url>
cd Tetriminos-GitHub

# Build
pio run -e m5stick-c-plus2

# Upload to device
pio run -e m5stick-c-plus2 -t upload

# Monitor serial output
pio device monitor
```

### Project Structure
```
src/main.cpp         - Main game code
src/UNIT_MiniJoyC.*  - JoyC Hat driver
platformio.ini       - Build config
releases/            - Pre-built binaries
```

## Code Review Process

1. Maintainer reviews PR for:
   - Code quality
   - Functionality
   - Testing
   - Documentation

2. Feedback provided if changes needed

3. Once approved, PR is merged

4. Your contribution is credited in CHANGELOG.md

## Questions?

Feel free to open an issue for any questions about contributing!

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

**Thank you for making Tetrimino Space Invaders better!** 🚀

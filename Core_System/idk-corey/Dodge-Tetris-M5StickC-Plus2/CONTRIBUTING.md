# Contributing to M5Stick Dodge

First off, thank you for considering contributing to M5Stick Dodge! 🎮

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues. When creating a bug report, include:

- **Clear title and description**
- **Steps to reproduce** the behavior
- **Expected behavior**
- **Actual behavior**
- **Hardware details** (M5StickC Plus2 version, Mini Joy C Hat)
- **Firmware version**
- **Screenshots** if applicable

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, include:

- **Clear title and description**
- **Use case** - why would this be useful?
- **Possible implementation** - if you have ideas
- **Mockups or examples** - if applicable

### Pull Requests

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Make your changes
4. Test on actual hardware
5. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
6. Push to the branch (`git push origin feature/AmazingFeature`)
7. Open a Pull Request

#### PR Guidelines

- Follow existing code style
- Comment complex logic
- Update documentation if needed
- Test on M5StickC Plus2 hardware
- Keep changes focused - one feature per PR
- Update version in platformio.ini if appropriate

### Code Style

- Use meaningful variable names
- Keep functions small and focused
- Comment non-obvious code
- Follow existing formatting (2-space indents, etc.)

### Testing Checklist

Before submitting PR, verify:

- [ ] Code compiles without errors
- [ ] Tested on actual M5StickC Plus2 hardware
- [ ] All 4 waves work correctly
- [ ] Controls are responsive
- [ ] No memory leaks or crashes
- [ ] Frame rate stays at ~60 FPS
- [ ] Documentation updated if needed

## Development Setup

### Prerequisites
```bash
# Install PlatformIO
pip install platformio

# Clone your fork
git clone https://github.com/yourusername/M5StickDodge4Wave.git
cd M5StickDodge4Wave
```

### Building
```bash
# Build
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor
```

### Project Structure
```
src/
├── main.cpp           # Entry & splash
├── dodge.cpp/h        # Main game
├── tetris.cpp/h       # Tetris mode
├── gamemanager.cpp/h  # State management
├── display.cpp/h      # Display utils
├── input.cpp/h        # Input handling
└── config.h           # Configuration
```

## Wave System Architecture

When adding new obstacle types:

1. **Define pattern** in dodge.cpp (bitmap array)
2. **Add to wave logic** in `spawnPiece()`
3. **Implement drawing** function
4. **Add collision detection**
5. **Update documentation**

Example:
```cpp
// 1. Define pattern
static const uint8_t PATTERN_STAR[5] = {
  0b00100,
  0b11111,
  0b01110,
  0b01010,
  0b10001
};

// 2. Add to wave (in spawnPiece)
pieces[i].type = 10;  // New type
pieces[i].rotation = 0;

// 3. Draw function
void drawStar(int x, int y, uint16_t color) {
  // Implementation
}

// 4. Collision detection
if (p->type == 10) {
  // Check collision
}
```

## Questions?

Feel free to open an issue with the "question" label!

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow
- Have fun! 🎮

---

Thank you for contributing! 🙏

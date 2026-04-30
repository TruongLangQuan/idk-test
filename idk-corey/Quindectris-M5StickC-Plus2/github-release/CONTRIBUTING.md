# Contributing to ThreeTris

Thank you for your interest in contributing to ThreeTris! 🎮

## How to Contribute

### Reporting Bugs 🐛
- Check if the bug has already been reported in Issues
- Include detailed steps to reproduce
- Mention your hardware setup (M5StickC Plus2, JoyC Hat version)
- Include any error messages or screenshots

### Suggesting Features 💡
- Open an issue with the "enhancement" label
- Describe the feature and its benefits
- Explain how it would work

### Submitting Pull Requests 🔧

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test thoroughly on actual hardware
5. Commit with clear messages (`git commit -m 'Add amazing feature'`)
6. Push to your branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Code Style
- Follow existing code formatting
- Comment complex logic
- Keep functions focused and readable
- Test on M5StickC Plus2 hardware before submitting

### Testing
- Test all game mechanics (rotation, hold, next piece, etc.)
- Verify display rendering (no artifacts)
- Check controls (JoyC Hat + M5 buttons)
- Play through multiple levels
- Test game over and restart functionality

## Development Setup

```bash
# Clone your fork
git clone https://github.com/yourusername/ThreeTris.git
cd ThreeTris

# Install PlatformIO dependencies
pio pkg install

# Build
pio run

# Upload to device
pio run -t upload
```

## Questions?

Feel free to open an issue for any questions or discussion!

Thank you for contributing! 🙏

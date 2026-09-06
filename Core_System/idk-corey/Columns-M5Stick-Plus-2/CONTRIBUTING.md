# Contributing to Columns M5

Thank you for your interest in contributing! 🎮

## How to Contribute

### Reporting Bugs
- Use GitHub Issues
- Include steps to reproduce
- Mention M5StickC Plus2 firmware version
- Attach serial monitor output if available

### Suggesting Features
- Open an issue with "Feature Request" label
- Describe the feature clearly
- Explain why it would be useful

### Submitting Code
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test thoroughly on actual hardware
5. Commit with clear messages (`git commit -m 'Add amazing feature'`)
6. Push to your branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Code Guidelines

### Style
- Follow existing code style
- Comment complex logic
- Keep functions focused and small
- Use meaningful variable names

### Testing
- Test on real M5StickC Plus2 hardware
- Verify all game mechanics work
- Check memory usage doesn't exceed limits
- Test with Mini Joy C hat

### Documentation
- Update README.md if adding features
- Add inline comments for complex code
- Update BUILD_INSTRUCTIONS.md if build process changes

## Development Setup

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/ColumnsM5.git
cd ColumnsM5

# Build and test
pio run --target upload
pio device monitor
```

## Areas for Contribution

### High Priority
- [ ] 2-player WiFi battle mode
- [ ] Next piece preview
- [ ] High score saving (EEPROM)

### Medium Priority
- [ ] Sound effects
- [ ] Animation effects
- [ ] Different game modes
- [ ] Customizable controls

### Low Priority
- [ ] Achievements system
- [ ] Statistics tracking
- [ ] Custom color themes
- [ ] Difficulty presets

## Questions?

Feel free to open an issue for questions or discussion!

---

**Thank you for contributing! 🙌**

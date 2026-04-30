# 🤝 Contributing to TetroidsM5

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## 🐛 Bug Reports

Found a bug? Please open an issue with:
- **Clear title** describing the problem
- **Steps to reproduce** the issue
- **Expected behavior** vs actual behavior
- **Hardware**: M5StickC Plus2 model, Mini JoyC version
- **Environment**: OS, PlatformIO version, Arduino framework version
- **Serial output** if applicable

## 💡 Feature Requests

Have an idea? Open an issue with:
- **Use case**: What problem does it solve?
- **Proposed solution**: How would it work?
- **Alternatives**: Other approaches considered

## 🔧 Pull Requests

### Before You Start
1. Check existing issues/PRs to avoid duplicates
2. For major changes, open an issue first to discuss
3. Fork the repository and create a feature branch

### Development Setup
```bash
git clone https://github.com/yourusername/TetroidsM5.git
cd TetroidsM5
git checkout -b feature/your-feature-name
```

### Code Guidelines
- **Style**: Match existing code style (2-space indent, C++ conventions)
- **Comments**: Document complex logic, not obvious code
- **Performance**: Keep 60 FPS target in mind (ESP32-S3 @ 240MHz)
- **Memory**: Avoid heap allocations in game loop
- **Testing**: Test on real hardware before submitting

### Commit Messages
- Use present tense: "Add feature" not "Added feature"
- First line: brief summary (50 chars max)
- Body: explain what and why, not how

Example:
```
Add particle effects to asteroid destruction

Implements colorful particle burst when asteroids break.
Uses fixed pool of 20 particles to avoid heap allocation.
Adds ~2KB to binary size, maintains 60 FPS.
```

### Submitting PR
1. Push to your fork
2. Open PR against `main` branch
3. Fill out PR template
4. Link related issues
5. Wait for review

## 🎨 Asset Contributions

### Sprites/Graphics
- Format: C header files with uint16_t arrays (RGB565)
- Max size: Keep under 50KB per asset
- Tool: Use [LCD Image Converter](https://github.com/riuson/lcd-image-converter)
- License: Must be CC0/Public Domain or your own work

### Sound Effects (Future)
- Format: Buzzer tone sequences
- Duration: <500ms per effect
- Volume: Respectful of device speaker limits

## 🧪 Testing

### Manual Testing Checklist
- [ ] Compiles without warnings
- [ ] Game starts and displays properly
- [ ] All controls respond correctly
- [ ] Collision detection works
- [ ] No crashes during extended play (15+ min)
- [ ] God mode toggle functions
- [ ] Score increments correctly
- [ ] Memory doesn't leak (monitor heap usage)

### Hardware Testing
Test on actual M5StickC Plus2 with Mini JoyC Hat. Emulation is not sufficient.

## 📝 Documentation

- Update README.md if adding features
- Add docstrings to new functions
- Update TETROIDS_DESIGN.md for design changes

## 🏷️ Versioning

We use semantic versioning (MAJOR.MINOR.PATCH):
- **MAJOR**: Breaking changes, major rewrites
- **MINOR**: New features, backward compatible
- **PATCH**: Bug fixes, optimizations

## 📜 License

By contributing, you agree that your contributions will be licensed under the MIT License.

## 🙏 Recognition

Contributors will be acknowledged in:
- GitHub contributors page
- Future CREDITS.md file
- Release notes for significant contributions

## ❓ Questions?

Open a discussion or reach out via issues. We're happy to help!

---

**Thank you for making TetroidsM5 better!** 🚀

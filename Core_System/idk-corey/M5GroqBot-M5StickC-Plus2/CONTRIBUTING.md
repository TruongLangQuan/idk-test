# Contributing to M5GroqBot

Thank you for considering contributing to M5GroqBot! 🎉

## How to Contribute

### Reporting Bugs

1. Check if the issue already exists in [GitHub Issues](https://github.com/Coreymillia/M5GroqBot/issues)
2. If not, create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Firmware version
   - Serial monitor output (if applicable)
   - Photos/videos (if relevant)

### Suggesting Features

1. Open a GitHub Issue with the "enhancement" label
2. Describe the feature and use case
3. Explain why it would be useful
4. Consider implementation complexity

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test on actual hardware
5. Commit with clear messages (`git commit -m 'Add amazing feature'`)
6. Push to your branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Development Guidelines

### Code Style

- Use descriptive variable names
- Keep functions focused and small
- Comment complex logic
- Follow existing indentation (2 spaces)
- Avoid unnecessary String concatenation in loops

### Testing

- Test on M5StickC Plus2 hardware
- Verify all button functions work
- Check memory usage (heap/PSRAM)
- Test WiFi setup flow
- Verify Groq API integration
- Test edge cases (no WiFi, invalid API key, etc.)

### Documentation

- Update README.md if adding features
- Add inline comments for complex code
- Update API.md for API changes
- Include examples where helpful

### Commit Messages

Good:
```
Add color cycling feature for text display
Fix button B scroll behavior
Update README with v1.4 changes
```

Bad:
```
updates
fix
wip
```

## Project Structure

```
src/main.cpp          - Main source code
platformio.ini        - Build configuration
releases/             - Binary releases
docs/                 - Documentation
```

## Building Locally

```bash
# Install PlatformIO
pip install platformio

# Clone repo
git clone https://github.com/Coreymillia/M5GroqBot.git
cd M5GroqBot

# Build
pio run

# Upload
pio run --target upload

# Monitor
pio device monitor
```

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn
- No harassment or discrimination

## Questions?

Feel free to open a GitHub Discussion or Issue if you have questions!

---

**Happy coding! 🚀**

# Contributing to Hector Interactive M5StickC Plus2

We love your input! We want to make contributing to Hector Interactive as easy and transparent as possible.

## 🚀 Quick Start for Contributors

1. **Fork** the repository
2. **Clone** your fork locally
3. **Create** a feature branch
4. **Make** your changes
5. **Test** thoroughly
6. **Submit** a pull request

## 🛠️ Development Setup

### Prerequisites
- **PlatformIO** (recommended) or Arduino IDE
- **M5StickC Plus2** device for testing
- **Git** for version control

### Local Development
```bash
# Clone your fork
git clone https://github.com/yourusername/hector-m5stickc-plus2.git
cd hector-m5stickc-plus2

# Create feature branch
git checkout -b feature/amazing-new-mode

# Install dependencies (PlatformIO)
pio lib install

# Build and test
pio run
pio run --target upload
```

## 🎨 Types of Contributions

### 🧮 New Mathematical Modes
Add new wave equations and interactive patterns:
```cpp
// Example: Add to main.cpp
float myCustomWave(float x, float y, float k) {
  // Your mathematical magic here
  return result;
}
```

### 🎵 Audio Enhancements
Improve audio processing or add new features:
- FFT analysis for frequency-based visualization
- Audio filters and effects
- Multiple audio input sources

### 🎮 Control Improvements
Enhance user interaction:
- Touch screen integration
- Additional button combinations
- Gesture recognition

### 📱 Hardware Support
Extend to other M5Stack devices:
- M5Core compatibility
- M5Atom support
- Other ESP32 devices

### 📚 Documentation
Improve documentation:
- User guides and tutorials
- Technical documentation
- Code comments and examples

### 🔧 Performance Optimization
Make it faster and more efficient:
- Assembly optimization
- Memory management
- Dual-core processing

## 📝 Coding Standards

### Code Style
```cpp
// Use descriptive function names
float calculateInterferenceWave(float x, float y, float time);

// Add comments for complex math
// Calculate wave interference using superposition principle
float wave1 = amplitude * sin(frequency1 * time + phase1);
float wave2 = amplitude * sin(frequency2 * time + phase2);
return wave1 + wave2;

// Use const for constants
const float PI = 3.14159265359f;
const int GRID_SIZE = 30;
```

### Mathematical Functions
- **Name clearly**: `spiralWave()`, `plasmaField()`, `mountainTerrain()`
- **Document math**: Explain the equations used
- **Normalize output**: Return values appropriate for surface height
- **Performance**: Consider computation cost for real-time rendering

### Hardware Integration
- **Test thoroughly**: Verify on actual hardware
- **Handle errors**: Graceful fallbacks for sensor failures
- **Optimize**: Consider power consumption and heat generation

## 🧪 Testing Guidelines

### Required Testing
- [ ] **Compile**: All modes build without errors
- [ ] **Visual**: Each mode displays correctly
- [ ] **Performance**: Maintains >20 FPS
- [ ] **Interaction**: Controls work as expected
- [ ] **Hardware**: IMU and audio respond properly

### Testing Checklist
```bash
# Build test
pio run

# Upload and manual test
pio run --target upload

# Test all modes and styles
# Button A: All 4 display styles work
# Button B: All modes cycle correctly
# Audio: SOUND mode responds to noise
# Motion: TILT mode responds to movement
```

## 📋 Pull Request Process

### Before Submitting
1. **Test thoroughly** on actual M5StickC Plus2 hardware
2. **Update documentation** if adding features
3. **Check performance** impact on frame rate
4. **Follow code style** guidelines
5. **Write clear commit messages**

### Pull Request Template
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] New mathematical mode
- [ ] Audio enhancement  
- [ ] Performance improvement
- [ ] Bug fix
- [ ] Documentation update

## Testing
- [ ] Compiled successfully
- [ ] Tested on M5StickC Plus2
- [ ] All modes work correctly
- [ ] Performance acceptable (>20 FPS)

## Screenshots/Videos
(If applicable)

## Additional Notes
Any special considerations or known issues
```

### Review Process
1. **Automated checks** run (GitHub Actions)
2. **Code review** by maintainers
3. **Testing** on hardware if possible
4. **Merge** when approved

## 🎯 Good First Issues

Looking to contribute but not sure where to start?

### Easy Contributions
- **Documentation improvements**: Fix typos, add examples
- **Code comments**: Explain complex mathematical functions
- **New wave patterns**: Simple mathematical equations
- **Color schemes**: Different color palettes for visualization

### Medium Contributions
- **Audio filters**: Noise reduction, frequency analysis
- **Performance optimization**: Faster rendering, memory efficiency
- **User interface**: Better status displays, menus
- **Hardware integration**: Additional sensor support

### Advanced Contributions
- **New mathematical engines**: Complex algorithms
- **Multi-device sync**: Networked displays
- **Advanced graphics**: GPU acceleration, anti-aliasing
- **Platform ports**: Support for other hardware

## 🔧 Mathematical Guidelines

### Adding New Wave Functions

1. **Function Signature**:
```cpp
float yourWaveName(float x, float y, float k) {
  // x, y: grid coordinates (-halfsize to +halfsize)
  // k: time variable for animation
  // return: height value for surface
}
```

2. **Mathematical Considerations**:
- **Range**: Return values should be roughly -100 to +100
- **Continuity**: Smooth transitions prevent visual artifacts
- **Performance**: Consider computational complexity
- **Beauty**: Aim for visually interesting patterns

3. **Integration Steps**:
```cpp
// 1. Add to enum
enum WaveStyle {
  // ... existing modes ...
  YOUR_NEW_MODE
};

// 2. Add function
float yourWaveFunction(float x, float y, float k) {
  // Your math here
}

// 3. Add to button handler
case YOUR_NEW_MODE:
  surfaceFunction = &yourWaveFunction;
  break;

// 4. Add to display
case YOUR_NEW_MODE: 
  M5.Display.printf("YOUR"); 
  break;
```

## 🐛 Bug Reports

### Good Bug Reports Include:
- **Hardware**: M5StickC Plus2 version/revision
- **Firmware**: Version number and build info
- **Steps to reproduce**: Exact sequence of actions
- **Expected vs actual**: What should happen vs what happens
- **Environment**: Conditions when bug occurs

### Bug Report Template
```markdown
## Bug Description
Clear description of the bug

## Hardware
- Device: M5StickC Plus2
- Firmware Version: v4.0.0

## Steps to Reproduce
1. Power on device
2. Press Button B to enter SOUND mode
3. Observe behavior

## Expected Behavior
Audio should create waves when noise is detected

## Actual Behavior  
No response to audio input

## Additional Context
- Room conditions
- Audio source tried
- Any error messages
```

## 📞 Getting Help

### Channels
- **GitHub Issues**: Bug reports and feature requests
- **Discussions**: General questions and ideas
- **M5Stack Community**: Hardware-specific help

### Response Times
- **Bug reports**: Usually within 24-48 hours
- **Feature requests**: Review within 1 week
- **Pull requests**: Review within 3-5 days

---

## 🙏 Recognition

Contributors are recognized in:
- **README**: Major contributors listed
- **CHANGELOG**: Changes attributed to contributors  
- **Releases**: Contributors mentioned in release notes

Thank you for helping make Hector Interactive better! 🎨✨
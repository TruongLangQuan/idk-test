# 🎨 Paint Program for M5StickC Plus2 with Mini Joy C Hat

[![Version](https://img.shields.io/badge/version-1.3.0-brightgreen.svg)](https://github.com/coreymillia/Paint-Program-M5StickCPlus2)
[![Platform](https://img.shields.io/badge/platform-M5StickC%20Plus2-blue.svg)](https://docs.m5stack.com/en/core/M5StickC%20PLUS2)
[![License](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

A **professional paint program** for the M5StickC Plus2 featuring full analog joystick control via Mini Joy C hat. Create digital art with smooth brush strokes, multiple colors, and intuitive controls.

![Paint Program Demo](docs/demo-flower.png)

## ✨ Features

- 🎨 **Full Analog Joystick Control** - 360° smooth cursor movement
- 🌈 **12 Vibrant Colors** - Complete color palette with LED feedback
- 🖌️ **6 Brush Sizes** - From fine detail (1px) to bold strokes (8px)
- ✏️ **Smooth Line Drawing** - Gap-free interpolated brush strokes
- 🎮 **Professional Controls** - Intuitive button mapping
- 🎯 **Precision Cursor** - Thin crosshair for accurate positioning
- 💾 **Canvas Preservation** - Artwork saved when accessing menus
- 🔄 **Hold-to-Draw System** - Toggle drawing mode for precise control
- 🎨 **Background Colors** - 6 different canvas backgrounds
- 🔋 **Battery Monitoring** - Real-time battery level display

## 🎮 Controls

| Input | Function |
|-------|----------|
| **Joystick Movement** | Full analog cursor control (360°) |
| **Joystick Press (Quick)** | Cycle through 12 colors + LED feedback |
| **Joystick Hold (1 sec)** | Toggle draw mode (DRAW/MOVE) |
| **Button A** | Change brush size (1→2→3→4→5→8) |
| **Button B** | Open color palette menu |
| **PWR Button** | Clear canvas |

## 🛠️ Hardware Requirements

- **M5StickC Plus2** (required)
- **Mini Joy C Hat** (required for joystick control)
- **I2C Connection**: SDA=GPIO0, SCL=GPIO26 (automatic)

## 🚀 Quick Start

### Option 1: M5Burner (Recommended)
1. Download `Paint-Program-M5StickCPlus2-v1.3-FINAL.bin` from releases
2. Open M5Burner, select M5StickC Plus2
3. Load the .bin file and flash (address 0x0)
4. Connect Mini Joy C hat and enjoy!

### Option 2: Build from Source
```bash
# Clone repository
git clone https://github.com/coreymillia/Paint-Program-M5StickCPlus2.git
cd Paint-Program-M5StickCPlus2

# Build and flash
pio run -t upload
```

## 📐 Technical Specifications

- **Canvas Size**: 135x240 pixels (portrait mode)
- **Color Depth**: 16-bit RGB565
- **Update Rate**: 30ms cycle for smooth performance
- **Joystick Range**: 12-bit ADC (0-4095) with 150-unit dead zone
- **Drawing Method**: Interpolated filled circles for smooth lines
- **Memory Usage**: 35% Flash (458KB), 7.7% RAM (25KB)

## 🎨 Usage Guide

### Getting Started
1. **Power On**: See demo flower artwork on welcome screen
2. **Press M5 Button**: Enter paint mode with white canvas
3. **Move Joystick**: Smooth cursor movement in all directions
4. **Quick Press Joystick**: Cycle through colors (LED shows current color)
5. **Hold Joystick**: Enable drawing mode (status shows DRAW)
6. **Create Art**: Move joystick while in draw mode for smooth lines

### Advanced Features
- **Brush Sizes**: Press Button A to cycle through 6 sizes
- **Color Menu**: Press Button B for grid-based color selection
- **Background Colors**: Available in brush/background menu
- **Canvas Clearing**: Press PWR button to clear with background color
- **Settings**: Automatically saved (brush size, color, background)

## 🔧 Development

### Building
```bash
# Install PlatformIO
pip install platformio

# Build project
pio run

# Flash to device
pio run -t upload
```

### Project Structure
```
src/
├── main.cpp              # Main paint program
├── UNIT_MiniJoyC.h       # Mini Joy C hat header
└── UNIT_MiniJoyC.cpp     # Mini Joy C hat implementation
platformio.ini            # Build configuration
releases/                 # Binary releases
docs/                     # Documentation
```

## 📦 Releases

### v1.3.0 (Final) - Perfect Control Scheme
- ✅ **Perfect Controls**: BtnA=brush size, Joy press=colors, Joy hold=draw
- ✅ **Optimized Speed**: Slower joystick for precise detailed work
- ✅ **No Repetitive Functions**: Each control has unique purpose

### v1.2.0 - Hold-to-Draw System
- ✅ **Hold-to-Draw**: Toggle system for precise control
- ✅ **Demo Artwork**: Cute flower on welcome screen
- ✅ **Status Indicators**: Visual feedback for draw mode

### v1.1.0 - Cursor Trail Fix
- ✅ **No Cursor Trails**: Perfect cursor erase system
- ✅ **Analog Control**: Full 360° joystick movement
- ✅ **Smooth Lines**: Eliminated railroad track artifacts

### v1.0.0 - Initial Release
- ✅ **Basic Paint Functionality**: Working color selection and drawing

## 🤝 Contributing

Contributions welcome! Please feel free to submit pull requests or open issues for:
- New features or improvements
- Bug fixes
- Documentation updates
- Hardware compatibility

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **M5Stack** for the amazing M5StickC Plus2 hardware
- **Mini Joy C Hat** for excellent analog joystick control
- **PlatformIO** for the development environment
- **GitHub Copilot CLI** for development assistance

## 📧 Contact

- **Author**: coreymillia
- **Repository**: [Paint-Program-M5StickCPlus2](https://github.com/coreymillia/Paint-Program-M5StickCPlus2)
- **Issues**: [GitHub Issues](https://github.com/coreymillia/Paint-Program-M5StickCPlus2/issues)

---

**Create beautiful digital art on your M5StickC Plus2!** 🎨✨🚀
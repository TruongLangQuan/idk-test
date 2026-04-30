# M5StickC Plus2 Sonic The Hedgehog

![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-M5StickC%20Plus2-orange.svg)
![License](https://img.shields.io/badge/license-Educational-green.svg)

**Play Sonic the Hedgehog Green Hill Zone Act 1 on M5StickC Plus2!**

A complete port of Sonic The Hedgehog's first level for the M5StickC Plus2 with Mini Joy C Hat controller. Features authentic Sonic physics, optimized rendering, and the classic **DOWN-TO-ROLL** mechanic!

---

## 🆕 Features

- 🦔 **Complete Green Hill Zone Act 1** - Playable from start to finish
- 🎮 **DOWN-TO-ROLL Mechanic** - Press down while moving to roll, release to stand up!
- ⚡ **Authentic Sonic Physics** - Based on the official Sonic Physics Guide
- 📱 **Optimized Display** - 211x119 resolution (88% screen coverage)
- 🏃 **Smooth Gameplay** - 27-30 FPS on ESP32
- 🕹️ **Full Analog Control** - Mini Joy C Hat joystick support
- 💨 **All Classic Moves** - Run, jump, roll, spindash
- 🎯 **Pixel-Perfect Collision** - Terrain and enemy interactions
- ⭕ **Ring Collection** - Collect rings for protection
- 🤖 **Enemy AI** - Motobug, Crabmeat, Chopper

---

## 🎮 Controls

| Input | Action |
|-------|--------|
| **Joystick Left** | Move Sonic left |
| **Joystick Right** | Move Sonic right |
| **Joystick DOWN** 🆕 | **Roll into ball (press to roll, release to stand)** |
| **Joystick Button** | Jump |
| **BtnA + Joy Button (standing)** | Spindash (charge up) |
| **BtnA + Joy Button (moving)** | Roll (alternative) |

---

## 🛠️ Hardware Requirements

- **M5StickC Plus 2** (ESP32-PICO-V3-02)
- **Mini Joy C Hat** (required for controls)
- USB-C cable for flashing

### Connections
- Mini Joy C connects to Grove port on M5StickC Plus2
- I2C: SDA=0, SCL=26 (automatic via Grove)
- JoyCHat I2C address: 0x54

---

## 📦 Quick Start - Pre-Built Binary

### Option 1: M5Burner (Easiest)

1. Download **M5Burner**: https://docs.m5stack.com/en/download
2. Download `M5StickSonic-v1.2-FINAL-MERGED.bin` from [releases/](releases/)
3. Connect M5StickC Plus2 via USB
4. Open M5Burner and select the .bin file
5. Click "Burn"
6. Connect Mini Joy C Hat
7. Play!

### Option 2: esptool (Advanced)

```bash
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x0 M5StickSonic-v1.2-FINAL-MERGED.bin
```

---

## 🔨 Building from Source

### Prerequisites

- [PlatformIO](https://platformio.org/) (recommended)
- OR [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 support

### PlatformIO (Recommended)

```bash
# Clone the repository
git clone <your-repo-url>
cd M5StickSonic

# Build and upload
pio run --target upload

# Or just build
pio run
```

### Arduino IDE

1. Install **M5StickCPlus2** library via Library Manager
2. Copy all files from `src/` and `include/` to your sketch folder
3. Select **Tools > Board > M5Stick-C**
4. Upload

---

## 📁 Project Structure

```
M5StickSonic/
├── src/
│   ├── main.cpp              # Main game loop
│   ├── Player.cpp            # Sonic physics & movement
│   ├── Level.cpp             # Level management
│   ├── Screen.cpp            # Custom renderer
│   ├── Keyboard.cpp          # Input handling
│   ├── UNIT_MiniJoyC.cpp     # JoyCHat driver
│   └── ...
├── include/
│   ├── Settings.h            # Platform configuration
│   ├── GameSettings.h        # Game configuration
│   ├── UNIT_MiniJoyC.h       # JoyCHat interface
│   ├── content/              # Sprites & tiles
│   ├── entities/             # Game objects
│   └── structs/              # Data structures
├── platformio.ini            # Build configuration
├── releases/                 # Pre-built binaries
└── README.md                 # This file
```

---

## ⚙️ Configuration

### Resolution Modes

Edit `include/GameSettings.h`:

```cpp
#define OPTIMAL_RESOLUTION    // 211x119 @ 27-30 FPS (default)
// #define FULL_RESOLUTION     // 240x135 @ 8-10 FPS
// #define MAX_FPS_RESOLUTION  // 120x68 @ 47-60 FPS
```

---

## 📝 Version History

### v1.2.0 (November 6, 2024) - FINAL
- ✅ **NEW: DOWN-TO-ROLL mechanic!**
- ✅ Press down while moving to roll
- ✅ Release down to stand up
- ✅ Just like original Sonic!

### v1.1.0
- Increased screen size to 211x119
- Fixed joystick axis mapping
- Optimized rendering

### v1.0.0
- Initial M5StickC Plus2 port

---

## 📚 Credits

### Original M5Stack Port
- **GalaxyShad** (2020)
- https://github.com/GalaxyShad/M5Stack-ESP32-ILI9341-Sonic-The-Hedgehog-Port

### M5StickC Plus2 Adaptation
- **coreymillia** with **GitHub Copilot CLI**

### Resources
- **Sonic Physics**: [Sonic Retro](http://info.sonicretro.org/Sonic_Physics_Guide)
- **Graphics**: [The Spriters Resource](https://www.spriters-resource.com/)
- **Characters**: © SEGA Corporation

---

## ⚖️ Legal

**This is an UNOFFICIAL fan-made educational project.**

All Sonic the Hedgehog characters, graphics, and assets are **© SEGA**. Not for commercial use.

---

**Gotta Go Fast!** 🦔💨

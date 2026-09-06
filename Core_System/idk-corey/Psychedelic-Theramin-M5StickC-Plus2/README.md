# TheraminM5 🎵✨

**Motion-Controlled Theremin with Psychedelic 3D Visuals for M5StickC Plus2**

Transform your M5StickC Plus2 into a playable musical instrument! Tilt to create music while watching mesmerizing frequency-reactive animations.

![Version](https://img.shields.io/badge/version-1.3.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-M5StickC%20Plus2-orange.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## 🎮 Features

- 🎵 **Real-time Theremin Synthesis** - Play music by tilting the device
- 🎶 **Smooth Audio Engine** - v1.3 continuous tone with no stuttering
- 📐 **IMU Motion Control** - 6-axis accelerometer/gyroscope tilt detection
- 🌈 **Psychedelic Hector 3D Visuals** - Frequency-reactive mesh animations
- 🎨 **Dynamic Color Mapping** - HSV colors respond to pitch
- 🔊 **Volume by Tilt** - Forward/back controls loudness
- 🎭 **3 Visual Modes** - Grid, Solid, and Zebra patterns
- 📊 **Real-time Display** - Live frequency and angle feedback
- 🔇 **Instant On/Off** - Toggle sound without stopping visuals

## 🎯 Quick Start

### Flash with M5Burner (Easiest)
1. Download `TheraminM5-v1.3-SMOOTH-AUDIO.bin`
2. Open M5Burner
3. Select your M5StickC Plus2
4. Flash at address `0x0`
5. Done! Power on and play

### Flash with esptool
```bash
esptool --chip esp32 --port /dev/ttyACM0 --baud 1500000 write_flash 0x0 TheraminM5-v1.3-SMOOTH-AUDIO.bin
```

## 🎹 How to Play

### Controls
- **Tilt Left/Right** → Change pitch (200-1200Hz)
- **Tilt Forward/Back** → Control volume (0-255)
- **Button A** → Toggle sound ON/OFF
- **Button B** → Cycle visual modes
- **Hold Power** → Show help screen

### Tilt Mapping
```
        HIGHER VOLUME (tilt toward you)
                    ↑
                    |
    LOWER     ←  FLAT  →    HIGHER
    PITCH         |         PITCH
                  ↓
        LOWER VOLUME (tilt away)
```

### Tips
- Start with device flat to find center pitch
- Tilt slowly for smooth frequency changes
- Tilt backward (toward you) for maximum volume
- Practice with sound off to learn movements
- Hold device steady for sustained notes

## 🎨 Visual Modes

NOTE: WE USED HECTOR... BUT... IT IS NOT. I cannot explain what actually happened I just rolled with it. 

| Mode | Description |
|------|-------------|
| **GRID** | Wireframe mesh - classic Hector look |
| **SOLID** | Filled triangles - psychedelic surface |
| **ZEBRA** | Alternating pattern - checkerboard mesh |

Colors shift with frequency, creating a synesthetic experience where you can "see" the pitch.

## 🛠️ Build from Source

### Requirements
- [PlatformIO](https://platformio.org/)
- M5StickC Plus2 hardware
- USB-C cable

### Build Steps
```bash
# Clone repository
git clone https://github.com/yourusername/TheraminM5.git
cd TheraminM5

# Build with PlatformIO
pio run

# Upload to device
pio run -t upload
```

### Project Structure
```
TheraminM5/
├── src/
│   └── main.cpp                    # Main theremin code
├── platformio.ini                  # PlatformIO configuration
├── TheraminM5-v1.3-SMOOTH-AUDIO.bin   # Pre-built firmware
├── TheraminM5-v1.3-SMOOTH-AUDIO.json  # M5Burner config
└── README.md                       # This file
```

## 🔧 Technical Specs

### Audio Engine
- **Frequency Range**: 200-1200Hz (chromatic scale)
- **Synthesis**: PWM tone generation with 50ms bursts
- **Update Rate**: 10ms (100Hz for smooth playback)
- **Smoothing**: Exponential moving average (EMA)
- **Speaker**: Built-in GPIO 2

### Motion Control
- **Sensor**: MPU6886 (6-axis IMU)
- **Roll Range**: ±45° → Frequency
- **Pitch Range**: ±30° → Volume
- **Sample Rate**: 50Hz (20ms intervals)
- **Angle Calc**: atan2 from accelerometer XYZ

### Display
- **Resolution**: 240x135 pixels (landscape)
- **Rendering**: 3D perspective mesh projection
- **Grid**: 50×50 unit space, 2-step increments
- **Animation**: Theremin-reactive wave function
- **Color**: 16-bit RGB565 with HSV mapping

### Performance
- **RAM Usage**: 9.7% (31KB / 327KB)
- **Flash Usage**: 15.4% (483KB / 3.1MB)
- **Frame Rate**: ~60 FPS
- **Power**: ~120mA @ 3.7V (playing audio)

## 📚 Version History

### v1.3.0 (2024-11-16) - **SMOOTH AUDIO** ✨
- Fixed audio stuttering with 50ms continuous tone bursts
- Removed frequency change threshold for seamless playback
- Smoother audio quality for sustained notes

### v1.2.0
- Improved frequency mapping and visual feedback
- Enhanced tilt angle ranges

### v1.1.0
- Enhanced IMU smoothing for better control
- Refined volume control sensitivity

### v1.0.0
- Initial release
- Full theremin controls with tilt sensing
- 3 visual modes with psychedelic animations
- Real-time synthesis and frequency display

## 🎓 Inspired By

- **Original Theremin** (Leon Theremin, 1920) - The world's first electronic instrument
- **Motion Sensing** - Modern accelerometer adaptation of proximity sensing
- **Hector Graphics** - Classic 3D mesh animation techniques
- **Synesthesia** - Visual representation of sound frequency

## 🤝 Credits

**Developer**: coreymillia with GitHub Copilot CLI

**Libraries**:
- M5StickCPlus2 SDK
- M5Unified Framework
- ESP32 Arduino Framework

## 📄 License

MIT License - Feel free to modify and share!

## 🐛 Known Issues

- Audio output limited by M5StickC Plus2 internal speaker quality
- Best results with external amplified speaker via GPIO
- Vertical orientation detection may drift during extended play

## 💡 Future Ideas

- [ ] Bluetooth MIDI output
- [ ] Recording/playback mode
- [ ] Scale/key locking (pentatonic, chromatic, etc.)
- [ ] Vibrato effect control
- [ ] Custom waveform selection
- [ ] Multi-octave switching
- [ ] WiFi streaming output

## 🌟 Show Your Support

If you enjoyed this project:
- ⭐ Star this repository
- 🍴 Fork and create your own variations
- 🐛 Report bugs or suggest features
- 📸 Share videos of your performances!

---

**Made with 🎵 for the M5Stack community**

*Turn your M5StickC Plus2 into a pocket-sized musical instrument!*

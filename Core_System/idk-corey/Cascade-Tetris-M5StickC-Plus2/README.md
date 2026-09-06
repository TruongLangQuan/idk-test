# 🕹️ CASCADE TETRIS BATTLE - M5StickC Plus2

**Revolutionary CASCADE TETRIS with breakaway physics + 2-Player WiFi Battle Mode!**

[![Platform](https://img.shields.io/badge/platform-M5StickC%20Plus2-blue.svg)](https://shop.m5stack.com/products/m5stickc-plus2-esp32-mini-iot-development-kit)
[![ESP32](https://img.shields.io/badge/ESP32-PICO--V3--02-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## 🎮 Features

### 🌟 Core Gameplay
- **CASCADE PHYSICS**: Watch pieces break apart and cascade when marked with X!
- **Toggle CASCADE Mode**: Button B toggles breakaway physics ON/OFF for regular Tetris
- **Hold System**: Hold pieces for strategic play (Button A)
- **Ghost Piece Preview**: See where your piece will land
- **Level Selection**: Choose your difficulty (1-10)
- **Proper Wall Kicks**: All pieces rotate consistently clockwise
- **Lock Delay System**: Extra time to move pieces when they hit the ground

### ⚔️ 2-Player WiFi Battle
- **Real-time WiFi Battle**: Two M5StickC Plus2 devices battle over WiFi
- **Attack System**: Send garbage lines to opponent when clearing CASCADE lines
- **Danger Level**: Visual indicator of incoming attacks
- **Ready-up System**: Both players must ready up before battle starts

### 🎯 Controls (FIXED!)
- **Joystick**: Move pieces (Left/Right/Down) + Rotate (Button)  
- **Button A**: Hold piece
- **Power Button**: Pause game
- **Button B**: Toggle CASCADE breakaway mode ON/OFF
- **L+R Buttons**: WiFi Beacon spam mode
- **Up on Joystick**: WiFi Scanner mode
- **Down on Joystick**: Battle Mode setup

## 🚀 Quick Start

### Option 1: Flash Pre-built Binary (Easiest)
1. Download `CASCADE_TETRIS_v1.0_FINAL_MERGED_*.bin`
2. Flash with esptool:
```bash
esptool --chip esp32 --port /dev/ttyACM0 write-flash 0x0 CASCADE_TETRIS_v1.0_FINAL_MERGED_*.bin
```

### Option 2: Build from Source
1. Install [PlatformIO](https://platformio.org/)
2. Clone this repository
3. Build and upload:
```bash
cd CASCADE-TETRIS-M5StickC-Plus2
platformio run --target upload
```

## 🎲 How to Play

### Single Player
1. Power on M5StickC Plus2
2. Press M5 button to start
3. Select level (L/R on joystick, M5 to confirm)
4. Play! Use Button B to toggle CASCADE mode
5. CASCADE mode: Pieces with X break apart when they land

### 2-Player Battle
1. Both players: Hold Down on joystick at startup
2. One device becomes host, other connects as client
3. Both players ready up (M5 button when prompted)
4. Battle begins! Clear CASCADE lines to attack opponent

## 🔧 Technical Details

### Hardware Required
- **M5StickC Plus2** (ESP32-PICO-V3-02)
- **MiniJoyC Unit** (I2C joystick accessory)

### Build Requirements
- PlatformIO Core
- ESP32 Arduino Framework
- Libraries (auto-installed):
  - M5StickCPlus2
  - IRremoteESP8266

### Performance
- **Flash Usage**: ~1MB (78.2% of 1.3MB available)
- **RAM Usage**: ~49KB (15.1% of 320KB available)
- **Frame Rate**: Smooth 60fps gameplay
- **Network**: WiFi 2.4GHz for battle mode

## 🏗️ Project Structure

```
├── src/
│   ├── main.cpp           # Main game logic
│   ├── UNIT_MiniJoyC.*    # Joystick interface  
│   ├── tet.h              # Background graphics
│   └── wifi_*             # WiFi utilities
├── platformio.ini         # Build configuration
├── CASCADE_TETRIS_v1.0_FINAL_MERGED_*.bin  # Ready-to-flash binary
└── README.md              # This file
```

## 🎨 Game Modes

### CASCADE Mode (ON)
- Pieces spawn with X markers randomly
- X-marked blocks break apart when piece lands
- Remaining blocks cascade down with physics
- Creates chain reactions and complex gameplay

### Regular Mode (OFF) 
- Standard Tetris gameplay
- No breakaway physics
- Traditional line clearing only

## 🛠️ Development Notes

### Rotation System
- All pieces rotate **clockwise consistently** 
- Fixed piece data ordering to ensure uniform rotation direction
- Wall kicks implemented for smooth rotation near boundaries

### Network Protocol
- Custom UDP protocol for real-time battle
- Heartbeat system maintains connection
- Attack data synchronized between devices

### Button Fixes Applied
- Swapped Button A and Power button functionality
- Button B dedicated to CASCADE mode toggle
- Clear, consistent control scheme

## 🏆 Credits

**CASCADE TETRIS Team**
- Revolutionary CASCADE physics system
- 2-Player WiFi battle implementation  
- M5StickC Plus2 optimization
- Button mapping fixes
- Clockwise rotation consistency

Original Firmware By
- AleksAlcDel/Tetris-for-M5StickCPlus2 - Original M5StickC Plus2 implementation

- Modified by Coreymillia

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

---

**🎮 Ready to CASCADE? Flash and play the most innovative Tetris experience on M5StickC Plus2!**

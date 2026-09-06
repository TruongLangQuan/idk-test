# 🎃 Halloween Tetris ULTIMATE for M5StickC Plus2

## The Ultimate Halloween Gaming Device! 
**Tetris + WiFi Tools + Universal TV Remote** - All in one spooky package!

![Halloween Tetris](https://img.shields.io/badge/Halloween-Tetris-orange?style=for-the-badge&logo=games)
![M5StickC Plus2](https://img.shields.io/badge/M5StickC-Plus2-red?style=for-the-badge)
![IR Remote](https://img.shields.io/badge/IR-Remote-blue?style=for-the-badge)

## 🎮 Features

### 🎯 Halloween Tetris Game
- **7 Spooky Colored Pieces** - Halloween-themed block colors
- **"HAPPY HALLOWEEN!!"** - Large orange text on opening screen  
- **Halloween Colored Borders** - 7 rotating spooky colors framing gameplay
- **Level Selection** - Choose difficulty 0-9
- **Ghost Piece Preview** - See where pieces will land
- **Enhanced Gameplay** - Smooth controls and scoring

### 📡 WiFi Tools
- **40 Halloween WiFi Beacons** - Spooky network names for fun
- **WiFi Scanner** - Analyze nearby networks

### 📺 Universal IR Remote (NEW!)
- **50 Universal TV Power Codes** - Works with most TV brands
- **Built-in IR Emitter** - Uses GPIO19 (no external hardware needed)
- **Progress Indicator** - Shows transmission status
- **Instant Response** - Turn TVs on/off immediately

## 🎮 Controls

| Control | Function |
|---------|----------|
| **M5 Button** | Start Game / Menu Select |
| **Left/Right Buttons** | Navigate menus / Move pieces |
| **Joystick** | Move/Rotate pieces in game |
| **Joy UP** | WiFi Scanner |
| **Joy DOWN** | Universal TV Remote |
| **L+R Buttons** | Halloween WiFi Beacon Spam |

## 🚀 Quick Start

### Option 1: Flash Pre-compiled Binary (Easiest)
1. Download `Halloween-Tetris-ULTIMATE-FINAL.bin`
2. Use [ESP32 Flash Tool](https://www.espressif.com/en/support/download/other-tools) or [M5Burner](https://m5stack.com/pages/download)
3. Flash to your M5StickC Plus2 at address 0x0

### Option 2: Compile from Source
1. Install [PlatformIO](https://platformio.org/)
2. Clone this repository
3. Open in PlatformIO
4. Build and upload to M5StickC Plus2

## 🔧 Hardware Requirements
- **M5StickC Plus2** with built-in IR emitter (GPIO19)
- **JoyC Hat** (for enhanced controls)
- No additional hardware required!

## 📋 File Structure
```
Halloween-Tetris-GitHub-Package/
├── Halloween-Tetris-ULTIMATE-FINAL.bin  # Ready-to-flash binary
├── src/                                  # Source code
├── lib/                                  # Libraries  
├── platformio.ini                        # Build configuration
└── README.md                            # This file
```

## 🎃 Version History
- **v1** - Base Halloween Tetris with colored pieces
- **v2** - Added Halloween colored borders
- **v3** - Added "HAPPY HALLOWEEN!!" text
- **v4** - Added power management fixes
- **ULTIMATE** - Added comprehensive IR remote (50 TV codes)

## 🏆 Credits & Acknowledgments

### Original Tetris Implementation
- **Base Tetris Code** - Original M5StickC Tetris implementation
- **M5Stack Community** - Hardware support and examples

### WiFi Features  
- **Bruce Project** - WiFi beacon and scanning inspiration
- **ESP32 WiFi Libraries** - Core networking functionality

### IR Remote Implementation
- **TV-B-Gone Project** - Universal IR code database by Mitch Altman & Limor Fried
- **IRremoteESP8266 Library** - ESP32 IR transmission support
- **Bruce Project** - IR implementation reference

### Enhancement Development
- **Halloween Theme** - Custom color schemes and spooky modifications
- **Ghost Piece Feature** - Advanced Tetris gameplay enhancement
- **Level Selection** - Difficulty progression system
- **Integration** - Combining all features into unified experience

## 📝 License
This project builds upon multiple open-source projects. Please respect the original licenses of all components.

## 🐛 Support
If you encounter issues, please check that you're using an M5StickC Plus2 with the JoyC hat properly connected.

---
**Happy Halloween! 🎃👻 Turn off TVs and play Tetris in spooky style!**
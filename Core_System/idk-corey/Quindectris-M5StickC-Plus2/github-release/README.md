# ThreeTris v3.0 🎮

**The Ultimate 3-Block Tetris Game for M5StickC Plus2**

A modern, feature-complete implementation of ThreeTris (Triomino Tetris) with 8 unique pieces, professional title screen, and advanced gameplay mechanics.

![Version](https://img.shields.io/badge/version-3.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-M5StickC%20Plus2-orange.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

---

## 📖 Table of Contents

- [Features](#-features)
- [Gameplay](#-gameplay)
- [Hardware Requirements](#-hardware-requirements)
- [Installation](#-installation)
- [Controls](#-controls)
- [Game Pieces](#-game-pieces)
- [Building from Source](#-building-from-source)
- [Screenshots](#-screenshots)
- [Credits](#-credits)
- [License](#-license)

---

## ✨ Features

### 🎯 Core Gameplay
- **8 Unique Triomino Pieces** - More variety than the original!
- **Perfect Rotation System** - All pieces rotate clockwise smoothly
- **Modern Tetris Mechanics** - Hold piece, next piece preview, ghost piece
- **Endless Gameplay** - Play forever with progressive difficulty
- **Score & Level Tracking** - Complete statistics display

### 🎨 Professional Presentation
- **Animated Title Screen** - Beautiful block letters with cascading pieces
- **Clean HUD** - Score, level, lines, hold/next previews
- **Polished Graphics** - No artifacts, perfect alignment
- **Portrait Mode** - Optimized for M5StickC Plus2's 135x240 display

### ⚡ Progressive Difficulty
- **Dynamic Lock Delay** - Gets harder as you progress (300ms → 600ms)
- **Speed Progression** - Levels 1-20 gradually increase speed
- **Endless Challenge** - Speed caps at level 20, game continues forever

---

## 🎮 Gameplay

ThreeTris is a unique twist on classic Tetris using only **triomino pieces** (1-3 blocks each). Stack pieces to clear lines and increase your score. The game features modern mechanics like:

- **Hold System** - Save a piece for later
- **Next Preview** - See what's coming
- **Ghost Piece** - Shows where piece will land
- **Progressive Difficulty** - Lock delay increases with level

### Scoring System
- Clear lines to earn points
- Level up every 10 lines
- Track your high score
- Endless gameplay for maximum scores

---

## 🔧 Hardware Requirements

- **M5StickC Plus2** - ESP32-based handheld device
- **Mini JoyC Hat** - Directional control + button (I2C address: 0x54)
- **Display**: 135x240 portrait orientation

### Tested Configuration
- M5StickC Plus2 (ESP32-PICO-D4)
- Mini JoyC Hat connected via Grove port
- 320KB RAM, 4MB Flash

---

## 📥 Installation

### Method 1: M5Burner (Recommended)

1. Download **[M5Burner](https://docs.m5stack.com/en/download)**
2. Download **ThreeTris-v3.0-MERGED.bin** from [Releases](releases/)
3. Open M5Burner and select your M5StickC Plus2
4. Load the MERGED.bin file
5. Click "Burn" and wait for completion
6. Enjoy!

### Method 2: esptool.py

```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash 0x0 ThreeTris-v3.0-MERGED.bin
```

### Method 3: PlatformIO Upload

```bash
pio run -t upload
```

---

## 🎮 Controls

### Title Screen
- **JoyC Button** - Start game

### In-Game
| Control | Action |
|---------|--------|
| **JoyC D-Pad Left/Right** | Move piece horizontally |
| **JoyC Button** | Rotate piece clockwise |
| **Button A (M5)** | Soft drop (faster fall) |
| **Button B (M5)** | Hold piece / Restart |

---

## 🧩 Game Pieces

ThreeTris features **8 unique triomino pieces**, all with exactly 1-3 blocks:

| # | Name | Blocks | Shape | Color |
|---|------|--------|-------|-------|
| 1 | **I-Triomino** | 3 | `###` | Cyan |
| 2 | **L-Triomino** | 3 | `#..` `#..` `##.` | Blue |
| 3 | **J-Triomino** | 3 | `..#` `..#` `.##` | Orange |
| 4 | **Single Block** | 1 | `#` | Yellow |
| 5 | **Double Block** | 2 | `##` | Green |
| 6 | **Diagonal Double** | 2 | `#.` `.#` | Red |
| 7 | **Diagonal Triple** | 3 | `#..` `.#.` `..#` | Purple |
| 8 | **T-minus-one** | 3 | `###` `.#.` | White |

### Rotation System
- All pieces rotate **clockwise** around their center block
- No wall-kicks or SRS complexity
- Simple, predictable rotation behavior

---

## 🛠️ Building from Source

### Prerequisites
- [PlatformIO](https://platformio.org/)
- VSCode with PlatformIO extension (recommended)

### Build Steps

1. **Clone the repository**
```bash
git clone https://github.com/yourusername/ThreeTris.git
cd ThreeTris
```

2. **Install dependencies**
```bash
pio pkg install
```

3. **Build**
```bash
pio run
```

4. **Upload to device**
```bash
pio run -t upload
```

### Project Structure
```
ThreeTris/
├── src/
│   ├── main.cpp          # Entry point, game states, title screen
│   ├── tetris.cpp        # Core game logic
│   ├── tetris.h
│   ├── display.cpp       # Display management
│   ├── display.h
│   ├── input.cpp         # Input handling
│   ├── input.h
│   ├── UNIT_MiniJoyC.cpp # JoyC Hat driver
│   ├── UNIT_MiniJoyC.h
│   └── config.h          # Game configuration
├── releases/
│   ├── ThreeTris-v3.0-MERGED.bin
│   └── ThreeTris-v3.0-FINAL.bin
├── platformio.ini
└── README.md
```

---

## 📸 Screenshots

### Title Screen
- Professional animated title with "THREETRIS" in block letters
- 3 cascading animated triomino pieces
- Version badge and game info
- Blinking "Press Button to Start" prompt

### Gameplay
- 10-column playfield with 23 visible rows
- Hold piece preview (left)
- Next piece preview (right)
- Score, Level, and Lines display
- Ghost piece showing landing position
- Clean borders and perfect alignment

### Game Over
- Stacked "GAME OVER" text
- Final score display
- Restart instructions

---

## 🎯 Game Configuration

### Difficulty Settings
```cpp
Lock Delay Progression:
- Levels 1-5:   300ms (Quick reactions)
- Levels 6-10:  400ms (Getting challenging)
- Levels 11-15: 500ms (Expert territory)
- Levels 16-20+: 600ms (Master level)

Drop Speed Progression:
- Level 1:  500ms per row
- Level 20: 50ms per row
- After 20: Locked at 50ms (max difficulty)
```

### Display Settings
```cpp
Screen:       135x240 portrait
Field Size:   10 columns x 23 rows
Block Size:   8x8 pixels
Colors:       Full RGB565 palette
```

---

## 🚀 Future Ideas

- Sound effects and music
- Save high scores to EEPROM
- Multiple game modes (time attack, survival)
- Multiplayer via WiFi
- Custom piece sets
- Difficulty presets

---

## 👏 Credits

**Development**: Built November 2024  
**Platform**: M5Stack M5StickC Plus2  
**Framework**: Arduino (ESP32)  
**Libraries**: M5StickCPlus2, M5Unified

### Inspiration
Based on the classic ThreeTris/Triomino Tetris concept, modernized with contemporary game mechanics and polished presentation.

---

## 📄 License

MIT License

Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs via GitHub Issues
- Submit pull requests for improvements
- Share your high scores and gameplay videos
- Suggest new features

---

## 📞 Support

Having issues? Check:
1. Hardware connections (JoyC Hat properly connected?)
2. Correct M5StickC Plus2 device selected
3. Latest firmware flashed (v3.0-MERGED.bin)
4. Battery charged

---

## 🎮 Enjoy Playing!

**ThreeTris v3.0** - The definitive triomino Tetris experience!

Built with ❤️ for the M5Stack community.

---

*Last Updated: November 2024*

# Quindectris 🎮

**The Ultimate Mixed-Block Tetris Experience!**

Welcome to Quindectris - a revolutionary Tetris variant featuring pieces from 1 to 4 blocks in size! Evolved from the successful ThreeTris project into the ultimate mixed-block challenge.

![Version](https://img.shields.io/badge/version-v2.2-blue) ![Platform](https://img.shields.io/badge/platform-M5StickC_Plus2-orange) ![License](https://img.shields.io/badge/license-MIT-green)

## 🎯 What Makes This Special

Quindectris combines **15 unique pieces** ranging from tiny 1-block monominos to classic 4-block tetrominoes, creating an unprecedented challenge that keeps you adapting your strategy constantly!

## 🔧 Hardware Requirements

- **M5StickC Plus2** 
- **Mini JoyC Hat** (I2C joystick controller)

## ✨ Features

- **15 unique pieces** (1-4 blocks each)
- **Ghost piece preview** for perfect placement
- **Hold piece functionality** for strategic play  
- **Progressive difficulty** (speeds up every 10 lines)
- **Modern Tetris mechanics** with lock delay
- **Professional UI** with stats and preview
- **Smooth gameplay** optimized for M5StickC Plus2

## 🎮 Controls

- **Joystick Left/Right**: Move piece horizontally
- **Joystick Down**: Soft drop (faster fall)
- **Joystick Up**: Hard drop (instant placement)
- **Joystick Button**: Rotate piece clockwise
- **Button A** (side): Hold/swap current piece
- **Button B**: Reset game

## 🚀 Quick Start

### Option 1: Flash Pre-built Binary (Recommended)
1. Download `Quindectris-v2.2-FINAL-PERFECT-20251201.bin` from releases
2. Use [M5Burner](https://docs.m5stack.com/en/download) or esptool to flash
3. Connect Mini JoyC Hat and play!

### Option 2: Build from Source
```bash
# Install PlatformIO
pip install platformio

# Clone and build
git clone [this-repo]
cd Quindectris
pio run -t upload
```

## 🧩 Piece Types

**Monominos (1 block)**
- Single dot - Ultimate precision piece

**Dominoes (2 blocks)**  
- I2: Straight line (2 blocks)
- Strategic gap fillers

**Triominoes (3 blocks)**
- I3, L3, V3 - Tactical placement pieces
- Perfect for complex patterns

**Tetrominoes (4 blocks)**
- I, L, J, T, O, S, Z - Classic Tetris power pieces
- The familiar favorites with new context

## 📊 Game Evolution

This project evolved through several iterations:
- **ThreeTris** → **MorphTris** → **Quindectris**
- Each version refined gameplay and added more piece variety
- v2.2 represents the perfect balance of challenge and playability

## 🔄 Version History

- **v2.2 (Dec 2024)**: Final perfect release - 15 pieces, balanced gameplay
- **v2.1**: QUIN15 release with 15 pieces
- **v2.0**: Major overhaul with mixed piece sizes
- **v1.x**: MorphTris iterations (11-13 pieces)

## 🛠️ Development

Built with:
- **PlatformIO** for cross-platform development
- **Arduino framework** for M5StickC Plus2
- **Custom graphics engine** for smooth 60fps gameplay
- **Optimized algorithms** for piece rotation and collision

## 📜 License

MIT License - Feel free to modify and share!

## 🎯 Ready to Challenge Yourself?

Download the latest release and experience Tetris like never before. Every game is different when you never know if the next piece will be a tiny dot or a massive tetromino!

---

*Quindectris - Where every block counts! 🎮✨*

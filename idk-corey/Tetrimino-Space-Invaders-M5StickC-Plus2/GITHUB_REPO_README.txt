================================================================================
TETRIMINO SPACE INVADERS - GITHUB REPOSITORY PACKAGE
================================================================================

This folder contains everything needed to:
✅ Flash the game to M5StickC Plus2
✅ Build from source with PlatformIO
✅ Contribute to the project
✅ Create a GitHub repository

================================================================================
CONTENTS
================================================================================

📁 src/                        - Source code
   ├── main.cpp                - Main game (~600 lines)
   ├── UNIT_MiniJoyC.cpp       - JoyC Hat driver
   └── UNIT_MiniJoyC.h         - JoyC Hat interface

📁 releases/                   - Pre-built binaries
   ├── Tetriminos-v1.1-MERGED.bin   - Complete binary (FLASH THIS!)
   ├── Tetriminos-v1.1-APP.bin      - App only
   ├── bootloader.bin               - ESP32 bootloader
   ├── partitions.bin               - Partition table
   ├── Tetriminos-v1.1.json         - M5Burner metadata
   ├── INSTALLATION.md              - Flash instructions
   ├── README.md                    - Release notes
   └── checksums.md5                - File verification

📁 include/                    - Empty (Arduino framework)

📄 platformio.ini              - Build configuration
📄 README.md                   - Main project README
📄 QUICKSTART.md               - 5-minute quick start
📄 CHANGELOG.md                - Version history
📄 CONTRIBUTING.md             - Contribution guide
📄 LICENSE                     - MIT License
📄 .gitignore                  - Git ignore rules

================================================================================
QUICK START - FLASH PRE-BUILT BINARY
================================================================================

1. Download: releases/Tetriminos-v1.1-MERGED.bin (491 KB)
2. Open M5Burner
3. Select "M5StickC Plus2"
4. Load the bin file
5. Flash to address 0x0
6. Done! Press BtnA to play

================================================================================
QUICK START - BUILD FROM SOURCE
================================================================================

Requirements:
- PlatformIO (pip install platformio)
- M5StickC Plus2 + Mini JoyC Hat
- USB-C cable

Build:
  pio run -e m5stick-c-plus2

Upload:
  pio run -e m5stick-c-plus2 -t upload

================================================================================
CREATE GITHUB REPOSITORY
================================================================================

1. Create new repo on GitHub:
   - Name: Tetriminos (or Tetrimino-Space-Invaders)
   - Description: "Classic Space Invaders with Tetris theme for M5StickC Plus2"
   - Public or Private
   - Don't initialize with README (we have one)

2. Initialize and push this folder:

   cd Tetriminos-GitHub
   git init
   git add .
   git commit -m "Initial commit - Tetrimino Space Invaders v1.1"
   git branch -M main
   git remote add origin https://github.com/YOUR_USERNAME/Tetriminos.git
   git push -u origin main

3. Add topics/tags on GitHub:
   - m5stack
   - m5stickc-plus2
   - arduino
   - esp32
   - game
   - space-invaders
   - tetris
   - retro-gaming
   - platformio

4. Upload a screenshot (recommended):
   - Take photo of game running
   - Add to repo as screenshot.jpg
   - Reference in README.md

5. Create first release:
   - Go to Releases → Create new release
   - Tag: v1.1.0
   - Title: "Tetrimino Space Invaders v1.1 - Feature Complete"
   - Upload releases/Tetriminos-v1.1-MERGED.bin
   - Copy text from releases/README.md

================================================================================
PROJECT INFO
================================================================================

Game: Tetrimino Space Invaders
Version: 1.1 (Feature Complete)
Platform: M5StickC Plus2
Controller: Mini JoyC Hat (I2C)
Framework: Arduino + PlatformIO
Language: C++
Size: 1.1 MB (491 KB merged binary)
Files: 18 files
License: MIT

Features:
✅ Classic Space Invaders gameplay
✅ Tetris-themed graphics
✅ Enemy shooting (Wave 2+)
✅ Bonus lives every 1000 points
✅ High score tracking
✅ Progressive difficulty
✅ Lives system (3-5 lives)
✅ God Mode for testing

================================================================================
FILE VERIFICATION
================================================================================

MD5 Checksums (releases/checksums.md5):

bootloader.bin:              6bf55d2acdd830c317f57426ce8dcfd9
partitions.bin:              a039c66cd3488176037b616b7595fe72
Tetriminos-v1.1-APP.bin:     8cfe5b7d66270389c38e12ceb77bbc15
Tetriminos-v1.1-MERGED.bin:  cbbf38745dce86f9e9ee6860e5565e43

Verify with:
  md5sum -c releases/checksums.md5

================================================================================
DOCUMENTATION
================================================================================

Start here:
1. README.md           - Project overview
2. QUICKSTART.md       - Get running in 5 minutes
3. releases/INSTALLATION.md - Detailed flash guide

Contributing:
4. CONTRIBUTING.md     - How to contribute
5. CHANGELOG.md        - Version history

Legal:
6. LICENSE             - MIT License

================================================================================
HARDWARE REQUIREMENTS
================================================================================

Essential:
- M5StickC Plus2 (ESP32-PICO-V3-02)
- Mini JoyC Hat (I2C joystick at address 0x54)
- USB-C cable (data capable)

Specs:
- Display: 135x240 TFT LCD
- Flash: 4MB
- RAM: 320KB (8.1% used)
- I2C: Pins 0 (SDA), 26 (SCL)

================================================================================
SUPPORT
================================================================================

Having issues?
1. Check releases/INSTALLATION.md
2. Verify binary checksums
3. Test with God Mode (BtnB)
4. Try reflashing

Want to contribute?
1. Read CONTRIBUTING.md
2. Fork repository
3. Make changes
4. Test on hardware
5. Submit pull request

================================================================================
CREDITS
================================================================================

Based on TetroidsM5 framework (Asteroids + Tetris)
Made with ❤️ for retro gaming enthusiasts
Copyright (c) 2025 CoreyMillia

================================================================================

Ready to share with the world! 🚀🎮

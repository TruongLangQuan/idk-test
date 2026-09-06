# Tetriminos Space Invaders - Release Package

## Version 1.2 - Visual Effects (LATEST)

**Download**: [`Tetriminos-v1.2-MERGED.bin`](Tetriminos-v1.2-MERGED.bin) (496 KB)

### What's New in v1.2
- 💥 **Particle explosion system** - Colorful particles on every hit
- 🎯 **Individual block destruction** - Shoot enemies one block at a time
- 🎨 **Random enemy positioning** - Organic, scattered formations
- 💀 **Death animations** - Player explodes into particles when hit
- ⏱️ **Respawn delay** - 2.5 second visual explosion before respawning
- 🎮 **Bottom border** - Clean visual floor at screen bottom
- ✨ **Polish** - Fixed pixel artifacts and visual glitches

---

## Version 1.1 - Feature Complete

### Files Included

- **Tetriminos-v1.1-MERGED.bin** (491 KB) - Complete merged binary for M5Burner
- **Tetriminos-v1.1-APP.bin** (427 KB) - Application binary only
- **bootloader.bin** (17 KB) - ESP32 bootloader
- **partitions.bin** (3 KB) - Partition table
- **Tetriminos-v1.1.json** - M5Burner metadata

### Installation Methods

#### Method 1: M5Burner (Recommended)
1. Open M5Burner
2. Select "M5StickC Plus2" device
3. Load `Tetriminos-v1.1-MERGED.bin`
4. Burn to device

#### Method 2: PlatformIO
```bash
pio run -e m5stick-c-plus2 -t upload
```

#### Method 3: esptool
```bash
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x0 Tetriminos-v1.1-MERGED.bin
```

### Game Features

#### Core Gameplay
- Classic Space Invaders with Tetris theme
- T-piece player ship at bottom
- Enemy formation of small Tetrimino pieces (9×4 grid)
- Vertical shooting mechanic
- 3 lives to start (max 5 with bonuses)

#### Progressive Difficulty
- **Wave 1**: No enemy shooting, slow movement (800ms delay)
- **Wave 2**: Light enemy shooting (30% chance), faster (750ms)
- **Wave 3-4**: Moderate shooting (50% chance), faster (700-650ms)
- **Wave 5+**: Heavy shooting (80% chance), very fast (600ms+)

#### Scoring System
- 10 points per enemy destroyed
- 100 points per wave completed
- **Bonus life every 1000 points** (max 5 lives)

#### Controls
- **Joystick Left/Right**: Move ship horizontally
- **Joystick Button**: Shoot projectile upward
- **BtnB**: Toggle God Mode (invincibility)
- **BtnA**: Restart game (when game over)

### Enemy Shooting System
- Starts at Wave 2
- Only frontmost enemies in each column can shoot
- Red bullets travel downward
- Frequency increases with wave number
-射击间隔: 2 seconds minimum between shots

### Visual Indicators
- **Lives**: Red squares at top right
- **Wave**: Top left (W#)
- **Score**: Top center (S:#)
- **God Mode**: Yellow bar below HUD
- **Enemy Alert**: Red "!" when enemies can shoot (Wave 2+)
- **Bonus Life**: Green "BONUS LIFE!" message appears (2 seconds)

### Technical Details
- Platform: M5StickC Plus2
- Chip: ESP32-PICO-V3-02
- Display: 135×240 LCD
- Controller: Mini JoyC Hat (I2C at 0x54)
- Block Size: 6×6 pixels
- Frame Rate: ~50 FPS (20ms delay)
- RAM Usage: 8.1% (26,644 bytes)
- Flash Usage: 33.3% (436,433 bytes)

### Version History

#### v1.2 (2025-11-08) - Visual Effects ✨
- ✅ Particle explosion system (20 particles)
- ✅ Individual block destruction (3-4 blocks per enemy group)
- ✅ Random enemy positioning for organic formations
- ✅ Death explosion animations for player
- ✅ 2.5 second respawn delay with visual feedback
- ✅ Bottom border/floor visual
- ✅ Fixed pixel artifacts
- ✅ Scoring adjusted (5 points per block)

#### v1.1 (2025-11-08) - Feature Complete ✅
- ✅ Fixed lives system (properly decrements on hit)
- ✅ Added enemy shooting (starts Wave 2)
- ✅ Progressive enemy fire rate (30% → 50% → 80%)
- ✅ Bonus life every 1000 points (max 5 lives)
- ✅ High score tracking
- ✅ "BONUS LIFE!" notification
- ✅ Enemy shooting alert indicator
- ✅ Improved HUD layout

#### v1.0 (2025-11-08) - Initial Release
- ✅ Basic Space Invaders gameplay
- ✅ Enemy formation movement
- ✅ Player shooting
- ✅ Wave progression
- ✅ Lives system
- ✅ God Mode

### Requirements
- M5StickC Plus2
- Mini JoyC Hat (I2C controller)
- USB-C cable for programming

### Known Issues
None! Game is fully functional and tested.

### Credits
Based on TetroidsM5 framework (Tetris + Asteroids mashup)

### License
MIT License

---

**Ready to play! 🎮**
Flash `Tetriminos-v1.1-MERGED.bin` to your M5StickC Plus2 and enjoy!

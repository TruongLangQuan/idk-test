# Tetriminos Space Invaders - Installation Guide

## Quick Start

### For M5Burner Users (Easiest)

1. **Download**: `Tetriminos-v1.1-MERGED.bin` (491 KB)
2. **Open M5Burner**
3. **Select Device**: M5StickC Plus2
4. **Load Binary**: Browse to the downloaded file
5. **Burn**: Click burn button
6. **Done!** Game will start automatically

### Binary File
```
File: Tetriminos-v1.1-MERGED.bin
Size: 491 KB (502,864 bytes)
MD5:  cbbf38745dce86f9e9ee6860e5565e43
```

This is a **complete merged binary** containing:
- ESP32 bootloader (0x1000)
- Partition table (0x8000)
- Application firmware (0x10000)

### Flash Address
Flash to address **0x0** (beginning of flash)

---

## Alternative Installation Methods

### Method 2: esptool (Command Line)

```bash
# Install esptool if needed
pip install esptool

# Flash the merged binary
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x0 Tetriminos-v1.1-MERGED.bin
```

**Windows users**: Replace `/dev/ttyACM0` with your COM port (e.g., `COM3`)

### Method 3: PlatformIO (For Developers)

If you have the source code:

```bash
cd Tetriminos
pio run -e m5stick-c-plus2 -t upload
```

### Method 4: Manual Flash (Advanced)

If you want to flash components separately:

```bash
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 Tetriminos-v1.1-APP.bin
```

---

## First Boot

After flashing:

1. **Title Screen** appears: "TETRIMINO SPACE INVADERS"
2. **Press BtnA** to start
3. **Game begins** with Wave 1

---

## Game Controls

### Mini JoyC Hat
- **Left/Right**: Move player ship
- **Button**: Shoot (hold for rapid fire)

### M5StickC Plus2 Buttons
- **BtnA**: Restart (when game over)
- **BtnB**: Toggle God Mode (for testing)

---

## Gameplay Overview

### Objective
Destroy all enemies to advance waves. Don't let them reach the bottom!

### Starting Conditions
- **Lives**: 3 (shown as red squares top-right)
- **Wave**: 1
- **Score**: 0

### Scoring
- **10 points** per enemy destroyed
- **100 points** per wave completed
- **Bonus life** every 1,000 points (max 5 lives)

### Wave Progression

| Wave | Enemy Speed | Enemy Shooting |
|------|-------------|----------------|
| 1    | 800ms       | None           |
| 2    | 750ms       | Light (30%)    |
| 3-4  | 700-650ms   | Moderate (50%) |
| 5+   | 600ms+      | Heavy (80%)    |

### Enemy Behavior
- Formation moves horizontally as a group
- Drops down and reverses at screen edges
- Starting Wave 2: Enemies shoot red bullets downward
- Only frontmost enemies in each column can shoot

### Game Over Conditions
1. All lives lost (hit by enemy or bullet)
2. Enemy reaches bottom of screen

---

## Display Elements

### HUD (Top Bar)
- **Top-Left**: Wave number (W#)
- **Top-Center**: Score (S:#)
- **Top-Right**: Lives remaining (red squares)
- **Red "!"**: Enemy shooting active (Wave 2+)
- **Yellow bar**: God Mode active

### In-Game Messages
- **"BONUS LIFE!"** (green): Appears when you earn 1000 points
- **"WAVE #"** (green): Appears when advancing to next wave
- **"GAME OVER"** (red): Game over screen with stats

---

## Troubleshooting

### Device Not Recognized
- Try a different USB cable (must support data transfer)
- Install USB drivers for ESP32
- Check device appears as /dev/ttyACM0 (Linux/Mac) or COM port (Windows)

### Flash Fails
- Hold BtnA while connecting USB cable
- Try lower baud rate: `--baud 115200`
- Verify MD5 checksum of downloaded file

### Game Doesn't Start
- Press BtnA on title screen
- Try power cycling the device
- Reflash the merged binary

### Joystick Not Working
- Ensure Mini JoyC Hat is properly connected
- Check I2C connection (pins 0, 26)
- Joystick I2C address is 0x54

---

## Hardware Requirements

### Essential
- **M5StickC Plus2** (ESP32-PICO-V3-02)
- **Mini JoyC Hat** (I2C controller)
- **USB-C cable** (data capable)

### Specifications
- Flash: 4MB
- RAM: 320KB (8.1% used by game)
- Display: 135×240 TFT LCD
- I2C: Pins 0 (SDA), 26 (SCL)

---

## File Checksums

Verify your download with MD5:

```
bootloader.bin:          6bf55d2acdd830c317f57426ce8dcfd9
partitions.bin:          a039c66cd3488176037b616b7595fe72
Tetriminos-v1.1-APP.bin: 8cfe5b7d66270389c38e12ceb77bbc15
Tetriminos-v1.1-MERGED.bin: cbbf38745dce86f9e9ee6860e5565e43
```

---

## Support

### Issues?
- Check wiring and connections
- Verify binary checksum
- Try reflashing
- Test with God Mode (BtnB)

### Want to Modify?
Source code available in main project folder. Build with PlatformIO.

---

## Version Information

**Version**: 1.1 (Feature Complete)  
**Release Date**: November 8, 2025  
**Build**: 20251108_113625  
**Platform**: M5StickC Plus2  
**Framework**: Arduino + ESP32  

---

**Ready to Play! 🎮**

Flash the merged binary and enjoy classic Space Invaders with a Tetris twist!

# Quick Start Guide

Get Tetrimino Space Invaders running in 5 minutes! 🚀

## Hardware Needed

- ✅ **M5StickC Plus2**
- ✅ **Mini JoyC Hat** (I2C joystick)
- ✅ **USB-C cable**

## Option 1: Flash Pre-Built Binary (Recommended)

### Step 1: Download Binary
Download [`releases/Tetriminos-v1.1-MERGED.bin`](releases/Tetriminos-v1.1-MERGED.bin) (491 KB)

### Step 2: Flash with M5Burner
1. Open **M5Burner** application
2. Select **"M5StickC Plus2"** from device list
3. Click **"Custom"** or **"Load File"**
4. Browse to downloaded `Tetriminos-v1.1-MERGED.bin`
5. Make sure address is set to **`0x0`**
6. Click **"Burn"** button
7. Wait for completion (about 10 seconds)

### Step 3: Play!
- Disconnect USB
- Reconnect power
- Press **BtnA** on title screen to start

## Option 2: Build from Source

### Prerequisites
```bash
# Install PlatformIO
pip install platformio

# Or use PlatformIO IDE
# Download from: https://platformio.org/platformio-ide
```

### Build & Upload
```bash
# Clone repository (replace with actual URL)
git clone https://github.com/yourusername/Tetriminos.git
cd Tetriminos

# Build project
pio run -e m5stick-c-plus2

# Upload to device (connect via USB first)
pio run -e m5stick-c-plus2 -t upload
```

### Verify Upload
```bash
# Monitor serial output
pio device monitor

# Should see:
# "Tetrimino Space Invaders Starting..."
```

## Controls

### Mini JoyC Hat
| Control | Action |
|---------|--------|
| ← Left  | Move ship left |
| → Right | Move ship right |
| 🔘 Button | Shoot |

### M5StickC Buttons
| Button | Action |
|--------|--------|
| BtnA | Restart game (when game over) |
| BtnB | Toggle God Mode |

## First Game

1. **Title Screen** appears: "TETRIMINO SPACE INVADERS"
2. Press **BtnA** to start
3. You spawn at the bottom with **3 lives** (red squares top-right)
4. **36 enemies** appear in formation at top
5. Use joystick to **move left/right**
6. Press joystick **button to shoot**
7. Destroy all enemies to advance to **Wave 2**

### Tips for Beginners
- Stay centered to have room to dodge
- Watch for the red "!" indicator - enemies will shoot!
- Bonus life at 1,000 points
- Use BtnB (God Mode) to practice

## Gameplay Goals

### Wave 1
- No enemy shooting
- Learn controls
- Clear formation (360 points)

### Wave 2+
- Enemies shoot back (red bullets)
- Faster movement
- More aggressive

### Scoring Milestones
- **1,000 pts** - First bonus life! 🎉
- **2,000 pts** - Second bonus life
- **5,000 pts** - Wave 5+ territory (intense!)

## Troubleshooting

### Device Not Recognized
```bash
# Linux: Check device name
ls /dev/tty*

# Should see /dev/ttyACM0 or similar

# Windows: Check COM port in Device Manager
```

### Upload Fails
- Hold **BtnA** while connecting USB
- Try different USB cable (must support data)
- Lower baud rate in platformio.ini

### Joystick Not Working
- Verify Mini JoyC Hat is seated properly
- Check I2C pins (0=SDA, 26=SCL)
- Try uploading again

### Game Won't Start
- Press **BtnA** on title screen
- Power cycle device
- Reflash binary

## Display Elements

### HUD (Top Bar)
```
W2    S:1450    ❤️❤️❤️
↑     ↑         ↑
Wave  Score     Lives
```

### Indicators
- **Red "!"** - Enemy shooting active (Wave 2+)
- **Yellow bar** - God Mode enabled
- **Green "BONUS LIFE!"** - Earned 1000 points

## Game Over Screen
```
GAME OVER

Wave: 5
Score: 2340
NEW HIGH!    ← (if beat record)

Press BtnA
to restart
```

## Performance Tips

- Game runs at **~50 FPS**
- Smooth movement and collision
- No lag with full enemy formation
- Memory usage under 10%

## Next Steps

1. ✅ Beat Wave 1
2. ✅ Survive enemy shooting (Wave 2)
3. ✅ Earn first bonus life (1000 pts)
4. ✅ Reach Wave 5 (heavy shooting!)
5. ✅ Set a high score

## Need Help?

- 📖 [Full README](README.md)
- 📥 [Installation Guide](releases/INSTALLATION.md)
- 🐛 [Report Issues](../../issues/)
- 💡 [Contributing Guide](CONTRIBUTING.md)

---

**Ready to play! Good luck defeating the Tetrimino invaders!** 🎮

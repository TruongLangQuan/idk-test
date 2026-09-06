# ThreeTris Quick Start Guide 🚀

Get up and running with ThreeTris in 5 minutes!

## 📦 What You Need

- M5StickC Plus2
- Mini JoyC Hat
- USB-C cable
- Computer with M5Burner or esptool

## ⚡ Quick Install (M5Burner)

1. **Download M5Burner**
   - Visit: https://docs.m5stack.com/en/download
   - Install for your OS (Windows/Mac/Linux)

2. **Get ThreeTris**
   - Download `ThreeTris-v3.0-MERGED.bin` from the `releases/` folder
   - This file includes everything needed!

3. **Connect Hardware**
   - Attach Mini JoyC Hat to M5StickC Plus2 Grove port
   - Connect M5StickC Plus2 to computer via USB-C

4. **Flash the Game**
   - Open M5Burner
   - Select your COM port
   - Click "Add" and choose `ThreeTris-v3.0-MERGED.bin`
   - Click "Burn"
   - Wait ~30 seconds

5. **Play!**
   - Device will restart automatically
   - See animated title screen
   - Press JoyC button to start
   - Enjoy! 🎮

## 🎮 First Game

### Title Screen
- Wait for animated pieces
- Press **JoyC center button** to start

### Playing
- **D-Pad Left/Right**: Move pieces
- **JoyC Button**: Rotate clockwise
- **M5 Button A**: Drop faster
- **M5 Button B**: Hold/swap piece

### Tips
- Use the ghost piece to see where blocks will land
- Hold piece lets you save a piece for later
- Watch the next piece preview to plan ahead
- Clear multiple lines for bonus points!

## 🔧 Troubleshooting

**Game won't start?**
- Check JoyC Hat is firmly connected
- Try pressing M5 button A to wake device
- Reflash the firmware

**Controls not working?**
- Verify JoyC Hat connection
- Check I2C address (should be 0x54)
- Restart device

**Display issues?**
- Make sure device is in portrait mode
- Reflash with MERGED.bin (not FINAL.bin)

## 📊 Game Mechanics

### Scoring
- 1 line = 100 points × level
- 2 lines = 300 points × level
- 3 lines = 500 points × level
- 4+ lines = 800 points × level

### Level Up
- Clear 10 lines = level up
- Speed increases each level
- Lock delay changes at levels 5, 10, 15, 20

### Difficulty
The game gets progressively harder:
- **Levels 1-5**: Beginner (300ms lock delay)
- **Levels 6-10**: Intermediate (400ms)
- **Levels 11-15**: Advanced (500ms)
- **Levels 16-20+**: Expert (600ms)

## 🎯 Advanced Tips

1. **Master the Hold System**
   - Save I-pieces for clearing multiple lines
   - Keep useful pieces for difficult situations

2. **Plan Ahead**
   - Watch the next piece preview
   - Think 2-3 moves ahead
   - Don't rush!

3. **Use Ghost Piece**
   - Shows exact landing position
   - Perfect for quick drops
   - Helps avoid mistakes

4. **Practice Rotation**
   - All pieces rotate clockwise
   - Learn rotation patterns
   - Practice near walls

## 🏆 High Score Challenge

- Can you reach level 10?
- Can you clear 50 lines?
- Can you score 10,000 points?

Share your high scores! 🎮

## 📱 Sharing

Record your gameplay and share:
- Screenshot final scores
- Post to social media
- Tag #ThreeTris #M5Stack

---

**Enjoy playing ThreeTris! 🎉**

Questions? Check the full [README.md](README.md) or open an issue on GitHub.

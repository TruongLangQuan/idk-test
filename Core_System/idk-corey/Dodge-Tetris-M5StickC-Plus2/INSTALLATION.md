# M5Stick Dodge - 4 Wave Challenge
## Installation Guide

### Quick Start
1. **Hardware Required:**
   - M5StickC Plus 2
   - Mini Joy C Hat (joystick controller)

2. **Flash Firmware:**
   - Use **M5Burner** (recommended) or **esptool**
   - Flash file: `M5StickDodge4Wave-v1.0-MERGED.bin`
   - Flash address: `0x0`

3. **Connect & Play:**
   - Attach Mini Joy C Hat to M5StickC Plus2
   - Power on and enjoy!

---

## Installation Methods

### Option 1: M5Burner (Easiest)
1. Download and install M5Burner from https://m5stack.com/pages/download
2. Connect M5StickC Plus2 via USB
3. In M5Burner, select "M5StickC Plus 2" device
4. Click "Custom" and load `M5StickDodge4Wave-v1.0-MERGED.bin`
5. Click "Burn" and wait for completion

### Option 2: esptool (Command Line)
```bash
# Install esptool if needed
pip install esptool

# Flash merged binary (all-in-one)
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x0 M5StickDodge4Wave-v1.0-MERGED.bin

# Or flash individual components
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 M5StickDodge4Wave-v1.0-APP.bin
```

**Note:** Replace `/dev/ttyACM0` with your actual port:
- Linux: `/dev/ttyACM0` or `/dev/ttyUSB0`
- Mac: `/dev/cu.usbserial-*`
- Windows: `COM3`, `COM4`, etc.

### Option 3: PlatformIO (Development)
```bash
cd M5StickDodge4Wave
pio run --target upload
```

---

## Game Controls

### Menu Navigation
- **Joystick Up/Down:** Select game (Dodge or Tetris)
- **BtnA (side button):** Start selected game

### In-Game (Dodge)
- **Joystick:** Move player (T-piece) in all directions
  - Portrait mode: Up→Right, Down→Left, Left→Up, Right→Down
- **Joystick Button (press):** Rotate player piece
- **BtnB (front button):** Return to menu

### Screen Layout
```
┌─────────────────┐
│ S:120 Lv:2 W:3  │ ← Score, Level, Wave
├─────────────────┤
│                 │
│   ████   ████   │ ← Falling obstacles
│     ▓▓▓▓        │
│                 │
│        ▼        │ ← Your T-piece
│       ▼▼▼       │
└─────────────────┘
```

---

## Wave System Breakdown

### 🎮 Wave 1 (0-30 seconds): Tetris Pieces
- **Obstacles:** 7 classic Tetris shapes (I, O, T, S, Z, L, J)
- **Colors:** Each piece type has its own color
- **Behavior:** Random spawning with rotation
- **Difficulty:** Beginner-friendly

### 🔤 Wave 2 (30-60 seconds): TETRIS Letters
- **Obstacles:** Sequential letters spelling T→E→T→R→I→S
- **Special Feature:** Backwards R for retro arcade style!
- **Colors:** Each letter has unique bright color
  - T = Cyan
  - E = Yellow
  - T = Green (different from first T!)
  - R = Red (backwards!)
  - I = Orange
  - S = Magenta
- **Behavior:** Letters appear in order, repeating the sequence
- **Difficulty:** Slightly slower for readability

### 💎 Wave 3 (60-90 seconds): Pattern Shapes
- **Obstacles:** 8 creative block patterns
  - Plus (+) - Yellow
  - Diamond (◇) - Cyan
  - Arrow (↓) - Magenta
  - Heart (♥) - Red
  - Zig-zag - Green
  - Cross (X) - Orange
  - Stairs - Cyan
  - Dot cluster - White
- **Behavior:** Random selection from 8 shapes
- **Difficulty:** Moderate speed increase

### ♾️ Wave 4+ (90+ seconds): Mixed Chaos
- **Obstacles:** Random mix of ALL three types!
  - 33% chance: Tetris pieces (random)
  - 33% chance: TETRIS letters (still sequential!)
  - 33% chance: Pattern shapes (random)
- **Behavior:** Continuously accelerating difficulty
- **Difficulty:** Endless escalation - survive as long as you can!

---

## Scoring System

- **+10 points** for each obstacle that passes off-screen
- **Level up** every 100 points
- **Speed increases** with each level
- **Spawn rate** gets faster every wave and level

**Example Progress:**
```
Score:   0 → Level: 1 → Spawn: 1.5s → Speed: 1.0x
Score: 100 → Level: 2 → Spawn: 1.3s → Speed: 1.2x
Score: 200 → Level: 3 → Spawn: 1.0s → Speed: 1.4x
Score: 300 → Level: 4 → Spawn: 0.8s → Speed: 1.6x
```

---

## Troubleshooting

### Game won't start
- Ensure Mini Joy C Hat is properly connected
- Check I2C connections: SDA=0, SCL=26
- Try power cycling the device

### Controls not responding
- Verify joystick is working (LED should light up)
- Check hat is seated firmly on M5StickC
- Restart the device

### Display issues
- Portrait mode is required (135x240)
- If screen is rotated wrong, reflash firmware

### Flash failed
- Try lower baud rate: `--baud 115200`
- Hold down the power button during flashing
- Check USB cable supports data transfer

---

## Technical Specifications

**Hardware:**
- Device: M5StickC Plus 2
- Chip: ESP32-PICO-V3-02
- Display: 135x240 TFT (portrait)
- Controller: Mini Joy C Hat (I2C)

**Performance:**
- Frame Rate: 60 FPS
- RAM Usage: 27KB (8.3%)
- Flash Usage: 443KB (33.8%)
- Response Time: <16ms

**Firmware:**
- Version: 1.0.0
- Build: ESP32 Arduino Framework
- Bootloader: 0x1000
- Partitions: 0x8000
- Application: 0x10000

---

## Credits

Developed collaboratively with **GitHub Copilot CLI**

Special features:
- Sequential TETRIS letter system
- Backwards R for retro arcade authenticity
- 8 hand-crafted block patterns
- Smooth pixel-perfect collision

Enjoy the game! 🎮✨

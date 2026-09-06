# 1942 Tetrimino Edition - v2.3 FINAL RELEASE

**Release Date**: November 10, 2025  
**Platform**: M5StickC Plus2  
**Development Time**: ~6 hours

---

## 🎮 What is 1942 Tetrimino Edition?

A modern tribute to Capcom's classic 1942 arcade shooter, reimagined with Tetris-inspired visuals. Pilot your T-piece fighter through endless waves of Tetrimino enemies, collect power-ups, battle massive bosses, and see how long you can survive as the difficulty ramps up to insane levels!

---

## ✨ Key Features

### 🎬 Animated Title Screen
- Flying Tetrimino formations (I-pieces and Z-pieces)
- Scrolling starfield background
- Player plane oscillates and shoots randomly
- Pulsing "PRESS ANY BUTTON" text
- Shows control hints

### �� Auto-Fire System (NEW!)
- **Press Button A** to toggle auto-fire on/off
- No more carpal tunnel from holding the fire button!
- "AUTO" indicator on HUD when active
- Same fire rate as manual (150ms cooldown)

### 👾 20 Different Enemy Types
- **6 Tetriminos**: I, O, L, J, S, Z pieces
- **6 TETRIS Letters**: T-E-T-R-I-S in sequence (Wave 6+)
- **8 Strange Symbols**: Plus, Diamond, Arrow, Heart, Zig, Cross, Stairs, Spiral
- Large variants (1.5x size) for extra challenge

### 🔥 Power-Up System
- **Collect "P" pickups** spawning every 20-45 seconds
- Stacks up to **3 levels** (max 6 wing guns!)
- Power-ups act as **shields** - lose them before losing lives
- Each level adds 2 wing guns (1 left, 1 right)
- Wing guns fire in perfect sync with main gun

### 🦾 Progressive Boss Scaling
- Boss appears every 60 seconds
- **Size scales**: 2x → 8x (HUGE by Wave 13!)
- **Health scales**: 20 HP → unlimited (never caps!)
- **Speed scales**: Gets faster each wave
- **Fire rate maxes** at Wave 4 (200ms = bullet hell!)

### 📊 End-Game Statistics
Comprehensive stats displayed on Game Over:
- **Score**: Total points earned
- **Wave**: Highest wave reached  
- **Shots**: Total bullets fired
- **Kills**: Total enemies destroyed
- **Hit %**: Your accuracy percentage

### 🎮 Smooth Controls
- **Joystick**: 8-direction movement
- **Joystick Button**: Fire (or auto when enabled)
- **Button A**: Toggle Auto-Fire
- **Button B**: Toggle God Mode (testing)

---

## 📈 Difficulty Progression

### Wave Structure
- **Waves 1-5**: Regular Tetriminos only
- **Wave 6**: TETRIS letters + strange symbols
- **Wave 7+**: Mix of everything (33% each type)

### Scaling Metrics

| Metric | Wave 1 | Wave 5 | Wave 10 | Max | Caps At |
|--------|--------|--------|---------|-----|---------|
| **Enemy Speed** | 1.0x | 1.6x | 2.35x | 3.0x | Wave 14+ |
| **Spawn Rate** | 1200ms | 800ms | 650ms | 600ms | Wave 7+ |
| **Enemy Fire** | 1000ms | 800ms | 550ms | 400ms | Wave 13+ |
| **Boss Fire** | 500ms | 300ms | 200ms | 200ms | Wave 4+ |
| **Boss Health** | 20 HP | 40 HP | 65 HP | ∞ | Never! |
| **Boss Size** | 2.0x | 4.0x | 6.5x | 8.0x | Wave 13+ |

### Peak Difficulty (Wave 14-15)
When all metrics except boss health are maxed:
- Enemies moving at 3x speed
- Spawning every 600ms (1.67/sec)
- Firing every 400ms
- Bosses firing every 200ms (bullet hell!)
- Boss size at maximum 8.0x

**After this**: Same difficulty, but bosses get tankier forever!

---

## 🚀 Quick Start

### Flash the Game
```bash
esptool.py --chip esp32 --port /dev/ttyACM0 write_flash 0x0 1942_Tetrimino-v2.3-MERGED.bin
```

### Play
1. Boot device → See animated title screen
2. Press any button → Start game
3. Survive waves, collect power-ups, defeat bosses!
4. Game Over → See your stats
5. Press button → Back to title screen

---

## 📦 Release Package Contents

```
releases/
├── 1942_Tetrimino-v2.3-MERGED.bin       (506 KB) ← Flash this!
├── 1942_Tetrimino-v2.3-RELEASE.json     (Complete metadata)
└── 1942_Tetrimino_v2.3_FINAL_*.bin      (442 KB, firmware only)

Documentation:
├── RELEASE_NOTES_v2.3.md                (This file)
├── DIFFICULTY_PROGRESSION.md            (Detailed scaling charts)
├── README.md                            (Project overview)
└── QUICKSTART.md                        (Setup guide)
```

---

## 🎯 Gameplay Tips

1. **Use Auto-Fire** - Toggle with Button A, save your thumb!
2. **Prioritize Power-Ups** - They're shields AND firepower
3. **Dodge First, Shoot Second** - Bullets are infinite, lives aren't
4. **Learn Boss Patterns** - They move side-to-side predictably
5. **Stay Mobile** - Don't camp corners in later waves
6. **Accuracy Matters** - Better accuracy = higher scores
7. **Practice Mode** - Use God Mode (Button B) to learn patterns

---

## 🔧 Technical Specs

- **RAM Usage**: 8.7% (28,500 / 327,680 bytes)
- **Flash Usage**: 34.5% (452,240 / 1,310,720 bytes)
- **Frame Rate**: 60 FPS locked
- **Code Size**: 1200+ lines of C++
- **Enemy Types**: 20 unique visual patterns
- **Max Enemies**: 15 simultaneous
- **Max Bullets**: 20 player + 30 wing + 30 enemy
- **Particles**: Up to 50 simultaneously

---

## 🐛 Known Issues

**None!** This is a stable, polished release. 🎉

---

## 💡 Future Enhancement Ideas

- Sound effects and music
- High score persistence (EEPROM)
- Multiple difficulty modes
- Different player ships
- Special weapons (bombs, lasers)
- Achievement system
- Multiplayer co-op
- More boss patterns

---

## 🙏 Credits

### Inspired By
- **Capcom's 1942** (1984) - Classic vertical shooter gameplay
- **Tetris** (1984) - Block-based visual design
- **Previous Projects**: Tetriminos, M5StickDodge4Wave, TetroidsM5

### Built With
- Arduino framework (PlatformIO)
- M5GFX graphics library
- Custom UNIT_MiniJoyC driver
- All sprites procedurally generated

### Development
- **Platform**: M5StickC Plus2 (ESP32-PICO)
- **Time**: Single night (Nov 9-10, 2025)
- **Duration**: ~6 hours total
- **Iterations**: 8 major versions (v1.0 → v2.3)

---

## 📝 Version History

- **v2.3 FINAL** - Smooth difficulty progression, release ready
- **v2.2** - Progressive boss scaling up to 8x
- **v2.1** - Animated title screen
- **v2.0** - Endless mode, letters/symbols, mixed waves
- **v1.6** - TETRIS letters + strange symbols
- **v1.5** - Five waves working
- **v1.4** - Wave progression system
- **v1.3** - Fixed wing gun positioning  
- **v1.2** - Power-up system added
- **v1.1** - Fixed controls + score display
- **v1.0** - First working version

---

## 🎮 Ready to Play?

Flash the merged binary and prepare for action-packed arcade gameplay!

**Good luck, pilot! The Tetrimino air force is counting on you!** ✈️🎯

---

*1942 Tetrimino Edition v2.3 FINAL - November 10, 2025*

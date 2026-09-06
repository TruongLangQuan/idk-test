# 1942 Tetrimino Edition - Quick Start Guide

## 🎮 Controls

| Input | Action |
|-------|--------|
| **Joystick** | Move your T-piece plane in 8 directions |
| **Joystick Button** | Fire bullets upward |
| **Button B** | Toggle God Mode (invincibility) |

## 🎯 Objective

Survive waves of enemy Tetrimino planes by shooting them down while dodging their fire. Each wave gets progressively harder with more enemies, faster movement, and complex formations.

## 💡 Pro Tips

### Survival Tips
1. **Stay Mobile**: Keep moving! Standing still makes you an easy target
2. **Bottom Third**: Stay in the lower third of the screen for more reaction time
3. **Diagonal Movement**: Use diagonal inputs to dodge while staying aligned to shoot
4. **Watch Patterns**: Learn enemy formation types to predict movement
5. **Use Invincibility**: After getting hit, use your 2 seconds wisely to reposition

### Combat Tips
1. **Lead Your Shots**: Aim slightly ahead of fast-moving enemies
2. **Prioritize Shooters**: From Wave 2 onward, focus on enemies that fire back
3. **Arc Formation**: Enemies from the sides are predictable - shoot early
4. **Swooping Enemies**: They zigzag - shoot when they're on the upswing
5. **Clear Quickly**: Faster wave completion = less time under fire

### Advanced Strategies
1. **Corner Juke**: Move to corners to funnel enemies into firing lanes
2. **Vertical Advantage**: Moving up the screen gives more bullet range
3. **Side Dodging**: Horizontal movement is safer than vertical (less enemy fire)
4. **Bullet Management**: Max 10 bullets - don't spam, aim carefully
5. **Wave Transitions**: Use the brief pause between waves to center yourself

## 📊 Understanding the HUD

```
┌─────────────────────────┐
│ S:0    W:1          ♥♥♥ │
│                         │
└─────────────────────────┘
  ↑      ↑            ↑
  │      │            └─ Lives remaining (3 max)
  │      └─ Current wave number
  └─ Score (10 pts per enemy)
```

## 🌊 Wave Progression

| Wave | What's New? |
|------|-------------|
| 1 | Straight line enemies, no shooting |
| 2 | Arc formations, **enemies start shooting** |
| 3 | Swooping diagonal attacks added |
| 4+ | V-formations, all patterns mixed |

**Difficulty Scaling:**
- More enemies per wave (+2 each wave, max 15)
- Faster enemy speed (+10% per wave)
- Faster spawn rate (down to 1 second minimum)
- More frequent enemy fire

## 🎨 Enemy Types

Each enemy is a different Tetrimino piece with unique colors:

| Piece | Color | Shape |
|-------|-------|-------|
| I | Cyan | Line |
| O | Yellow | Square |
| L | Orange | L-shape |
| J | Blue | Reverse L |
| S | Green | S-shape |
| Z | Red | Z-shape |

**All worth 10 points!** No difference in behavior - just variety!

## 💀 Death & Respawning

- **3 Lives**: Shown as small magenta blocks in top-right
- **Getting Hit**: You become invincible for 2 seconds (blinking effect)
- **Game Over**: When all lives are lost, press joystick button to restart

## 🛡️ God Mode

Press **Button B** to toggle invincibility:
- No damage from enemies or bullets
- "GOD" appears on HUD
- Perfect for practice or just having fun!

## 📈 Scoring Milestones

| Score | Achievement |
|-------|-------------|
| 50 | Survived Wave 1! |
| 200 | Getting the hang of it |
| 500 | Serious pilot! |
| 1000 | Ace status! |
| 2000+ | Legendary! |

## 🐛 Troubleshooting

**JoyC not responding?**
- Check I2C connection (pins 0, 26)
- Ensure JoyC hat is firmly seated
- Try power cycling the M5StickC

**Game running slow?**
- Should be 60 FPS - check serial monitor for errors
- Ensure latest M5StickCPlus2 library installed

**Bullets not firing?**
- 150ms cooldown between shots
- Max 10 bullets on screen at once
- Press joystick button firmly

## 🎓 Training Exercises

### Exercise 1: Movement Mastery
- Use God Mode
- Practice moving in all 8 directions smoothly
- Stay in the center while dodging

### Exercise 2: Aim Training
- Wave 1, no God Mode
- Try to hit every enemy
- Practice leading your shots

### Exercise 3: Survival Test
- Turn off God Mode
- See how many waves you can survive
- Focus on dodging over shooting

### Exercise 4: High Score Run
- No God Mode
- Maximize score by clearing waves quickly
- Don't miss any enemies

## 🎯 Challenge Modes (Self-Imposed)

1. **Pacifist Run**: How long can you survive without shooting?
2. **Perfect Run**: Complete a wave without getting hit
3. **Speed Run**: How fast can you reach Wave 10?
4. **Bottom Feeder**: Never move above the bottom 1/3 of screen
5. **One Life**: Play with God Mode but pretend you only have 1 life

## 🏆 Achievements (Unofficial)

- ⭐ **First Blood**: Destroy your first enemy
- ⭐ **Wave Warrior**: Complete Wave 5
- ⭐ **Bullet Hell**: Dodge 10 enemy bullets in one life
- ⭐ **Sharpshooter**: 10 kills without missing
- ⭐ **Survivor**: Reach Wave 10
- ⭐ **Untouchable**: Complete a wave without damage
- ⭐ **Century Club**: Score 100+ points
- ⭐ **Ace Pilot**: Score 500+ points
- ⭐ **Legend**: Score 1000+ points

---

## 🎮 Ready to Play?

**Build & Upload:**
```bash
cd /home/coreymillia/Documents/194T2
pio run --target upload
```

**Good luck, pilot!** ✈️

The Tetrimino air force is counting on you! 🎯

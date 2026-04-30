# 🎮 TETROIDS - Design Document

**Tetris meets Asteroids for M5StickC Plus2**

---

## 🎯 Core Concept

**Player:** T-piece spaceship that rotates and shoots  
**Enemies:** Floating Tetris pieces (asteroids)  
**Objective:** Survive waves, destroy asteroids, get high score

---

## 🚀 Player Ship (T-Piece)

### Movement
- **Rotation:** Joystick Left/Right = rotate ship (360°, 8 directions)
- **Thrust:** Joystick Up = move forward in facing direction
- **Momentum:** Ship continues drifting (asteroids physics)
- **Screen Wrap:** Go off edge, appear on opposite side

### Combat
- **Shoot:** Joy Button or BtnA = fire bullet
- **Rate of Fire:** 0.2 seconds between shots
- **Bullet Speed:** Fast, straight line
- **Bullet Limit:** Max 4 bullets on screen

### Visual
```
Rotation 0 (UP):     Rotation 2 (RIGHT):
    ▀▀▀                  ▐
     █                   ▐▀▀
     █                   ▐

Rotation 4 (DOWN):   Rotation 6 (LEFT):
     █                   ▀▀▌
     █                     ▌
    ▀▀▀                    ▌
```

---

## 💎 Asteroids (Tetris Pieces)

### Large Asteroids (Full Pieces)
- **Types:** I, O, L, J, S, Z pieces (NOT T - that's the player)
- **Movement:** Slow drift in random direction
- **Rotation:** Slowly spinning
- **Health:** 1 hit = break into 4 blocks
- **Colors:** Match Tetris colors (I=cyan, O=yellow, etc)

### Small Asteroids (Individual Blocks)
- **Size:** Single square (from broken piece)
- **Movement:** Faster drift from parent direction
- **Health:** 1 hit = destroyed completely
- **Points:** 10 pts for large, 5 pts for small

### Spawning
- **Wave Start:** 3-5 large asteroids
- **Random Position:** Edges of screen
- **Random Velocity:** Slow, random direction
- **Random Rotation:** 0.5-2 rpm

---

## 🎮 Controls

| Input | Action |
|-------|--------|
| **Joystick Left** | Rotate ship counter-clockwise |
| **Joystick Right** | Rotate ship clockwise |
| **Joystick Up** | Thrust forward |
| **Joy Button** | Fire bullet |
| **BtnA** | Fire bullet (alternative) |
| **BtnB** | Pause/Menu |

---

## 📊 Game Flow

### Start Screen
```
┌─────────────┐
│  TETROIDS   │
│             │
│   ▀▀▀       │  (T-piece ship)
│    █        │
│    █        │
│             │
│ Press START │
└─────────────┘
```

### Gameplay Loop
1. **Wave Start** → Spawn asteroids
2. **Player Moves** → Thrust, rotate, shoot
3. **Collisions** → Bullets hit asteroids
4. **Breaking** → Large → 4 small blocks
5. **Wave Clear** → All destroyed = next wave
6. **Death** → Hit by asteroid = game over

### Scoring
- **Small block destroyed:** 10 points
- **Large asteroid broken:** 50 points
- **Wave completed:** 100 × wave number
- **Survival time:** 1 point per second

---

## 🎨 Visual Design

### Display (135×240 Portrait)
```
┌─────────────┐ 135px
│ Score: 1234 │ ← Top HUD
│ Wave: 2     │
├─────────────┤
│             │
│   💎  ▀▀▀   │
│      █      │ ← Play area
│   💎  █     │   (180px tall)
│      💎     │
├─────────────┤
│ Lives: ❤❤❤ │ ← Bottom HUD
└─────────────┘ 240px
```

### Colors
- **Player T:** Magenta/Purple (0xF81F)
- **Bullets:** White (0xFFFF)
- **Asteroids:** Tetris colors
- **Background:** Black (0x0000)
- **Stars:** White dots (scrolling)

---

## 💥 Physics System

### Velocity
- **Player:** Max speed 3.0 pixels/frame
- **Thrust:** Acceleration 0.3 pixels/frame²
- **Friction:** 0.95 (gradual slowdown)
- **Large Asteroid:** 0.5-1.5 pixels/frame
- **Small Block:** 1.0-2.0 pixels/frame
- **Bullet:** 5.0 pixels/frame

### Collision
- **Player vs Asteroid:** Death
- **Bullet vs Asteroid:** Destroy/Break
- **Screen Wrap:** X and Y both wrap

### Breaking Physics
```
Large I-piece hit:
   ████  →  💥  →  █ █ █ █
                    ↗ ↗ ↘ ↘
                    (4 blocks scatter)
```

---

## 🔊 Feedback (Future)

### Sounds
- **Shoot:** Pew (buzzer beep)
- **Hit:** Crack (short buzz)
- **Break:** Pop (multi-tone)
- **Death:** Explosion (descending tone)

### Screen Effects
- **Shoot:** Brief flash
- **Hit:** Asteroid flash white
- **Death:** Screen shake

---

## 🎯 Difficulty Progression

### Wave Scaling
| Wave | Large Asteroids | Speed Multiplier | Spawn Rate |
|------|----------------|------------------|------------|
| 1 | 3 | 1.0x | Slow |
| 2 | 4 | 1.1x | Normal |
| 3 | 5 | 1.2x | Normal |
| 5 | 6 | 1.3x | Fast |
| 10 | 8 | 1.5x | Fast |

---

## 🛠️ Technical Implementation

### Based on M5StickDodge4Wave
- ✅ Display system (portrait 135×240)
- ✅ JoyC input handling
- ✅ Tetris piece definitions
- ✅ Game state management
- ✅ Collision detection base

### New Systems Needed
- [ ] Player ship rotation (8 directions)
- [ ] Momentum/physics system
- [ ] Bullet management (array of 4)
- [ ] Asteroid breaking logic
- [ ] Screen wrapping
- [ ] Wave spawning system

### Memory Budget
- **Target Flash:** <500KB (38% of 1.3MB)
- **Target RAM:** <50KB (15% of 327KB)
- **Assets:** Minimal (shapes only, no bitmaps)

---

## 🚀 MVP Features (v1.0)

### Must Have
- [x] Player T-piece ship with 8-way rotation
- [ ] Thrust and momentum physics
- [ ] Shoot bullets (max 4)
- [ ] 6 types of large asteroids (I, O, L, J, S, Z)
- [ ] Asteroids break into 4 blocks
- [ ] Screen wrapping
- [ ] Collision detection
- [ ] Score system
- [ ] Wave progression
- [ ] Game over screen

### Nice to Have
- [ ] Starfield background
- [ ] Screen shake on death
- [ ] Particle effects
- [ ] Sound effects
- [ ] High score saving

---

## 🎮 Inspiration

**Classic Asteroids (1979):**
- Vector graphics → Tetris blocks
- Ship rotation → T-piece
- Thrust mechanics → Same
- Asteroids break → Into blocks
- Screen wrap → Same

**Tetris (1984):**
- 7 piece types → 6 as asteroids, 1 as player
- Block aesthetic → Visual style
- Colors → Piece colors

---

**Development Time Estimate:** 2-3 hours  
**Difficulty:** Medium (physics + collision)  
**Fun Factor:** HIGH! 🎮🚀

---

*Let's build this!*

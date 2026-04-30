# Tetrimino Space Invaders v1.2 - Visual Effects Update

**Release Date**: November 8, 2025  
**Status**: Final Release ✅

## 🎉 What's New

### Major Features

#### 💥 Particle Explosion System
- **20 active particles** can exist simultaneously
- Particles spawn on every block destruction
- **Colored debris** matches destroyed block color
- Random velocities create realistic explosions
- 300-600ms particle lifespan

#### 🎯 Individual Block Destruction
- Enemies now made of **3-4 individual blocks** instead of whole pieces
- Shoot and destroy blocks **one at a time**
- More targets = more engaging gameplay
- Scoring adjusted to **5 points per block**

#### 🎨 Random Enemy Formations
- Each block has **random offset** (-2 to +2 pixels)
- Creates **organic, varied formations**
- No two waves look identical
- Still moves as cohesive group

#### 💀 Death Animations
- **Player explosion** with magenta particles
- **2.5 second visual pause** to see explosion
- Multiple particle bursts on death
- Smooth respawn after animation

#### 🎮 Visual Polish
- **Bottom border** - Clean floor at screen bottom
- **Fixed pixel artifacts** under ship
- Dark grey floor with light separator line
- Professional, polished appearance

## 📥 Download

**Latest Release**: [`Tetriminos-v1.2-MERGED.bin`](releases/Tetriminos-v1.2-MERGED.bin)

- **Size**: 496 KB (507,568 bytes)
- **MD5**: `baaaa9d2fe8866c1b32f366d9282b905`
- **Flash Address**: 0x0

## 🎮 Gameplay Changes

### Scoring
- **5 points** per block destroyed (was 10 per group)
- Total scoring per formation remains similar
- More frequent score feedback

### Enemy Behavior
- Individual blocks can be picked off
- Formations look more organic
- Same movement and shooting mechanics
- Still spawns 9×4 grid of groups

### Death & Respawn
- **Visual explosion** when player dies
- **2.5 second delay** showing particles
- Automatic respawn with invincibility
- Lives system works correctly

## 🔧 Technical Details

### Performance
- **RAM**: 9.2% (30,164 bytes)
- **Flash**: 33.7% (441,237 bytes)
- **Frame Rate**: ~50 FPS (unchanged)
- **Particle System**: Minimal performance impact

### Code Changes
- Added `Particle` struct with physics
- Enemy structure now includes `offsetX`, `offsetY`
- Spawn formation creates individual blocks
- Collision detection updated for single blocks
- Main loop handles death animation timing

## 🆚 Version Comparison

| Feature | v1.1 | v1.2 |
|---------|------|------|
| Enemy Destruction | Whole piece | Individual blocks |
| Explosions | None | Particle system |
| Death Animation | Instant | 2.5s explosion |
| Enemy Positioning | Fixed grid | Random offsets |
| Bottom Border | No | Yes |
| Scoring per Hit | 10 pts | 5 pts |
| Visual Polish | Good | Excellent |

## 📋 Complete Feature List

### Core Gameplay
- ✅ Classic Space Invaders mechanics
- ✅ T-piece player ship
- ✅ Horizontal movement + vertical shooting
- ✅ Enemy formations (9×4 groups)
- ✅ Progressive difficulty
- ✅ Wave system

### Advanced Features (v1.1)
- ✅ Enemy shooting (starts Wave 2)
- ✅ Progressive fire rate (30% → 50% → 80%)
- ✅ Bonus life every 1000 points
- ✅ High score tracking
- ✅ Lives system (3-5 lives)
- ✅ God Mode toggle

### Visual Effects (v1.2)
- ✅ Particle explosions
- ✅ Individual block destruction
- ✅ Random formations
- ✅ Death animations
- ✅ Bottom border
- ✅ Polish & bug fixes

## 🎯 Installation

### Option 1: M5Burner (Easiest)
1. Download `Tetriminos-v1.2-MERGED.bin`
2. Open M5Burner
3. Select "M5StickC Plus2"
4. Load binary, flash to address 0x0
5. Play!

### Option 2: PlatformIO
```bash
cd Tetriminos-GitHub
pio run -e m5stick-c-plus2 -t upload
```

## 📝 Changelog Summary

**Added:**
- Particle system (20 particles)
- Individual block enemies
- Random positioning
- Death animations
- Bottom border

**Fixed:**
- Pixel artifacts
- Respawn timing
- Lives system

**Changed:**
- Scoring (5 pts/block)
- Collision detection
- Enemy spawning

## 🙏 Credits

Based on **TetroidsM5** framework (Asteroids + Tetris).

Made with ❤️ for retro gaming enthusiasts.

## 📜 License

MIT License - See LICENSE file

---

**Ready to play the most visually satisfying version yet!** 🚀🎮💥

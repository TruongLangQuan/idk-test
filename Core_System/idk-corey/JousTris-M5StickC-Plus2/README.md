# JOUSTRIS

A creative mashup of **Joust** (Atari classic) and **Tetris** - ride S and Z tetromino pieces as flying birds!

## 🎮 Gameplay

- **Player**: Green S-piece bird
- **Enemies**: Red Z-piece birds
- **Combat**: Vertical position determines winner - **Higher Y wins!** (lower on screen = higher position)
- **Eggs**: Defeated enemies drop eggs that hatch into new enemies if not collected
- **Platforms**: Land to rest or gain position advantage
- **Lava**: Deadly hazard at the bottom of the screen
- **Waves**: Endless waves with increasing difficulty

## 🕹️ Controls

- **Joystick Left/Right**: Fly horizontally
- **Joystick Button**: Flap wings (upward impulse)
- **Button A**: Toggle auto-flap mode
- **Button B**: Toggle god mode (invincibility)

## 🏆 Scoring

- Defeat enemy: **50 points**
- Collect egg: **25 points**
- Complete wave: Progress to next wave with more enemies!

## ⚔️ Combat System (Classic Joust Rules)

When you collide with an enemy:
- **You are HIGHER** (lower Y value): ✅ You win - enemy dies and drops egg
- **You are LOWER** (higher Y value): ❌ You lose - lose a life
- **Equal height**: Both bounce apart

## 🎯 Strategy

1. **Get above enemies** before collision
2. **Collect eggs quickly** before they hatch (10 seconds, shorter each wave)
3. **Use platforms** to land and control your altitude
4. **Screen wraps** horizontally - use it to escape or chase
5. **Watch the lava** - falling to the bottom costs a life!

## 📈 Difficulty Progression

- **Wave 1**: 3 enemies, slow AI
- **Wave 2+**: More enemies (up to 8), faster AI (2x max speed)
- **Egg hatch time**: Decreases each wave (10s → 5s minimum)
- **Enemy behavior**: More aggressive chase patterns

## 🎨 Visual Design

- **Player**: Green S-tetromino (4 blocks)
- **Enemies**: Red Z-tetromino (4 blocks)  
- **Eggs**: Pulsing yellow circles with hatch timer
- **Platforms**: Gray ledges for landing
- **Lava**: Animated red waves at bottom
- **Particles**: Colorful explosion effects

## 🛠️ Technical Details

- **Platform**: M5StickC Plus2 with Mini JoyC Hat
- **Display**: 135x240 portrait orientation
- **Physics**: Gravity, flap impulse, drag, platform collision
- **Frame Rate**: ~60 FPS
- **Language**: Arduino C++

## 📦 Hardware Requirements

- M5StickC Plus2
- Mini JoyC Hat (I2C joystick controller)

## 🚀 Installation

1. Open project in PlatformIO
2. Connect M5StickC Plus2 with Mini JoyC Hat
3. Build and upload: `pio run --target upload`
4. Play!

## 🎪 Game States

1. **Title Screen**: Animated demo with instructions
2. **Playing**: Main gameplay
3. **Wave Complete**: Inter-wave screen
4. **Game Over**: Final score and statistics

## 🐛 Debug Features

- **Auto-flap**: Hold button to continuously flap
- **God mode**: Invincibility for testing

---

**Enjoy the fusion of classic arcade action and tetromino aesthetics!** 🎮✨

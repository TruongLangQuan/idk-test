# M5Stick Dodge - 4 Wave Challenge

A fun dodge game for M5StickC Plus2 with 4 progressive difficulty waves!

## Game Description
Control a rotating T-piece and dodge falling obstacles across 4 unique waves. Each wave introduces new types of obstacles and increases difficulty.

## Wave System

### Wave 1 (0-30s): Tetris Pieces
Classic falling Tetris pieces (I, O, T, S, Z, L, J) with rotation.
- 7 different colored tetromino shapes
- Progressive speed increase

### Wave 2 (30-60s): TETRIS Letters
Sequential colorful letters spelling T-E-T-R-I-S repeatedly!
- Each letter has its own bright color
- Features a backwards R for retro arcade style
- T = Cyan, E = Yellow, T = Green, R = Red, I = Orange, S = Magenta

### Wave 3 (60-90s): Pattern Shapes
8 fun block-based shapes made from pixel patterns:
- Plus (+) - Yellow
- Diamond (◇) - Cyan
- Arrow (↓) - Magenta
- Heart (♥) - Red
- Zig-zag - Green
- Cross (X) - Orange
- Stairs - Cyan
- Dot cluster - White

### Wave 4+ (90s+): Mixed Chaos
Random mix of all three types:
- 33% Tetris pieces
- 33% TETRIS letters (sequential)
- 33% Pattern shapes
- Spawn rate continues to increase!

## Controls
- **Joystick**: Move player in all directions (portrait mode)
  - Up: Move right
  - Down: Move left
  - Left: Move up
  - Right: Move down
- **Joystick Button**: Rotate player piece

## Scoring
- +10 points for each obstacle that passes
- Level up every 100 points
- Speed increases with level and wave progression

## Hardware Required
- M5StickC Plus2
- Mini JoyC Hat (joystick controller)

## Installation
1. Open project in PlatformIO
2. Connect M5StickC Plus2 via USB
3. Build and upload: `pio run --target upload`
4. Enjoy!

## Technical Details
- Platform: ESP32 (M5StickC Plus2)
- Framework: Arduino
- Display: 135x240 portrait mode
- Libraries: M5StickCPlus2 v1.0.2

## Game Features
- Smooth 60 FPS gameplay
- Precise pixel-perfect collision detection
- Progressive difficulty system
- Clean retro pixel art aesthetic
- Unlimited waves with increasing challenge

---

Built with ❤️ for M5StickC Plus2

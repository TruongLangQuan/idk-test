# ThreeTris Project Analysis - For Building Quintris

## 🎮 Game Overview
ThreeTris is a Tetris variant using 3-block pieces (Trominoes) instead of 4-block Tetrominoes. Successfully built and working on M5StickC Plus2 with Mini JoyC Hat controller.

## 🕹️ Hardware Setup
- **M5StickC Plus2** (ESP32-based handheld)
- **Mini JoyC Hat** - I2C joystick controller (pins 0 & 26)
- **Display**: 240x135 LCD in portrait mode (rotation 0)

## 🎯 Key Game Mechanics

### Rotation System - **CRITICAL FOR QUINTRIS**
- **ALWAYS CLOCKWISE ROTATION** - This is emphasized multiple times in notes
- Code implementation: `int newRot = (currentRot + 1) % 4;` (NOT +3!)
- All pieces rotate around center pivot (0,0)
- Wall kicks implemented for edge cases
- Lock delay system prevents accidental placements

### Piece Structure
```cpp
// Format: [piece][rotation][axis][block_index]
// axis 0 = Y coordinates, axis 1 = X coordinates  
// Pivot always at (0,0)
int pieces[8][4][2][3];  // 8 pieces, 4 rotations, Y/X coords, 3 blocks max
```

### 8 Pieces in ThreeTris:
1. **I-Triomino** - 3-block line (horizontal/vertical)
2. **L-Triomino** - Corner shape (4 rotations)
3. **J-Triomino** - Reverse L (4 rotations, mirror of L)
4. **Single Block** - Just 1 block (no rotation needed)
5. **Double Block** - 2 blocks in line (horizontal/vertical)
6. **Diagonal Double** - 2 blocks diagonal (2 rotations: / and \)
7. **Diagonal Triple** - 3 blocks diagonal (2 rotations: / and \)
8. **T-minus** - T-shape with 3 blocks instead of 4

## 🎮 Controls (Mini JoyC Hat)
- **Joystick Left/Right**: Move piece horizontally
- **Joystick Down**: Soft drop (faster fall)
- **Joystick Up**: Hard drop (instant placement)
- **Joystick Button**: Rotate clockwise
- **Button A**: Hold/swap piece
- **Button B**: Reset game

### Input Mapping (Important!)
```cpp
// Axes are swapped for M5Stick orientation:
buttons.down = (joyX < 1200);   // Left on joystick = DOWN in game
buttons.up = (joyX > 3000);     // Right on joystick = UP in game  
buttons.right = (joyY < 1600);  // Down on joystick = RIGHT in game
buttons.left = (joyY > 3000);   // Up on joystick = LEFT in game
```

## 📐 Game Board Setup
- **Field Size**: 10x23 (width x height)
- **Block Size**: 10 pixels
- **Screen Offset**: X=5, Y=10 pixels
- **Border**: White rectangle around playing field
- **Portrait Mode**: Rotation 0 for optimal gameplay

## 🎨 Display Features
- **Ghost piece preview** - Shows where piece will land
- **Hold piece display** - Shows held piece in mini format
- **Next piece preview** - Shows upcoming piece
- **Score and level display**
- **Progressive difficulty** - Speeds up every 10 lines cleared

## 🔧 Technical Implementation

### File Structure:
```
src/
├── main.cpp          - Entry point (just runs Tetris)
├── tetris.cpp/.h     - Main game logic
├── input.cpp/.h      - Mini JoyC Hat input handling  
├── display.cpp/.h    - Display functions
├── config.h          - Hardware configuration
├── UNIT_MiniJoyC.cpp/.h - Joystick driver
```

### PlatformIO Configuration:
```ini
[env:m5stick-c-plus2]
platform = espressif32
board = m5stick-c
framework = arduino
lib_deps = m5stack/M5StickCPlus2@^1.0.2
```

### Memory Usage (Working Build):
- RAM: 8.1% (26,700 bytes)
- Flash: 33.3% (436,597 bytes)

## 🚨 Critical Rotation Notes for Quintris

### Clockwise Rotation Pattern:
```
R0 → R90 → R180 → R270 → R0...
```

### Example L-Piece Rotation (CLOCKWISE):
```
R0:    R90:   R180:   R270:
##     #      #       ###
#      #      ##        #
       ##
```

### Coordinate System:
- Pivot at (0,0) - center of rotation
- Y negative = up, Y positive = down
- X negative = left, X positive = right

### Wall Kick System - **COMPLETE AND WORKING!**:
✅ **Full 6-step wall kick algorithm implemented and tested**
- The game implements comprehensive wall kicks when rotation fails at boundaries
- **Prevents pieces from getting "stuck" against walls on BOTH sides**
- **Works perfectly with spins** - pieces can rotate in tight spaces
- 6-step kick sequence:
  1. Direct rotation
  2. Kick right (+1)
  3. Kick left (-1) 
  4. Kick up (-1 Y)
  5. Kick right 2 (+2) - for left wall
  6. Kick left 2 (-2) - for right wall
- **This was a major breakthrough** - rotation problems are completely solved!

## 🎯 For Quintris Development

### Key Differences for 5-Block Pieces:
1. **Change array size**: `[3]` → `[5]` blocks per piece
2. **Update loops**: `for (int i = 0; i < 3; i++)` → `for (int i = 0; i < 5; i++)`
3. **Design new 5-block pieces** with proper clockwise rotation
4. **Test rotation carefully** - 5-block pieces will have more complex collision cases
5. **May need larger playing field** - 5-block pieces might need more space

### Proven Working Elements to Reuse:
- ✅ Hardware setup (M5StickC Plus2 + Mini JoyC Hat)
- ✅ Input handling and mapping
- ✅ Display system and game board rendering
- ✅ **Clockwise rotation system (PERFECT)**
- ✅ **Complete wall kick system (6-step algorithm works flawlessly!)**
- ✅ Ghost piece and hold functionality
- ✅ Lock delay system
- ✅ PlatformIO build configuration

**CRITICAL SUCCESS:** ThreeTris was built from scratch and is essentially flawless - rotation problems completely solved with wall kicks working on both walls and spins. This gives us a rock-solid foundation for Quintris!

### Available Working Binaries:
- Multiple tested .bin files in releases/ folder
- Latest: `ThreeTris-v3.3-FINAL.bin` and `ThreeTris-v3.3-MERGED.bin`

## 🔍 Next Steps for Quintris:
1. **Copy working ThreeTris base** as starting point
2. **Design 5-block piece set** (research Pentominoes)
3. **Update piece arrays** from 3 blocks to 5 blocks
4. **Test each piece rotation** methodically
5. **Adjust game balance** (may need to slow down due to complexity)

The ThreeTris project provides an excellent foundation with all the hardware integration, input handling, and core Tetris mechanics already working and tested.
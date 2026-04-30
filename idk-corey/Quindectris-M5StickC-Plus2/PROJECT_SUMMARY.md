# ThreeTris - Project Summary

## What Was Done

Successfully extracted the Tetris game from M5StickDodge4Wave and created ThreeTris - a 3-block variant!

## Key Changes from Original Tetris

### 1. **Piece Structure (3 blocks instead of 4)**
   - Changed array size from `[4]` to `[3]` blocks per piece
   - All loops changed from `for (int i = 0; i < 4; i++)` to `for (int i = 0; i < 3; i++)`

### 2. **New Piece Definitions (7 Trominoes)**
   - I-piece: 3-block straight line
   - L-piece: Corner shape
   - J-piece: Reverse L
   - T-piece: T shape (3 blocks)
   - V-piece: Bent corner
   - Z-piece: Zigzag
   - S-piece: Reverse zigzag

### 3. **Files Extracted**
   - `tetris.cpp` / `tetris.h` - Main game logic (modified for 3 blocks)
   - `input.cpp` / `input.h` - Mini JoyC Hat input handling
   - `display.cpp` / `display.h` - Display functions
   - `config.h` - Hardware configuration (cleaned up)
   - `UNIT_MiniJoyC.cpp` / `UNIT_MiniJoyC.h` - Joystick driver
   - `main.cpp` - Simple entry point (NEW - just runs Tetris)

### 4. **Removed from Original**
   - All Dodge game files
   - Game menu system
   - Multi-game manager

## Project Status

✅ **Compiles successfully!**
- RAM Usage: 8.1% (26,700 bytes)
- Flash Usage: 33.3% (436,597 bytes)

## Hardware Requirements

- M5StickC Plus2
- Mini JoyC Hat (I2C joystick on pins 0 & 26)

## Controls

- **Joystick**: Move pieces left/right, soft/hard drop
- **Joystick Button**: Rotate
- **Button A**: Hold piece
- **Button B**: Reset game

## Ready to Upload!

Run: `pio run -t upload` when the M5StickC Plus2 is connected.

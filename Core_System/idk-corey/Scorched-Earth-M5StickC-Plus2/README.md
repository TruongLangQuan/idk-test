# ScorchedM5 - Tetris Artillery Game

A turn-based artillery game for the M5StickC Plus2, featuring destructible terrain and tetrimino-styled graphics.

## Overview

ScorchedM5 is a Scorched Earth-style artillery game where you face off against an AI opponent. Both players control upside-down T-shaped tetrimino tanks positioned on randomly generated destructible terrain. Take turns adjusting your angle and power to fire projectiles across the map and destroy your opponent.

## Features

- **Destructible Terrain**: Randomly generated landscapes with smooth terrain profiles
- **Turn-based Combat**: Adjust angle and power, then fire at your opponent
- **Physics-based Projectiles**: Realistic arc trajectory with gravity
- **AI Opponent**: Computer-controlled enemy with randomized but strategic firing
- **Animated Sky**: Scrolling clouds and tetrimino birds add atmosphere
- **Win/Loss Tracking**: Keep track of your victories and defeats
- **Tetris-themed Graphics**: All game elements use the classic tetrimino block style

## Controls

### M5StickC Plus2 Buttons:
- **Left Button (PWR)**: Decrease angle (move aim left)
- **Right Button (BtnB)**: Increase angle (move aim right)  
- **Middle Button (BtnA)**: Cycle power (1-100%, hold to reverse direction)
- **Hold Middle Button (BtnA)**: Fire projectile

### Gameplay:
1. Adjust your firing angle using Left/Right buttons (in 2.5% increments)
2. Set your power level using Middle button (in 2.5% increments)
3. Hold Middle button to fire
4. Watch the projectile arc across the screen
5. AI takes its turn automatically
6. First to destroy opponent's tank wins!

## Hardware Requirements

- **M5StickC Plus2**
- No additional hardware required

## Software Requirements

- PlatformIO IDE or PlatformIO CLI
- ESP32 toolchain (installed automatically by PlatformIO)

## Installation

### Method 1: Using PlatformIO

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/ScorchedM5.git
   cd ScorchedM5
   ```

2. Open in PlatformIO or build from command line:
   ```bash
   pio run -e m5stick-c-plus2 -t upload
   ```

### Method 2: Using M5Burner

1. Download the latest `ScorchedM5-v1.0-MERGED.bin` from the releases page
2. Open M5Burner
3. Select your M5StickC Plus2
4. Load the .bin file and burn to device

## Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/ScorchedM5.git
cd ScorchedM5

# Build the project
pio run -e m5stick-c-plus2

# Upload to device
pio run -e m5stick-c-plus2 -t upload

# Or build and upload in one command
pio run -e m5stick-c-plus2 -t upload
```

The compiled firmware will be located at:
`.pio/build/m5stick-c-plus2/firmware.bin`

## Project Structure

```
ScorchedM5/
├── src/
│   └── main.cpp          # Main game code
├── include/              # Header files (if any)
├── platformio.ini        # PlatformIO configuration
└── README.md            # This file
```

## Game Mechanics

### Terrain Generation
- Randomly generated smooth terrain using sine wave patterns
- Different heights and profiles each game
- Fully destructible - blocks removed by explosions

### Tank Placement
- Tanks spawn on opposite sides of the terrain
- Always on solid ground
- Random horizontal positions each game

### Projectile Physics
- Gravity affects trajectory
- Angle range: adjustable via buttons
- Power range: 1-100%
- Explosion radius removes terrain blocks

### AI Behavior
- Calculates approximate trajectory to player
- Adds randomization to power and angle
- Becomes more accurate as game progresses
- Avoids shooting directly off-screen

### Win Conditions
- Destroy opponent's tank with direct hit
- Track wins and losses across multiple rounds
- Terrain collapses are visual only (tanks don't fall)

## Credits

Part of the Tetrimino game series for M5StickC Plus2:
- **Tetriminos** - Space Invaders variant
- **TetriSnake** - Snake with tetrimino obstacles
- **ScorchedM5** - Artillery combat (this project)
- **TetroidsM5** - Asteroids meets Tetris

## License

This project is open source. Feel free to modify and distribute.

## Troubleshooting

**Display issues:** Ensure you have the latest M5StickCPlus2 library installed

**Upload fails:** Try pressing the reset button during upload or check USB connection

**Game freezes:** Reset the device using the power button

## Future Enhancements

- Multiplayer via WiFi
- Power-ups and special weapons
- Different terrain types
- Wind effects on projectiles
- More AI difficulty levels

## Version History

- **v1.0** - Initial release with full gameplay features
- **v0.4** - Added win/loss counter
- **v0.3** - Improved AI and controls
- **v0.2** - Added animated sky elements
- **v0.1** - Basic gameplay prototype

---

Enjoy playing ScorchedM5! 🎮💥

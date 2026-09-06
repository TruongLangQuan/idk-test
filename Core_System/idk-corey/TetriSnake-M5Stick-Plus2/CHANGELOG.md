# Changelog

All notable changes to TetriSnake will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2025-11-08

### Added
- **Differential rendering system** - Only redraws changed cells
  - Eliminates screen flicker completely
  - Tracks previous grid state for comparison
  - Much smoother 50 FPS gameplay
- **Improved food spawn logic**
  - Increased random attempts from 200 to 500
  - Added fallback grid scan if random fails
  - Game over trigger if grid is completely full
  - Guarantees food always spawns or game ends gracefully

### Changed
- **Food color changed to white** (#FFFF)
  - Was red (#F800) which conflicted with Z-piece color
  - Now highly visible against all tetrimino colors
  - Much easier to locate during gameplay

### Fixed
- **Screen flicker eliminated** - Removed full screen clear each frame
- **Food visibility** - No longer blends with obstacles

### Technical
- RAM: 8.8% (28,892 bytes)
- Flash: 33.2% (434,889 bytes)
- Grid comparison array added (770 bytes)

## [1.1.0] - 2025-11-08

### Added
- **Colored tetriminos** - Each obstacle type now displays in proper Tetris color
  - I-piece: Cyan
  - O-piece: Yellow
  - T-piece: Magenta
  - L-piece: Orange
  - J-piece: Blue
  - S-piece: Green
  - Z-piece: Red
- **Blue border** around entire play area for visual clarity
- **Block borders** - Dark outline on each block for definition
- **Extended play area** - Bottom extended by 2 blocks

### Changed
- Grid system now stores tetrimino type (1-7) instead of just "obstacle" flag
- **PLAY_AREA_TOP**: 20 → 22 pixels
- **PLAY_AREA_BOTTOM**: 220 → 234 pixels (14px more room)
- **GRID_ROWS**: 33 → 35 (2 additional rows)

### Fixed
- Snake can now reach bottom of screen without cutoff
- Visual clarity improved with colored obstacles

## [1.0.0] - 2025-11-08

### Added
- Initial release of TetriSnake
- **Core Snake gameplay**
  - Classic snake movement with growth
  - Food collection mechanics
  - Self-collision detection
  - Wall collision detection
- **Tetrimino obstacles system**
  - 7 tetrimino types (I, O, T, L, J, S, Z)
  - Random placement algorithm
  - One of each type per game
  - Collision detection with obstacles
- **Progressive difficulty**
  - Starting speed: 200ms per move
  - Speed decrease: 5ms per food
  - Minimum speed: 80ms (very challenging)
- **Scoring system**
  - 10 points per food collected
  - High score tracking (session-based)
  - Score and length display in HUD
- **God Mode**
  - Toggle with BtnB
  - Pass through obstacles
  - Wrap around edges
  - Yellow indicator bar
- **Joystick controls** via Mini JoyC Hat
  - 4-way directional movement
  - 180-degree turn prevention
  - Portrait mode orientation (up/down swapped for natural feel)
- **Visual features**
  - 22×35 block grid (770 cells)
  - 6×6 pixel blocks
  - HUD with score, length, high score
  - Title screen
  - Game over screen with statistics
- **Complete M5StickC Plus2 support**
  - Portrait mode (135×240)
  - Button input
  - I2C joystick integration

### Technical
- Built with Arduino framework + PlatformIO
- ESP32-PICO-V3-02 chip support
- RAM usage: 8.6% (28,076 bytes)
- Flash usage: 33.2% (434,657 bytes)
- Frame rate: ~50 FPS (20ms delay)
- Block-based rendering system
- Grid-based collision detection

---

## Future Considerations

### Potential Features
- [ ] Sound effects (food collect, collision, game over)
- [ ] Particle effects on food collection
- [ ] EEPROM-based persistent high score
- [ ] Multiple difficulty modes (Easy/Normal/Hard)
- [ ] Different tetrimino placement patterns
- [ ] Power-ups (speed boost, invincibility, shrink)
- [ ] Animated food (pulsing/glowing)
- [ ] Different snake color themes
- [ ] Two-player mode (turn-based high score competition)
- [ ] Timed challenges
- [ ] Achievement system
- [ ] Food combo multipliers

### Technical Improvements
- [ ] Optimize collision detection
- [ ] Implement double buffering
- [ ] Add configuration menu
- [ ] WiFi leaderboard support
- [ ] Support for other M5Stack devices
- [ ] Custom tetrimino patterns editor

---

## Version Naming Convention

- **Major version** (X.0.0): Significant gameplay changes or complete rewrites
- **Minor version** (1.X.0): New features, major improvements
- **Patch version** (1.0.X): Bug fixes, minor tweaks, optimizations

## Release Files

Each release includes:
- `TetriSnake-vX.X-MERGED.bin` - Complete merged binary for M5Burner (flash to 0x0)
- `TetriSnake-vX.X-APP.bin` - Application binary only
- `bootloader.bin` - ESP32 bootloader
- `partitions.bin` - Partition table
- `TetriSnake-vX.X.json` - M5Burner metadata

## Links

- [Installation Guide](releases/)
- [Source Code](src/)
- [Issue Tracker](../../issues/)

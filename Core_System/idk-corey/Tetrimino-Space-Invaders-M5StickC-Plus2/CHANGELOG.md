# Changelog

All notable changes to Tetrimino Space Invaders will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2025-11-08

### Added
- **Particle explosion system** - 20 active particles at once
  - Colorful particles spawn when blocks are destroyed
  - Random velocities and angles for realistic explosions
  - 300-600ms lifespan with gradual fadeout
- **Player death animation** - Magenta particles explode outward when hit
  - 2.5 second visual delay before respawn
  - Multiple particle bursts from player position
  - Game continues to render during explosion
- **Individual block destruction** - Enemies are now composed of individual blocks
  - 3-4 blocks per enemy group (clustered tetrimino pattern)
  - Each block can be destroyed separately
  - More targets, more engaging gameplay
- **Random enemy positioning** - Each block has slight offset (-2 to +2 pixels)
  - Creates organic, varied formations
  - No two waves look exactly the same
  - Still maintains cohesive group movement
- **Bottom border visual** - Clean floor at bottom of screen
  - Dark grey fill below play area
  - Light grey separator line
  - Prevents pixel artifacts

### Fixed
- **Pixel artifacts** under player ship eliminated
- **Lives system respawn** - Player now respawns automatically after explosion
- **Death handling** - Removed blocking delays, game loop shows animation properly

### Changed
- **Scoring adjusted** - Now 5 points per block (instead of 10 per group)
  - Total scoring remains similar per formation
  - More satisfying feedback per shot
- **Respawn timing** - Increased to 2.5 seconds to show full explosion
- **Collision detection** - Updated for individual block positioning

## [1.1.0] - 2025-11-08

### Added
- **Enemy shooting system** - Enemies fire red bullets starting from Wave 2
  - Progressive fire rate: 30% (Wave 2) → 50% (Wave 3-4) → 80% (Wave 5+)
  - Only frontmost enemies in each column can shoot
  - 2-second minimum interval between shots
- **Bonus life system** - Earn an extra life every 1,000 points
  - Lives capped at maximum of 5
  - "BONUS LIFE!" notification displays for 2 seconds in green
  - Tracks last bonus threshold to prevent duplicates
- **High score tracking** - Persistent high score display
  - Shows "NEW HIGH!" message when beating record
  - Displays previous high score on game over screen
- **Visual indicators**
  - Red "!" icon when enemy shooting is active (Wave 2+)
  - Green bonus life notification overlay
  - High score badge on game over
- **Improved HUD layout**
  - Wave number on top-left
  - Score centered at top
  - Lives as red squares on top-right (reversed order for better visibility)

### Fixed
- **Lives system** now properly decrements when hit
  - Enemy bullet collisions correctly reduce lives
  - Enemy contact properly reduces lives
  - Game over only triggers when lives reach 0
  - Respawn invincibility works correctly
- Lives counter display bug fixed

### Changed
- Enemy bullets are red colored (distinct from white player bullets)
- HUD spacing optimized for better readability
- Lives display changed from left-to-right to right-to-left alignment
- Wave progression now accounts for shooting difficulty

## [1.0.0] - 2025-11-08

### Added
- Initial release of Tetrimino Space Invaders
- **Core Gameplay**
  - T-piece player ship at bottom of screen
  - Horizontal movement with joystick
  - Vertical shooting mechanic
  - Enemy formation (9 columns × 4 rows)
- **Enemy Formation System**
  - Synchronized group movement
  - Horizontal sliding with edge detection
  - Drop-down and direction reversal at boundaries
  - Six Tetrimino enemy types (I, O, L, J, S, Z pieces)
- **Wave System**
  - Progressive difficulty
  - Speed increases each wave (800ms → 200ms minimum)
  - Wave completion bonus (100 points)
  - Spawn new formation after clearing all enemies
- **Lives System**
  - Start with 3 lives
  - Respawn with 2 seconds invincibility
  - Game over when all lives lost
- **Scoring System**
  - 10 points per enemy destroyed
  - 100 points per wave completed
- **Controls**
  - Mini JoyC Hat (I2C) for movement and shooting
  - BtnB for God Mode toggle
  - BtnA for game restart
- **Visual Features**
  - Tetris-themed colored blocks (6×6 pixels)
  - HUD with wave, score, and lives display
  - Game over screen with statistics
  - Title screen
- **God Mode** - Invincibility toggle for testing
- **Mini JoyC Hat integration** via I2C (address 0x54)
- **M5StickC Plus2 support** with 135×240 display
- Complete documentation and installation guides

### Technical
- Built with Arduino framework + PlatformIO
- ESP32-PICO-V3-02 chip support
- RAM usage: 8.1% (26,644 bytes)
- Flash usage: 33.3% (436,433 bytes)
- Frame rate: ~50 FPS (20ms delay)
- Block-based rendering system
- Grid-based collision detection

---

## Future Considerations

### Potential Features
- [ ] EEPROM-based persistent high score storage
- [ ] Sound effects (shoot, hit, explosion, wave clear)
- [ ] Background music
- [ ] Shield barriers (like classic Space Invaders)
- [ ] Power-ups (rapid fire, shield, slow-mo)
- [ ] Different enemy formation patterns
- [ ] Boss waves every 5-10 levels
- [ ] Animated explosions with particle effects
- [ ] Enemy drop items (bonus points)
- [ ] Multiple difficulty modes
- [ ] Two-player co-op mode
- [ ] Combo multipliers for consecutive hits
- [ ] Achievements system

### Technical Improvements
- [ ] Optimize collision detection algorithm
- [ ] Implement sprite-based rendering
- [ ] Add configuration menu
- [ ] Support for other M5Stack devices
- [ ] WiFi leaderboard support
- [ ] Battery level indicator
- [ ] Brightness control

---

## Version Naming Convention

- **Major version** (X.0.0): Significant gameplay changes or complete rewrites
- **Minor version** (1.X.0): New features, major improvements
- **Patch version** (1.0.X): Bug fixes, minor tweaks, optimizations

## Release Files

Each release includes:
- `Tetriminos-vX.X-MERGED.bin` - Complete merged binary for M5Burner
- `Tetriminos-vX.X-APP.bin` - Application binary only
- `bootloader.bin` - ESP32 bootloader
- `partitions.bin` - Partition table
- `Tetriminos-vX.X.json` - M5Burner metadata
- `checksums.md5` - File verification checksums

## Links

- [Installation Guide](releases/INSTALLATION.md)
- [Release Binaries](releases/)
- [Source Code](src/)
- [Issue Tracker](../../issues/)

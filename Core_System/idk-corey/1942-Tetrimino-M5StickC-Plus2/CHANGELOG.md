# Changelog

All notable changes to 1942 Tetrimino Edition will be documented in this file.

## [2.3] - 2025-11-10 - FINAL RELEASE

### Changed
- Unified difficulty progression across all waves
- Removed special case for Wave 6 - now follows same scaling formula
- Smooth difficulty curve from Wave 1 through endless mode
- All difficulty metrics properly capped with documented formulas

### Documentation
- Added DIFFICULTY_PROGRESSION.md with complete scaling charts
- Created comprehensive release notes
- Added gameplay tips and strategies

### Fixed
- Wave 6 now scales naturally with other waves
- Consistent speed multiplier formula throughout

## [2.2] - 2025-11-10

### Added
- Progressive boss scaling system
- Bosses grow from 2x to 8x size as waves progress
- Boss size scales: 2.0 + (wave - 1) × 0.5, capped at 8.0x
- Visual and hitbox scaling for huge bosses
- Bosses become screen-filling by Wave 13+

### Changed
- Boss hitboxes now scale proportionally with size
- Maximum boss size capped at 8x to prevent off-screen issues

## [2.1] - 2025-11-10

### Added
- Animated title screen with multiple layers
- Scrolling starfield background
- Flying Tetrimino formations (I-pieces and Z-pieces)
- Player plane oscillates and shoots randomly every 2-4 seconds
- Pulsing "PRESS ANY BUTTON" text
- Control hints displayed on title screen
- Game state system (TITLE_SCREEN, PLAYING, GAME_OVER)

### Changed
- Game now starts at title screen instead of immediately playing
- Game Over returns to title screen instead of restarting directly

## [2.0] - 2025-11-10 - ENDLESS MODE

### Added
- Endless wave system with progressive difficulty scaling
- Wave 6: TETRIS letters (T-E-T-R-I-S) spawn in sequence
- Wave 6: 8 strange symbols (Plus, Diamond, Arrow, Heart, Zig, Cross, Stairs, Spiral)
- Wave 7+: Mix of Tetriminos, letters, and symbols (33% each)
- Difficulty caps for safety (speed, spawn rate, fire rates)
- Boss health scales infinitely (+5 HP per wave)
- End-game statistics tracking (shots, kills, accuracy)
- Statistics displayed on Game Over screen
- Auto-fire toggle system (Button A)
- "AUTO" indicator on HUD when enabled

### Changed
- Boss battles every 60 seconds (was per-wave)
- Spawn rate progressively faster with 600ms cap (Wave 7+)
- Enemy fire rate scales: 1000ms → 400ms (Wave 13+ cap)
- Boss fire rate scales: 500ms → 200ms (Wave 4+ cap - bullet hell!)
- Enemy speed scales: 1.0x → 3.0x (Wave 14+ cap)

### Performance
- All difficulty metrics have safety caps
- Maximum difficulty reached at Wave 14-15
- Boss health continues scaling after caps hit

## [1.6] - 2025-11-10

### Added
- TETRIS letter enemies (T, E, T, R, I, S)
- 8 unique strange symbol enemies
- Letters spell out TETRIS in sequence
- Colorful 5x5 pixel art for letters and symbols
- Wave 6 features letters and symbols

## [1.5] - 2025-11-10

### Added
- Five complete waves with progressive difficulty
- Each wave increases spawn rate by 20%
- Each wave increases enemy count by 20%
- Boss health increases by 25% per wave
- Large enemies (1.5x size) with 30% spawn chance in waves 2-5

### Changed
- Wave structure now supports multiple waves before boss
- Boss spawns at end of each wave
- Difficulty scales smoothly across waves

## [1.4] - 2025-11-10

### Added
- Wave progression system
- Boss battles with health bars
- Boss appears every 60 seconds
- Boss shoots more frequently than regular enemies
- Boss moves in patterns across screen
- Large enemy variants (1.5x size, worth 15 points)
- Wave indicator on HUD

### Changed
- Enemies now spawn in waves with varied patterns
- Scoring adjusted for different enemy types

## [1.3] - 2025-11-10

### Fixed
- Wing gun positioning corrected
- Right wing guns now properly offset to the right
- All guns fire in sync
- Visual alignment improved

## [1.2] - 2025-11-09

### Added
- Power-up system with "P" pickups
- Stackable wing guns (up to 3 levels, 6 total wing guns)
- Power-ups act as shields before losing lives
- Wing bullets (30 max) fire alongside main gun
- Random power-up spawning every 20-45 seconds
- Power level indicator on HUD

### Changed
- Lives system now interacts with power-up shields
- Lose power-up before losing life when hit
- Increased survivability with shield mechanic

## [1.1] - 2025-11-09

### Fixed
- Controls corrected for portrait orientation
- Forward/backward movement swapped (joystick up now moves player up)
- Left/right movement working correctly

### Added
- Score display on Game Over screen
- Game Over shows final score before restart

## [1.0] - 2025-11-09 - INITIAL RELEASE

### Added
- Core 1942-style vertical scrolling shooter gameplay
- Player T-piece fighter with 8-direction movement
- 6 Tetrimino enemy types (I, O, L, J, S, Z)
- 3 enemy formation patterns (straight, arc, swoop)
- Bullet system (20 player bullets, 30 enemy bullets)
- Collision detection
- Particle effects for impacts
- Explosion animations
- Lives system with invincibility frames
- Score tracking
- God mode toggle (Button B)
- 60 FPS gameplay
- HUD with score, lives, wave timer
- JoyC Hat joystick controls
- Game Over and restart functionality

### Technical
- 1200+ lines of C++
- Single-file architecture
- Procedurally generated sprites
- M5GFX graphics
- Custom JoyC Hat driver
- RAM usage: 8.7%
- Flash usage: 34.5%

---

## Version Naming

- **Major version** (X.0): Significant feature additions or gameplay changes
- **Minor version** (X.Y): Feature additions, improvements
- **Patch version** (X.Y.Z): Bug fixes, minor tweaks

## Links

- [Repository](https://github.com/yourusername/1942-tetrimino)
- [Releases](https://github.com/yourusername/1942-tetrimino/releases)
- [Issues](https://github.com/yourusername/1942-tetrimino/issues)

# Changelog

All notable changes to M5StickSonic will be documented in this file.

## [1.2.0] - 2024-11-06 - FINAL RELEASE

### Added
- **DOWN-TO-ROLL mechanic** - The biggest feature!
  - Press DOWN on joystick while moving to roll into ball
  - Release DOWN to stand back up
  - Maintains momentum during roll/unroll
  - Just like the original Sonic game!
- Added `isKeyPressedDown()` function to Keyboard class
- Enhanced Player.cpp with roll/unroll state management

### Changed
- Game now feels more authentic to original Sonic
- Perfect control over rolling state
- Two ways to roll: DOWN button (toggle) or BtnA+Joy (hold)

### Technical
- X-axis joystick (ADC 0) used for DOWN detection
- Triggers ACT_ROLL when DOWN pressed and gsp >= 0.5
- Returns to ACT_NORMAL when DOWN released
- Y position adjusted on roll/unroll for proper hitbox

## [1.1.0] - 2024-11-06

### Added
- Larger screen size: 211x119 pixels (was 160x90)
- Optimized rendering with pushPixels API

### Changed
- Fixed joystick axis mapping (Y-axis for left/right movement)
- Removed FPS/Ring counter overlay for clean gameplay
- Improved centering calculation for larger display

### Fixed
- Joystick directions now intuitive (left=left, right=right)
- Deprecated pushColors replaced with pushPixels

### Technical
- Display centered at (120, 67) instead of (160, 120)
- Removed unnecessary StickCP2.update() call from game loop

## [1.0.0] - 2024-11-06 - Initial Release

### Added
- Complete Green Hill Zone Act 1 port to M5StickC Plus2
- Custom UNIT_JOYC driver for Mini Joy C Hat
- Palette-based renderer optimized for ESP32
- Authentic Sonic physics from Physics Guide
- All classic moves: run, jump, roll, spindash
- Enemy AI: Motobug, Crabmeat, Chopper
- Ring collection system
- Terrain system: loops, slopes, springs
- Animated backgrounds
- 160x90 resolution at 27-30 FPS

### Technical Implementation
- PlatformIO project structure
- M5StickCPlus2 library integration
- I2C joystick polling at 0x54
- Custom 20-line partial buffer rendering
- RGB565 color encoding
- 66.7% Flash, 20.1% RAM usage

### Credits
- Original M5Stack port by GalaxyShad
- Adapted for M5StickC Plus2
- Mini Joy C Hat integration
- Display scaling and optimization

---

## Version Comparison

| Feature | v1.0 | v1.1 | v1.2 |
|---------|------|------|------|
| Resolution | 160x90 | 211x119 | 211x119 |
| FPS Overlay | Yes | No | No |
| Screen Size | 71% | 88% | 88% |
| Joystick Axes | Wrong | Fixed | Fixed |
| Down-to-Roll | ❌ | ❌ | ✅ |
| Roll Control | BtnA+Joy | BtnA+Joy | DOWN or BtnA+Joy |

---

## Migration Guide

### From v1.1 to v1.2
No breaking changes! Just flash the new firmware.
- New DOWN button functionality
- All existing controls still work
- BtnA+Joy Button still works for rolling

### From v1.0 to v1.1
No breaking changes! Just flash the new firmware.
- Larger display automatically centered
- FPS overlay removed (cleaner)
- Joystick directions fixed

---

## Future Possibilities

Potential enhancements (not guaranteed):
- Additional levels (memory permitting)
- More enemy types
- Simple sound effects (beeps)
- Save state system
- Custom color palettes
- Performance modes

---

**Current Version: 1.2.0 FINAL**

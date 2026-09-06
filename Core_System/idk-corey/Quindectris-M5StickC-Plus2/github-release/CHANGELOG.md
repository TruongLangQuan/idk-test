# Changelog

All notable changes to ThreeTris will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [3.0] - 2024-11-09

### Added
- **Professional animated title screen**
  - "THREETRIS" in vertical colorful block letters
  - 3 cascading animated triomino pieces
  - Dark blue background with cyan border
  - Version badge display
  - Blinking start prompt
- All 8 triomino pieces properly represented in animations
- Proper game state management (TITLE → PLAYING → GAME_OVER)

### Changed
- Updated version display to v3.0
- Improved piece animation positioning
- Better cascade layout for animated pieces
- Refined title screen aesthetics

### Fixed
- Pieces no longer overlap letters on title screen
- All animated pieces are now actual triominoes (no 4-block pieces)
- Proper spacing and positioning throughout

## [2.5] - 2024-11-09

### Added
- **Score tracking system**
  - Current score display
  - High score tracking
  - Level display
  - Lines cleared counter
- **Next piece preview box** (right side of screen)
- Progressive difficulty system
- Game over screen improvements

### Changed
- Extended playfield to 23 rows
- Improved HUD layout with stats
- Better screen space utilization

### Fixed
- Bottom border extended properly
- Screen alignment perfected
- Pixel artifact cleanup on game over

## [2.0] - 2024-11-09

### Added
- **8 unique triomino pieces**:
  1. I-Triomino (3-block line)
  2. L-Triomino (corner right)
  3. J-Triomino (corner left)
  4. Single Block
  5. Double Block
  6. Diagonal Double
  7. Diagonal Triple
  8. T-minus-one (checkerboard)
- Perfect clockwise rotation for all pieces
- Hold piece system with preview box
- Ghost piece indicator
- Progressive lock delay system

### Changed
- All pieces now rotate consistently clockwise
- Rotation centered on middle block
- Improved piece spawn logic

### Fixed
- **MAJOR**: Fixed all rotation issues
  - No more morphing pieces
  - No more flipping pieces
  - No more skipped rotations
  - Stable pivot points for all pieces

## [1.0] - 2024-11-09

### Added
- Initial release
- Basic ThreeTris gameplay
- 3 triomino pieces (I, L, J)
- M5StickC Plus2 support
- Mini JoyC Hat controls
- Basic scoring
- Line clearing
- Game over detection

### Known Issues (Fixed in later versions)
- Pieces rotating incorrectly
- Some pieces morphing shapes
- Limited piece variety

---

## Version History Summary

- **v3.0**: Professional title screen, perfect animations
- **v2.5**: Complete HUD, score tracking, next piece preview
- **v2.0**: All 8 pieces, perfect rotation system
- **v1.0**: Initial playable version

---

*For more details, see commit history on GitHub.*

# Changelog

All notable changes to Columns M5 will be documented in this file.

## [1.0.0] - 2024-11-07

### Added
- Initial release of Columns M5
- Classic Columns match-3 gameplay mechanics
- 5 colorblind-friendly gem colors (Red, Yellow, Blue, White, Green)
- Visual markers on gems (white: dot+X, green: X)
- Ghost piece preview showing landing position
- Ghost piece markers (white dot on white, white X on green)
- 400ms lock delay with reset on horizontal movement
- Match-3 detection in all 4 directions (horizontal, vertical, both diagonals)
- Gravity system with chain reaction support
- Chain score multiplier (100 × chain number)
- Progressive difficulty (3% speed increase per level)
- Level progression every 1000 points
- Speed range: 500ms (level 1) to 261ms (level 20)
- Mini Joy C hat controller support
- Smooth 60fps gameplay
- Game over detection and restart
- Complete M5Burner package
- Full documentation and build instructions

### Technical
- Flash usage: 441KB (33.2%)
- RAM usage: 28KB (8.5%)
- Portrait orientation (135×240)
- Grid: 10 columns × 20 rows
- Block size: 11 pixels
- I2C configuration: SDA=0, SCL=26

## [Unreleased]

### Planned for v2.0
- 2-player WiFi battle mode
- Send garbage rows to opponent
- Next piece preview
- High score saving (EEPROM)
- Sound effects (buzzer)
- Match animation effects

---

**Format**: Based on [Keep a Changelog](https://keepachangelog.com/)

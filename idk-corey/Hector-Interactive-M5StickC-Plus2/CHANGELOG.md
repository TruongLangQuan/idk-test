# Changelog

All notable changes to the Hector Interactive M5StickC Plus2 project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [4.0.0] - 2024-10-31

### Added
- **Professional I2S Audio Processing**: Complete rewrite of audio system using proper I2S driver
- **5 New Interactive Modes**: SPIRAL, INTERFERENCE, MOUNTAIN, RIPPLE, PLASMA patterns
- **Advanced Audio Features**: RMS analysis, DC offset removal, noise gate, professional smoothing
- **Audio Debug Display**: Real-time sound level and power indicators
- **Enhanced Documentation**: Complete user guides and technical documentation
- **GitHub CI/CD**: Automated builds and testing workflow

### Fixed
- **Audio System Completely Overhauled**: Sound mode now works perfectly with I2S digital microphone
- **Microphone Pin Correction**: Changed from GPIO 36 to GPIO 34 (correct for M5StickC Plus2)
- **Audio Sensitivity**: Proper noise gate prevents false triggers from background noise
- **Performance Optimization**: Improved frame rates and memory efficiency

### Changed
- **Audio Processing**: Switched from analogRead() to I2S PDM mode (44.1kHz, 16-bit)
- **Smoothing Algorithm**: Enhanced audio smoothing (80% previous + 20% new)
- **Debug Information**: More detailed audio feedback and visual indicators
- **Flash Usage**: Increased to 35.6% due to additional features and I2S driver

## [3.0.0] - 2024-10-31

### Added
- **5 New Mathematical Patterns**: SPIRAL, INTERFERENCE, MOUNTAIN, RIPPLE, PLASMA
- **Enhanced Pattern Variety**: Total of 10 interactive modes
- **Pattern Documentation**: Detailed descriptions of each mathematical algorithm

### Fixed
- **Audio Sensitivity Issues**: Improved microphone processing (still using analogRead)
- **Display Coordinate Bounds**: Better landscape mode coordinate handling

### Changed
- **Mode Cycling**: Extended Button B cycling through all 10 modes
- **Flash Usage**: Increased to 34.8% due to additional pattern algorithms

## [2.0.0] - 2024-10-31

### Added
- **Real IMU Integration**: 6-axis MPU6886 sensor for tilt-reactive surfaces
- **Interactive Audio Mode**: Basic microphone input processing
- **5 Interactive Modes**: FLAT, TILT, SOUND, SINE, DRIP
- **Manual Mode Control**: Button-based mode switching (removed auto-cycling)
- **M5Burner Package**: Complete merged bootloader binary with JSON metadata

### Fixed
- **Landscape Mode**: Proper orientation and coordinate system
- **Display Modes**: All 4 styles (Grid/Solid/Zebra/Checkboard) working correctly
- **Button Controls**: Intuitive A=Style, B=Mode interface

### Changed
- **Display Orientation**: Changed from portrait (0) to landscape (1)
- **Screen Resolution**: Optimized for 240×135 landscape layout
- **Control Scheme**: Simplified two-button interface

## [1.2.0] - 2024-10-31

### Fixed
- **Critical Display Issue**: Fixed non-GRID modes showing only dots instead of proper patterns
- **Solid Mode**: Now draws proper filled triangles using fillTriangle()
- **Zebra Mode**: Alternating stripe patterns with drawLine()
- **Checkboard Mode**: Visible squares using fillRect() instead of single pixels
- **Coordinate Bounds**: Added proper constraint checking for landscape mode

### Changed
- **Rendering System**: Complete rewrite of drawPath() function for proper pattern display
- **Visual Quality**: All 4 display modes now show distinct, beautiful patterns

## [1.1.0] - 2024-10-31

### Added
- **Landscape Mode**: Rotated display for better viewing angle
- **Manual Controls**: Button-based mode switching
- **Status Display**: Current mode shown on screen

### Changed
- **Display Rotation**: setRotation(1) for landscape orientation
- **Screen Dimensions**: Updated for 240×135 coordinate system
- **User Interface**: Manual control replacing automatic cycling

## [1.0.0] - 2024-10-31

### Added
- **Initial Port**: Successfully ported ESP32-Hector to M5StickC Plus2
- **M5StickCPlus2 Library**: Updated to use proper M5StickCPlus2 v1.0.2
- **Basic Display**: Working mathematical visualization with GRID mode
- **Platform Support**: esp32dev board configuration
- **Original Algorithms**: SINE and DRIP wave mathematics preserved

### Fixed
- **Library Compatibility**: Resolved M5Stack library conflicts
- **Board Configuration**: Proper ESP32 device configuration for M5StickC Plus2
- **Display Initialization**: Working TFT display with correct brightness

### Changed
- **Platform**: Migrated from M5Stack Core to M5StickC Plus2
- **Library**: M5StickCPlus → M5StickCPlus2
- **Board Config**: m5stick-c → esp32dev

---

## Version History Summary

- **v4.0.0**: 🎵 **FINAL** - Professional I2S audio + 10 modes + complete documentation
- **v3.0.0**: 🎨 Enhanced with 5 new mathematical patterns (10 total modes)
- **v2.0.0**: 🎮 Interactive with IMU + basic audio (5 modes)
- **v1.2.0**: 🔧 Fixed display modes (no more dots!)
- **v1.1.0**: 📱 Landscape mode + manual controls
- **v1.0.0**: ✅ Initial working port to M5StickC Plus2

Each version built upon the previous, culminating in a complete, professional-grade interactive mathematical visualization system that exceeds the original ESP32-Hector in features and capabilities.
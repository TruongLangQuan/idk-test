# Changelog - M5StickC Plus2 Audio Spectrum Analyzer

## Version 1.0.0 - 2024-11-20

### Added
- Initial M5StickC Plus2 port from original M5StickC Plus1 version
- Audio spectrum analyzer with 8 frequency bands (125Hz - 16kHz)
- Real-time oscilloscope mode
- Musical note tuner with pitch accuracy display
- Button A cycling through all 3 modes
- Optimized display for 135x240 landscape orientation
- PlatformIO project structure for easy building
- Fixed-point FFT implementation for efficient processing
- Compatible with M5StickC Plus2 built-in I2S microphone

### Changed
- Updated from M5StickC library to M5StickCPlus2 library
- Converted TFT_eSprite to LGFX_Sprite for Plus2 compatibility
- Fixed deprecated I2S communication format warnings
- Adjusted display layout for Plus2 screen proportions
- Enhanced color palette for better visualization

### Technical Details
- Platform: ESP32 on M5StickC Plus2
- Display: 135x240 pixels, landscape mode
- Audio: I2S microphone (pins 0, 34)
- Sample Rate: 8kHz optimized for voice/music
- Processing: Dual-core with dedicated audio task
- Memory Usage: 22.9% RAM, 36.7% Flash

### Dependencies
- M5StickCPlus2 v1.0.2+
- PlatformIO ESP32 platform
- Custom FFT and pitch detection libraries included
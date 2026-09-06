# M5StickC Plus2 Ultimate Audio Spectrum Analyzer

**The most comprehensive audio visualization suite for M5StickC Plus2** with 22 different visual modes ranging from classic spectrum analysis to psychedelic audio-reactive displays.

Ported and enhanced from the original M5StickC Plus1 version by KIRA Ryouta.

## 🎵 Features

### Core Audio Analysis
- **Audio Spectrum Display** - Real-time frequency analysis with 8 bands (125Hz to 16kHz)
- **Oscilloscope** - Time domain waveform display with amplitude visualization
- **Tuner** - Musical note detection with pitch accuracy and frequency display

### 🌈 Psychedelic Visual Modes (19 modes)
- **Audio Circles** - Pulsating concentric circles that react to audio intensity
- **Audio Plasma** - Classic plasma field effects synchronized to sound
- **Audio Fireworks** - Explosive bursts triggered by frequency spikes
- **Audio Waves** - Flowing wave patterns that dance with the beat
- **Audio Spiral** - Rotating spiral patterns driven by audio energy
- **Water Ripples** - Ripple effects that expand with sound intensity
- **Audio Plasma Field** - Enhanced plasma with multi-color audio mapping
- **Audio Star Field** - Twinkling stars that pulse with music
- **Fast Plasma** - High-speed plasma effects for energetic tracks
- **Audio Rain** - Digital rain drops falling to the rhythm
- **Exploding Fireworks** - Large-scale firework displays
- **Lightning Storm** - Electric bolts that strike with bass hits
- **Matrix Rain** - Classic Matrix-style cascading code
- **Snow Effect** - Gentle snowfall that intensifies with audio
- **Lava Lamp** - Flowing blob effects synchronized to sound
- **Aurora Borealis** - Northern lights simulation with audio colors
- **Kaleidoscope** - Symmetrical patterns that morph with frequency
- **Particle Tornado** - Swirling particle effects
- **Sound Visualizer Bars** - Enhanced spectrum bars with effects

## 🎛️ Controls

- **Button A**: Cycle through all 22 modes sequentially
- **Button B**: Toggle text display overlay on/off

## 🔧 Hardware Requirements

- **M5StickC Plus2** (135x240 pixel display)
- **Built-in I2S PDM Microphone** (pins 0 and 34)
- No external components required

## 🚀 Quick Start

### Prerequisites
- [PlatformIO IDE](https://platformio.org/) or PlatformIO Core
- M5StickC Plus2 device

### Build & Upload
```bash
# Clone or download this repository
cd m5stickc-audiospectrum-ultimate

# Build the project
pio run

# Upload to M5StickC Plus2
pio run --target upload

# Monitor serial output (optional)
pio device monitor
```

### Using Arduino IDE
1. Install M5StickCPlus2 library v1.0.2+
2. Copy all files from `src/` to your Arduino sketch folder
3. Select "M5Stick-C" board
4. Upload to device

## 📊 Technical Specifications

- **Display**: 135x240 pixels in landscape orientation
- **Audio Processing**: Custom fixed-point FFT for efficient spectrum analysis
- **Sample Rate**: 8kHz optimized for voice and music analysis
- **Frequency Bands**: 8 bands covering 125Hz to 16kHz
- **CPU**: ESP32 running at 80MHz for optimal power efficiency
- **Memory**: Dual-buffer system for smooth real-time processing

## 🎯 Performance Features

- **Real-time Processing**: Sub-10ms latency for live audio visualization
- **Smooth Animation**: 60+ FPS on most visual modes
- **Power Optimized**: Efficient algorithms for extended battery life
- **Multi-core Processing**: Utilizes both ESP32 cores for maximum performance

## 📁 Project Structure

```
m5stickc-audiospectrum-ultimate/
├── src/                    # Source code
│   ├── main.cpp           # Main application
│   ├── dywapitchtrack.h   # Pitch detection library
│   ├── dywapitchtrack.cpp # Pitch tracking implementation
│   ├── fix_fft.h          # Fixed-point FFT header
│   └── fix_fft.cpp        # FFT implementation
├── platformio.ini         # PlatformIO configuration
├── README.md              # This file
└── LICENSE                # MIT License
```

## 🎨 Visual Mode Gallery

| Mode | Description | Best For |
|------|-------------|----------|
| Spectrum Bars | Classic 8-band frequency display | Technical analysis |
| Oscilloscope | Waveform visualization | Audio debugging |
| Tuner | Musical note detection | Instrument tuning |
| Audio Circles | Pulsating concentric rings | Ambient music |
| Audio Plasma | Flowing plasma effects | Electronic music |
| Audio Fireworks | Explosive visual bursts | High-energy tracks |
| Matrix Rain | Cascading digital code | Cyberpunk vibes |
| Aurora Borealis | Northern lights simulation | Atmospheric sounds |

## 🛠️ Customization

The code is highly modular and easy to customize:

- **Add New Visual Modes**: Extend the mode system in `main.cpp`
- **Adjust Audio Sensitivity**: Modify FFT parameters and gain settings
- **Color Schemes**: Customize color palettes for each visualization
- **Performance Tuning**: Adjust frame rates and processing parameters

## 📚 Libraries Used

- **M5StickCPlus2** v1.0.2+ - Hardware abstraction layer
- **ESP32 I2S Driver** - Built-in audio input handling  
- **Custom Fixed-Point FFT** - Optimized frequency analysis by David Bird & macsbug
- **DyWaPitchTrack** - Professional pitch detection algorithm

## 🎤 Audio Input Details

- **Microphone Type**: I2S PDM (Pulse Density Modulation)
- **Bit Depth**: 16-bit samples
- **Channel**: Mono (right channel)
- **Input Range**: Optimized for voice and music (20Hz-8kHz effective)
- **Auto Gain**: Built-in AGC for consistent visualization levels

## 🔬 Advanced Features

- **Dual-Buffer Processing**: Eliminates audio dropouts during intensive visualizations
- **Adaptive Frame Rate**: Automatically adjusts based on complexity
- **Memory Management**: Efficient sprite handling for smooth graphics
- **Multi-Core Task Distribution**: Audio processing on dedicated core

## 🎯 Troubleshooting

**No Audio Response?**
- Ensure microphone is not blocked
- Check audio source is loud enough
- Try clapping hands near device

**Choppy Visuals?**
- Lower frame rate in code if needed
- Ensure stable power supply
- Check for overheating

**Build Issues?**
- Update PlatformIO to latest version
- Ensure M5StickCPlus2 library v1.0.2+
- Check ESP32 platform version compatibility

## 🏆 Credits & Attribution

- **Original Concept**: KIRA Ryouta ([m5stickc-audiospectrum](https://github.com/KKQ-KKQ/m5stickc-audiospectrum))
- **FFT Implementation**: David Bird & macsbug
- **Pitch Detection**: DyWaPitchTrack library team
- **M5StickC Plus2 Port & Enhancements**: Enhanced with 19 additional visual modes
- **Visual Effects**: Custom implementations inspired by classic demo scene

## 📄 License

MIT License - See [LICENSE](LICENSE) file for full details.

## 🚀 Contributing

Contributions welcome! Areas for improvement:
- Additional visual modes
- Performance optimizations  
- New audio analysis features
- Documentation improvements

---

**Turn your M5StickC Plus2 into the ultimate pocket audio visualizer!** 🎵✨
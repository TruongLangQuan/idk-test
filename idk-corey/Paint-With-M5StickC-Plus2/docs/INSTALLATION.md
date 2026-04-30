# 📦 Installation Guide

## 🚀 Method 1: M5Burner (Recommended)

### Step 1: Download Firmware
- Download `Paint-Program-M5StickCPlus2-v1.3-FINAL.bin` from the releases folder
- File size: 524KB
- MD5: 814608ecf5173cd1e35fc05d2a9d5202

### Step 2: M5Burner Setup
1. **Open M5Burner** application
2. **Select Device**: M5StickC Plus2
3. **Load Firmware**: Browse and select the .bin file
4. **Flash Address**: 0x0 (automatic with merged binary)
5. **Click Burn**: Wait for completion

### Step 3: Hardware Setup
1. **Connect Mini Joy C Hat** to M5StickC Plus2
2. **Power On**: Device should boot to welcome screen
3. **LED Check**: Joy C LED should turn green when connected

## 🔧 Method 2: PlatformIO (Development)

### Prerequisites
```bash
# Install PlatformIO
pip install platformio

# Or via VS Code extension
# Install "PlatformIO IDE" extension
```

### Build and Flash
```bash
# Clone repository
git clone https://github.com/coreymillia/Paint-Program-M5StickCPlus2.git
cd Paint-Program-M5StickCPlus2

# Build project
pio run

# Flash to device (USB connected)
pio run -t upload

# Monitor serial output (optional)
pio device monitor
```

## 🛠️ Hardware Requirements

### Required Components
- **M5StickC Plus2**
  - ESP32-PICO-V3-02 chip
  - 135x240 color LCD
  - Built-in battery and charging

- **Mini Joy C Hat**
  - I2C address: 0x54
  - Analog joystick with center button
  - RGB LED indicator
  - Connects via Grove connector

### Connections
- **SDA**: GPIO0 (automatic)
- **SCL**: GPIO26 (automatic)
- **Power**: 3.3V from M5StickC Plus2
- **Grove Connector**: Hat connects directly

## 🎮 First Run

### Expected Behavior
1. **Boot Screen**: "PAINT PROGRAM v1.3" with demo flower
2. **Battery Display**: Shows current battery percentage
3. **Instructions**: Control layout and usage guide
4. **Joy C LED**: Green when hat connected properly

### Troubleshooting
| Issue | Solution |
|-------|----------|
| **Blank screen** | Check M5Burner flash was successful |
| **No LED on Joy C** | Check hat connection, try reseating |
| **No cursor movement** | Verify I2C connections (SDA=0, SCL=26) |
| **Boot loop** | Reflash firmware, check power supply |

## 🔧 Configuration

### Default Settings
- **Brush Size**: 2 pixels
- **Color**: Red
- **Background**: White
- **Draw Mode**: Disabled (MOVE mode)

### Customization
Settings are automatically saved to EEPROM:
- Last used brush size
- Last selected color
- Last background color
- Menu selections

## 📐 Technical Details

### Memory Usage
- **Flash**: 458KB (35% of 4MB)
- **RAM**: 25KB (7.7% of 320KB)
- **EEPROM**: <1KB for settings

### Performance
- **Update Rate**: 30ms cycle
- **Joystick Sampling**: 12-bit ADC
- **Response Time**: <50ms input to display

### Compatibility
- **M5StickC Plus2**: ✅ Full support
- **M5StickC Plus**: ❓ Untested (likely compatible)
- **M5StickC**: ❌ Different display size

## 🔄 Updates

### Checking Version
- Version displayed on welcome screen
- Current version: v1.3.0

### Installing Updates
1. Download new .bin file from releases
2. Flash via M5Burner (same process)
3. Settings preserved automatically

## 💡 Tips

### Best Performance
- **Fully charge** device before use
- **Clean connections** between hat and device
- **Stable surface** for detailed artwork

### Creative Tips
- Start with **small brush sizes** for details
- Use **background colors** for different effects
- **Hold drawing mode** only when actively painting
- **Quick color cycling** for fast color changes

---

**Ready to create digital art!** 🎨✨
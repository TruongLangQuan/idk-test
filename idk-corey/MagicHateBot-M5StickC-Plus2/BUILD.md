# Magic Hate Ball - Build Instructions

## 🛠️ Building from Source

### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- M5StickC Plus2 device
- USB cable for flashing

### Build Steps

1. **Clone the repository**
   ```bash
   git clone <your-repo-url>
   cd MagicHateBall
   ```

2. **Open in PlatformIO**
   - Open the project folder in PlatformIO
   - PlatformIO will automatically install dependencies from `platformio.ini`

3. **Build the project**
   ```bash
   pio run
   ```

4. **Flash to device**
   ```bash
   pio run --target upload
   ```

### Project Structure
```
MagicHateBall/
├── src/
│   └── main.cpp          # Main source code (138 responses + 3 animated faces)
├── platformio.ini        # PlatformIO configuration
├── MagicHateBot2.bin     # Pre-built merged binary (ready to flash)
├── m5burner_config.json  # M5Burner metadata
├── development-bins.zip  # All development versions
└── README.md             # This file
```

### Dependencies
Automatically installed by PlatformIO:
- `M5StickCPlus2 @ ^1.0.2`
- `M5Unified Framework`
- `M5GFX`

### Configuration
All settings are in `src/main.cpp`:
- `SHAKE_THRESHOLD`: 2.0g (sensitivity for shake detection)
- `cooldownPeriod`: 1000ms (delay between answers)
- `SLEEP_TIMEOUT`: 60000ms (auto-sleep after 60 seconds)
- `BRIGHTNESS_LEVELS`: {1, 25, 50, 75, 100}%

### Customization

#### Add More Responses
Edit the `answers[]` array in `src/main.cpp` (starting at line 37):
```cpp
const String answers[] = {
    "Your new sarcastic response here",
    // ... add more
};
```

#### Adjust Face Animation Speed
Change timing in `animateFaces()` function (line 262):
```cpp
for (int d = 0; d < 8; d++) {  // 8 × 100ms = 800ms per face
    delay(100);
```

#### Modify Cooldown Period
Change line 9:
```cpp
const long cooldownPeriod = 1000;  // milliseconds
```

### Flashing Methods

#### Method 1: PlatformIO (Developers)
```bash
pio run --target upload
```

#### Method 2: M5Burner (Users)
1. Download [M5Burner](https://docs.m5stack.com/en/download)
2. Open M5Burner
3. Select M5StickC Plus2
4. Choose `MagicHateBot2.bin`
5. Flash at address `0x0`

### Creating Merged Binary
If you modify the code and want to create a new merged binary:
```bash
pio run
python3 -c "
import sys

# Read bootloader (at 0x1000)
with open('.pio/build/esp32dev/bootloader.bin', 'rb') as f:
    bootloader = f.read()

# Read partitions (at 0x8000)
with open('.pio/build/esp32dev/partitions.bin', 'rb') as f:
    partitions = f.read()

# Read firmware (at 0x10000)
with open('.pio/build/esp32dev/firmware.bin', 'rb') as f:
    firmware = f.read()

# Create merged binary
merged = bytearray(0x10000 + len(firmware))
merged[0x1000:0x1000 + len(bootloader)] = bootloader
merged[0x8000:0x8000 + len(partitions)] = partitions
merged[0x10000:0x10000 + len(firmware)] = firmware

# Write merged file
with open('MagicHateBot2-MERGED.bin', 'wb') as f:
    f.write(merged)
print('Merged binary created!')
"
```

### Troubleshooting

**Issue**: Device not detected
- **Solution**: Check USB cable supports data transfer
- **Solution**: Install [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

**Issue**: Build fails
- **Solution**: Clean build directory: `pio run --target clean`
- **Solution**: Update PlatformIO: `pio upgrade`

**Issue**: Shake not detected
- **Solution**: Increase `SHAKE_THRESHOLD` (line 8)
- **Solution**: Shake more vigorously

**Issue**: Responses repeat too often
- **Solution**: Add more responses to `answers[]` array

### Memory Usage
- **RAM**: 8.6% (28,072 bytes)
- **Flash**: 35.2% (461,245 bytes)
- **Plenty of room** for more responses or features!

### Contributing
1. Fork the repository
2. Make your changes
3. Test thoroughly on hardware
4. Submit a pull request

### License
See LICENSE file in repository

---

**Made with 💀 sarcasm and ☕ caffeine**

*"Ask a stupid question, get an animated judgmental face!" 🎱🎭*

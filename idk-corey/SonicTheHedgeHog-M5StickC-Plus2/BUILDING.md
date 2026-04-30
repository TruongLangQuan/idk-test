# Building M5StickSonic from Source

Complete guide to building the project from source code.

---

## Prerequisites

### Required Software

1. **PlatformIO** (Recommended)
   - Install: https://platformio.org/install
   - VS Code extension: `platformio-ide`
   - OR Command line: `pip install platformio`

2. **Git** (for cloning)
   - Install: https://git-scm.com/

### Alternative: Arduino IDE

- Arduino IDE 2.x: https://www.arduino.cc/en/software
- ESP32 board support
- M5StickCPlus2 library

---

## Method 1: PlatformIO (Recommended)

### Step 1: Clone Repository

```bash
git clone <repository-url>
cd M5StickSonic
```

### Step 2: Build

```bash
# Build the project
pio run

# Build and upload
pio run --target upload

# Clean build
pio run --target clean
```

### Step 3: Monitor Serial Output

```bash
pio device monitor
```

### Build Outputs

```
.pio/build/m5stick-c/
├── firmware.bin          # Application binary
├── firmware.elf          # ELF file with symbols
├── bootloader.bin        # ESP32 bootloader
└── partitions.bin        # Partition table
```

---

## Method 2: PlatformIO IDE (VS Code)

### Step 1: Open Project

1. Open VS Code
2. Install PlatformIO IDE extension
3. File > Open Folder > Select M5StickSonic folder

### Step 2: Build and Upload

- **Build**: Click ✓ button (bottom toolbar)
- **Upload**: Click → button
- **Serial Monitor**: Click 🔌 button

---

## Method 3: Arduino IDE

### Step 1: Setup Arduino IDE

1. Install Arduino IDE 2.x
2. Add ESP32 board support:
   - File > Preferences
   - Additional Board URLs: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Tools > Board > Boards Manager > Search "ESP32" > Install

3. Install M5StickCPlus2 library:
   - Sketch > Include Library > Manage Libraries
   - Search "M5StickCPlus2" > Install

### Step 2: Prepare Files

1. Create new sketch: `M5StickSonic.ino`
2. Copy `src/main.cpp` content to .ino file
3. Copy all `.cpp` files from `src/` to sketch folder
4. Copy all `.h` files from `include/` to sketch folder
5. Maintain folder structure for `content/`, `entities/`, `structs/`

### Step 3: Configure Board

- Tools > Board > ESP32 Arduino > M5Stick-C
- Tools > Upload Speed > 1500000
- Tools > Flash Size > 4MB
- Tools > Partition Scheme > Default 4MB with spiffs

### Step 4: Build and Upload

- Sketch > Verify/Compile (Ctrl+R)
- Sketch > Upload (Ctrl+U)

---

## Configuration Options

### Resolution Settings

Edit `include/GameSettings.h`:

```cpp
// Choose ONE:
#define OPTIMAL_RESOLUTION    // 211x119 @ 27-30 FPS (default)
// #define FULL_RESOLUTION     // 240x135 @ 8-10 FPS (detailed but slow)
// #define MAX_FPS_RESOLUTION  // 120x68 @ 47-60 FPS (fast but small)
```

### JoyCHat Settings

```cpp
#define JOYC_ADDR       0x54    // I2C address (default)
#define JOYC_DEADZONE   20      // Joystick deadzone
#define JOYC_THRESHOLD  100     // Direction threshold
```

### Debug Settings

Edit `platformio.ini`:

```ini
build_flags =
    -DCORE_DEBUG_LEVEL=0    # 0=None, 3=Verbose
```

---

## Creating Release Binaries

### Merged Binary for M5Burner

```bash
# After building with PlatformIO:
cd .pio/build/m5stick-c/

# Create merged binary
~/.platformio/packages/tool-esptoolpy/esptool.py \
  --chip esp32 merge_bin \
  -o M5StickSonic-MERGED.bin \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 4MB \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

This creates a single .bin file that can be flashed to address 0x0.

---

## Troubleshooting

### Build Errors

**Error: "M5StickCPlus2.h not found"**
```bash
# PlatformIO will auto-install, but if needed:
pio lib install "m5stack/M5StickCPlus2@^1.0.2"
```

**Error: "UNIT_MiniJoyC.h not found"**
- This is included in the project (src/UNIT_MiniJoyC.h)
- Make sure all files are copied correctly

**Error: "Backslash errors"**
- The project uses forward slashes (/)
- Check include statements don't use backslashes (\)

### Upload Errors

**Error: "Serial port not found"**
- Check USB cable is connected
- Install CP210x drivers if needed
- Check port permissions (Linux: add user to dialout group)

**Error: "Timeout connecting to bootloader"**
- Hold BtnA while connecting USB
- Press reset button during upload
- Try lower baud rate: `upload_speed = 115200`

### Runtime Errors

**"JoyC not found!" on screen**
- Connect Mini Joy C Hat to Grove port BEFORE powering on
- Check I2C address is 0x54
- Verify SDA=0, SCL=26 wiring

**Game too slow**
- Try MAX_FPS_RESOLUTION (120x68)
- Or reduce game logic complexity

**Game screen too small**
- Try FULL_RESOLUTION (240x135)
- Note: Will run at 8-10 FPS

---

## Advanced: Custom Modifications

### Adding New Levels

1. Create level data in `include/content/levels/`
2. Add terrain tiles and objects
3. Update `Level.cpp` to load new level
4. Test and adjust for memory constraints

### Modifying Physics

Edit `include/Player.h`:

```cpp
#define PL_ACC          0.046875    // Acceleration
#define PL_DEC          0.5         // Deceleration
#define PL_FRC          0.046875    // Friction
#define PL_TOP          6.0         // Top speed
```

### Adding Custom Graphics

1. Convert image to indexed color (palette-based)
2. Create header file with uint8_t array
3. Add to `include/content/textures/`
4. Reference in `Screen.cpp` texture list

---

## Build Performance

### Compilation Time

- **Clean build**: ~2-3 minutes
- **Incremental**: ~30-60 seconds
- **Upload time**: ~20 seconds

### Binary Sizes

- **Application**: ~855 KB (66.7% flash)
- **Bootloader**: ~17 KB
- **Partitions**: ~3 KB
- **Total merged**: ~919 KB

### Memory Usage

- **Flash**: 855 KB / 1280 KB (66.7%)
- **RAM**: 65 KB / 320 KB (20.1%)
- **Heap**: Dynamic during runtime

---

## Platform Specific Notes

### Windows
- Use PowerShell or CMD
- Install CP210x drivers
- COM port typically COM3, COM4, etc.

### Linux
- Add user to dialout group: `sudo usermod -a -G dialout $USER`
- Port typically `/dev/ttyUSB0` or `/dev/ttyACM0`
- May need `sudo` for flashing

### macOS
- Port typically `/dev/cu.usbserial-*`
- Install CP210x drivers from Silicon Labs
- May need to allow in Security & Privacy

---

## Getting Help

- Check this guide first
- Read error messages carefully
- Search GitHub Issues
- Check PlatformIO documentation
- M5Stack forum: https://community.m5stack.com/

---

**Happy Building!** 🔨

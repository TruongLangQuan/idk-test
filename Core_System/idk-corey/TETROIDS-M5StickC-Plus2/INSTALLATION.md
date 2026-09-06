# 📥 Installation Guide - TetroidsM5

Complete instructions for getting TetroidsM5 running on your M5StickC Plus2.

---

## 🎯 Quick Flash (Recommended)

### Prerequisites
- M5StickC Plus2 device
- Mini JoyC Hat accessory
- USB-C cable
- Computer (Windows/Mac/Linux)

### Step 1: Download Binary
Download the pre-built firmware: `TetroidsM5-v1.0-MERGED.bin`

### Step 2: Flash with M5Burner (Easiest)

**M5Burner** is the official M5Stack flashing tool.

1. Download [M5Burner](https://docs.m5stack.com/en/download)
2. Install and launch M5Burner
3. Connect M5StickC Plus2 via USB-C
4. Select your COM port
5. Click "Custom" tab
6. Load `TetroidsM5-v1.0-MERGED.bin`
7. Set flash parameters:
   - **Offset**: 0x0
   - **Baud Rate**: 1500000
8. Click "Burn" and wait ~30 seconds

### Step 3: Flash with esptool (Advanced)

If you prefer command line:

```bash
# Install esptool
pip install esptool

# Flash the binary (Linux/Mac)
esptool.py --chip esp32s3 --port /dev/ttyACM0 \
  --baud 1500000 \
  write_flash 0x0 TetroidsM5-v1.0-MERGED.bin

# Flash the binary (Windows)
esptool.py --chip esp32s3 --port COM3 ^
  --baud 1500000 ^
  write_flash 0x0 TetroidsM5-v1.0-MERGED.bin
```

**Note**: Replace `/dev/ttyACM0` or `COM3` with your actual port.

### Step 4: Attach Hardware
1. Power off M5StickC Plus2
2. Attach Mini JoyC Hat to Grove connector
3. Power on - game starts automatically!

---

## 🛠️ Build from Source

For developers who want to modify the code.

### Prerequisites
- [PlatformIO IDE](https://platformio.org/install) or PlatformIO CLI
- Git (to clone repository)
- USB-C cable

### Step 1: Clone Repository

```bash
git clone https://github.com/yourusername/TetroidsM5.git
cd TetroidsM5
```

### Step 2: Open in PlatformIO

**VS Code + PlatformIO Extension:**
1. Open VS Code
2. File → Open Folder → Select `TetroidsM5`
3. PlatformIO will auto-detect `platformio.ini`
4. Wait for dependencies to install

**PlatformIO CLI:**
```bash
cd TetroidsM5
pio run  # Build only
```

### Step 3: Build & Upload

**Via VS Code:**
1. Connect M5StickC Plus2
2. Click PlatformIO icon (alien head)
3. Project Tasks → m5stick-c-plus2 → Upload

**Via CLI:**
```bash
pio run --target upload
```

**Monitor Serial Output:**
```bash
pio device monitor -b 115200
```

---

## 🔧 Troubleshooting

### Device Not Detected

**Linux:**
```bash
# Check if device appears
lsusb | grep -i "CP210\|CH340\|FTDI"

# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and back in
```

**Windows:**
- Install [CP210x USB drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- Check Device Manager for COM port

**Mac:**
```bash
# Check devices
ls /dev/cu.*

# Install drivers if needed
brew install --cask homebrew/cask-drivers/silicon-labs-vcp-driver
```

### Flash Fails

**Error: "Timeout waiting for packet header"**
1. Hold Button A during connection
2. Release after esptool connects
3. Try lower baud rate: `--baud 921600`

**Error: "Flash size mismatch"**
- Use `--flash_size detect` or `--flash_size 8MB`

### Mini JoyC Not Working

1. Check Grove connector is fully seated
2. Verify I2C address 0x54:
   ```bash
   pio device monitor
   # Should see "JoyC initialized" in output
   ```
3. Try power cycling the device

### Game Crashes/Freezes

1. Re-flash with erase:
   ```bash
   esptool.py --chip esp32s3 erase_flash
   # Then flash again
   ```
2. Check USB cable quality (data-capable, not charge-only)
3. Monitor serial output for error messages

---

## 📋 platformio.ini Configuration

The project uses these settings:

```ini
[env:m5stick-c-plus2]
platform = espressif32
board = m5stick-c-plus2
framework = arduino
lib_deps = m5stack/M5StickCPlus2@^1.0.2
monitor_speed = 115200
upload_speed = 1500000
```

---

## 🔄 Updating Firmware

To update to a new version:

1. Download new `.bin` file
2. Flash using same method as installation
3. Previous save data is preserved (if implemented)

---

## 🆘 Still Having Issues?

- Check [GitHub Issues](https://github.com/yourusername/TetroidsM5/issues)
- Open a new issue with:
  - Your OS and version
  - Error messages
  - Serial monitor output
  - Hardware setup photos

---

**Happy Gaming!** 🚀

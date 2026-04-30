# Building and Flashing ScorchedM5

## Prerequisites

### For Building from Source:
- [PlatformIO Core](https://platformio.org/install/cli) or [PlatformIO IDE](https://platformio.org/install/ide)
- USB cable to connect M5StickC Plus2 to computer
- M5StickC Plus2 device

### For Flashing Pre-built Binary:
- [M5Burner](https://docs.m5stack.com/en/download) tool
- USB cable
- M5StickC Plus2 device

## Method 1: Build from Source with PlatformIO

### Using PlatformIO CLI:

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/ScorchedM5.git
   cd ScorchedM5
   ```

2. **Build the firmware:**
   ```bash
   pio run -e m5stick-c-plus2
   ```

3. **Upload to device:**
   ```bash
   pio run -e m5stick-c-plus2 -t upload
   ```

4. **Build and upload in one command:**
   ```bash
   pio run -e m5stick-c-plus2 -t upload
   ```

### Using PlatformIO IDE (VSCode):

1. Install VSCode and PlatformIO extension
2. Open folder in VSCode: `File -> Open Folder` → select ScorchedM5 folder
3. Wait for PlatformIO to initialize the project
4. Click "Build" button (checkmark icon) in bottom toolbar
5. Connect M5StickC Plus2 via USB
6. Click "Upload" button (arrow icon) in bottom toolbar

### Troubleshooting Build Issues:

**"Library not found" errors:**
```bash
pio pkg install
```

**Clean build:**
```bash
pio run -t clean
pio run -e m5stick-c-plus2
```

**Update platform:**
```bash
pio pkg update
```

## Method 2: Flash Pre-built Binary with M5Burner

### Step-by-step:

1. **Download M5Burner:**
   - Visit [M5Stack Downloads](https://docs.m5stack.com/en/download)
   - Download M5Burner for your OS (Windows/Mac/Linux)
   - Install and run M5Burner

2. **Get the firmware:**
   - Download `ScorchedM5-v1.0-MERGED.bin` from the [Releases](https://github.com/yourusername/ScorchedM5/releases) page
   - This file includes the bootloader and partition table

3. **Flash to device:**
   - Connect M5StickC Plus2 to computer via USB
   - Open M5Burner
   - Select COM port for your device
   - Click "Custom" or "Add" button
   - Browse and select the downloaded `.bin` file
   - Click "Burn" button
   - Wait for flash to complete (usually 30-60 seconds)

4. **Verify:**
   - Device will auto-restart
   - Game should launch automatically
   - If not, press the power button to start

### M5Burner Troubleshooting:

**Device not detected:**
- Check USB cable (use data cable, not charge-only)
- Install [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) if needed
- Try different USB port
- Press reset button on device

**Flash fails:**
- Press and hold reset button during flash initialization
- Reduce baud rate in M5Burner settings
- Ensure battery is charged or device is powered

**Device boots to wrong app:**
- Flash again ensuring complete write
- Perform erase flash before burning

## Method 3: Manual Flash with esptool

For advanced users:

```bash
# Install esptool
pip install esptool

# Erase flash (optional but recommended)
esptool.py --chip esp32s3 --port /dev/ttyUSB0 erase_flash

# Flash the merged binary
esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 \
  write_flash 0x0 ScorchedM5-v1.0-MERGED.bin
```

Replace `/dev/ttyUSB0` with your actual port (check with `pio device list`).

## Verifying Installation

After flashing:

1. Device displays game screen
2. Two tanks visible on terrain
3. Clouds scrolling in sky
4. Buttons respond to input
5. Projectile fires when middle button held

## Creating Your Own Merged Binary

If you modify the code and want to create a merged binary for M5Burner:

```bash
# Build the project
pio run -e m5stick-c-plus2

# The firmware.bin will be at:
# .pio/build/m5stick-c-plus2/firmware.bin

# Get bootloader and partitions from:
# ~/.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32s3/bin/

# Merge them (example using esptool):
python -m esptool --chip esp32s3 merge_bin \
  -o ScorchedM5-custom-MERGED.bin \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 4MB \
  0x0 bootloader_dio_80m.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

Or follow the same method used in the TetriSnake project.

## Platform-Specific Notes

### Windows:
- May need to install CH340 or CP210x drivers
- Use Device Manager to verify COM port
- M5Burner handles most driver issues automatically

### macOS:
- Port usually appears as `/dev/cu.usbserial-*`
- May need to approve kext in Security & Privacy settings
- Use `ls /dev/cu.*` to find device

### Linux:
- Port usually appears as `/dev/ttyUSB0` or `/dev/ttyACM0`
- Add user to dialout group: `sudo usermod -a -G dialout $USER`
- Log out and back in for group change to take effect
- Use `pio device list` to find port

## Getting Help

- Check [Issues](https://github.com/yourusername/ScorchedM5/issues) for known problems
- Review [M5StickC Plus2 Documentation](https://docs.m5stack.com/en/core/M5StickC%20PLUS2)
- Visit [PlatformIO Documentation](https://docs.platformio.org/)

## Next Steps

Once installed, see [README.md](README.md) for:
- How to play
- Game controls
- Gameplay mechanics
- Tips and strategies

---

Happy building and gaming! 🎮

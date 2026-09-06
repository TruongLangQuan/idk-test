# M5GroqBot v1.3 Installation Guide

## Package Contents
✅ **M5GroqBot-v1.4-MERGED.bin** (1.3MB) - Complete firmware with bootloader & partitions  
✅ **m5burner_config.json** - M5Burner configuration file  
✅ **README_M5BURNER.md** - User documentation  
✅ **bootloader.bin** - ESP32 bootloader (for reference)  
✅ **partitions.bin** - Partition table (for reference)  
✅ **M5GroqBot-v1.4-APP.bin** - Application only (for OTA updates)  
✅ **checksums.md5** - MD5 checksums for verification  

---

## Installation Method 1: M5Burner (Easiest)

### Step 1: Download M5Burner
- Download from: https://m5stack.com/pages/download
- Available for Windows, macOS, and Linux

### Step 2: Add Custom Firmware
1. Open M5Burner
2. Click "Custom" tab
3. Click "Add" button
4. Select `M5GroqBot-v1.3-MERGED.bin`
5. The configuration will auto-load from `m5burner_config.json`

### Step 3: Flash to Device
1. Connect M5StickC Plus2 via USB
2. Select the COM port
3. Click "Burn" button
4. Wait for "Success" message (~30 seconds)
5. Device will auto-restart

---

## Installation Method 2: esptool.py (Command Line)

### Requirements
```bash
pip install esptool
```

### Flash Command
```bash
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 write_flash 0x0 M5GroqBot-v1.4-MERGED.bin
```

**Port Notes:**
- Linux: Usually `/dev/ttyUSB0` or `/dev/ttyACM0`
- macOS: Usually `/dev/cu.usbserial-*`
- Windows: Usually `COM3` or `COM4`

---

## Installation Method 3: PlatformIO (For Developers)

### Upload via PlatformIO
```bash
pio run --target upload
```

### Or flash the merged binary
```bash
pio run --target upload --upload-port /dev/ttyACM0
```

---

## First Boot Setup

### 1. Connect to WiFi AP
- Device creates AP: `M5GroqBot-Setup`
- Password: (none - open network)
- Connect with phone or computer

### 2. Configure Settings
- Browser should auto-open to setup page
- If not, navigate to: `http://192.168.4.1`
- Enter your WiFi credentials
- Enter Groq API key (get free at: https://console.groq.com/keys)
- Click "Save"

### 3. Device Connects
- Device will restart
- Connects to your WiFi
- Shows "READY!" screen with animated waves

---

## Usage

### Ask a Question
1. **Press Button A** to start recording
2. Speak your question clearly (2 seconds)
3. Release button when done
4. View transcription
5. AI response appears on screen

### Navigate Response
- **Tap Button B** - Scroll up
- **Hold Button B** - Cycle text colors (8 colors available)
- **Power Button** - Scroll down
- **PWR at end** - Reset for next question

### Available Colors
1. White
2. Cyan
3. Green
4. Yellow
5. Magenta
6. Red
7. Blue
8. Orange

---

## Troubleshooting

### Device Not Detected
- Install CP210x or CH340 USB drivers
- Try different USB cable (must support data)
- Press and hold Button A during power-on to enter download mode

### Flash Failed
- Reduce baud rate: `--baud 115200`
- Try holding Button A while flashing
- Check USB cable quality

### WiFi Setup Not Working
- Hold power button for 6 seconds to factory reset
- Device will restart in AP mode

### Verify Installation
```bash
md5sum M5GroqBot-v1.4-MERGED.bin
# Compare with checksums.md5
```

---

## Technical Specifications

| Specification | Value |
|---------------|-------|
| Target Device | M5StickC Plus2 |
| Chip | ESP32-PICO-V3-02 |
| Flash Offset | 0x0 |
| Flash Size | 16MB |
| Firmware Size | 1.3MB |
| Bootloader | 0x1000 (17KB) |
| Partitions | 0x8000 (3KB) |
| Application | 0x10000 (1.2MB) |

---

## Support

- **GitHub Issues:** https://github.com/Coreymillia
- **Groq Documentation:** https://console.groq.com/docs
- **M5Stack Forum:** https://community.m5stack.com

---

**Happy chatting!** 🎤🤖

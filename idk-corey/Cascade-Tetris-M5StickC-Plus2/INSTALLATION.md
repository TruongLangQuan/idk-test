# 📦 Installation Guide - CASCADE TETRIS

## 🔧 Hardware Setup

### Required Hardware
1. **M5StickC Plus2** - Main device
2. **MiniJoyC Unit** - I2C joystick accessory
3. **USB-C Cable** - For programming/power

### Hardware Connection
1. Connect MiniJoyC to M5StickC Plus2 Grove port
2. Power on M5StickC Plus2
3. Joystick should be detected automatically

## ⚡ Quick Flash (Recommended)

### Prerequisites
- Install [esptool](https://pypi.org/project/esptool/):
```bash
pip install esptool
```

### Flash Steps
1. Download `CASCADE_TETRIS_v1.0_FINAL_MERGED_*.bin`
2. Connect M5StickC Plus2 via USB-C
3. Flash the binary:
```bash
esptool --chip esp32 --port /dev/ttyUSB0 write-flash 0x0 CASCADE_TETRIS_v1.0_FINAL_MERGED_*.bin
```

**Note**: Replace `/dev/ttyUSB0` with your device port:
- **Linux/Mac**: `/dev/ttyUSB0` or `/dev/ttyACM0`  
- **Windows**: `COM3`, `COM4`, etc.

## 🛠️ Build from Source

### Prerequisites
1. Install [PlatformIO](https://platformio.org/platformio-ide)
2. Install [Git](https://git-scm.com/)

### Build Steps
1. Clone repository:
```bash
git clone https://github.com/YOUR_USERNAME/CASCADE-TETRIS-M5StickC-Plus2.git
cd CASCADE-TETRIS-M5StickC-Plus2
```

2. Build and upload:
```bash
platformio run --target upload
```

### Alternative: PlatformIO IDE
1. Open PlatformIO IDE
2. Import project folder
3. Click "Build" then "Upload"

## 🔍 Troubleshooting

### Device Not Found
- **Linux**: Add user to `dialout` group: `sudo usermod -a -G dialout $USER`
- **Windows**: Install [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- **Mac**: Install [CP210x drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

### Flash Failed
- Try different baud rate: `--baud 115200`
- Hold RESET button during flash
- Check USB cable (data cable, not charge-only)

### Build Errors
- Update PlatformIO: `pio update`
- Clean build: `platformio run --target clean`
- Check library versions in `platformio.ini`

### Joystick Not Working
- Check Grove connection
- Power cycle device
- Verify MiniJoyC address (0x52)

## 📋 Verification

### Success Indicators
1. Device boots to CASCADE TETRIS screen
2. Joystick responds to movement
3. Buttons function correctly:
   - **Button A**: Hold piece
   - **Power**: Pause
   - **Button B**: Toggle CASCADE

### Next Steps
- See [README.md](README.md) for gameplay instructions
- Try 2-player battle mode with second device
- Experiment with CASCADE physics!

## 🆘 Support

Having issues? Check:
1. Hardware connections
2. Driver installation  
3. Port permissions
4. USB cable quality
5. M5StickC Plus2 battery level

**Still stuck?** Open an issue with:
- Operating system
- Error messages
- Hardware setup details
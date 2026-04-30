# Development Bins

⚠️ **WARNING**: These are experimental development builds. Use at your own risk!

## Files

### `M5GroqBot_v1.9_BOOT_MENU.bin`
- **Date**: 2024-11-13
- **Status**: ⚠️ Experimental
- **MD5**: 1966ce31a1d7a0598ed31951ec1384c5
- **Features**:
  - Settings menu accessible during boot (10 second delay)
  - Hold B button to access menu
  - WiFi reset and scan options
- **Issues**:
  - 10 second boot delay on every startup
  - Menu conflicts with normal operation
  - Not recommended for production use

### `M5GroqBot_v1.9_TLS_FIX_LATEST.bin`
- **Date**: 2024-11-13
- **Status**: ⚠️ Unstable
- **MD5**: 47a01faa2c15b3b2b01f09bc28af231a
- **Features**:
  - Attempted fix for TLS buffer leak
  - WiFiClientSecure properly managed
  - 100ms delay after requests
- **Issues**:
  - Still crashes after 2-3 messages
  - Not fully tested
  - Use v1.8 stable instead

## Installation

Flash using esptool:

```bash
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x10000 [filename].bin
```

**Note**: These bins are firmware-only. You'll need to configure WiFi and API key via the captive portal after flashing.

## Reverting to Stable

If you experience issues, flash the stable v1.8:

```bash
cd ..
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x0 M5GroqBot-v1.8-MERGED.bin
```

## Feedback

Found a bug or want to test? Please report:
- GitHub Issues: https://github.com/YourUsername/M5StickCPlus2-GroqBot/issues
- Include: version, what you were doing, serial output if possible

---

**✅ For stable, production-ready firmware, use `M5GroqBot-v1.8-MERGED.bin` from the parent folder.**

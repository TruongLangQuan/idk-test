# Troubleshooting Guide

## WiFi Issues

### Device Not Creating AP
**Problem:** WiFi AP "M5GroqBot-Setup" doesn't appear

**Solutions:**
1. Hold power button for 6 seconds to force reset
2. Check if device is fully charged
3. Wait 30 seconds after boot for AP to start
4. Try restarting the device

### Cannot Connect to WiFi AP
**Problem:** Phone/computer can't connect to setup AP

**Solutions:**
1. Forget the network and reconnect
2. Disable mobile data on your phone
3. Try a different device
4. Check if AP name appears correctly

### Setup Page Not Opening
**Problem:** Browser doesn't auto-open after connecting

**Solutions:**
1. Manually navigate to `http://192.168.4.1`
2. Try `http://m5groqbot.local`
3. Disable VPN if active
4. Clear browser cache

### WiFi Connection Fails
**Problem:** Device can't connect to home WiFi

**Solutions:**
1. Verify WiFi password is correct
2. Ensure 2.4GHz WiFi is enabled (not 5GHz only)
3. Check if WiFi SSID has special characters
4. Move device closer to router
5. Restart router

## API Key Issues

### "Error: No API key"
**Problem:** Device shows no API key error

**Solutions:**
1. Re-enter setup mode (hold PWR button)
2. Verify API key is valid at console.groq.com
3. Check for extra spaces when pasting
4. Ensure key hasn't expired

### "Error: Connection failed"
**Problem:** Cannot reach Groq servers

**Solutions:**
1. Check WiFi connection (icon on screen)
2. Verify internet connectivity
3. Check if Groq API is operational (status.groq.com)
4. Try different WiFi network

### "Error: Parse failed"
**Problem:** JSON parsing error from API

**Solutions:**
1. Restart device
2. Check API rate limits
3. Verify Groq API key permissions
4. Try again after a few minutes

## Recording Issues

### No Audio Detected
**Problem:** Recording happens but no transcription

**Solutions:**
1. Speak louder and clearer
2. Reduce background noise
3. Hold device 6-12 inches from mouth
4. Check if microphone hole is blocked
5. Try recording in quieter environment

### "Error: Recording too short"
**Problem:** Not enough audio captured

**Solutions:**
1. Speak for full 2 seconds
2. Don't release button early
3. Speak continuously, don't pause
4. Start speaking immediately after pressing button

### Transcription Inaccurate
**Problem:** Words not recognized correctly

**Solutions:**
1. Speak more clearly
2. Reduce background noise
3. Use simpler sentences
4. Avoid technical jargon
5. Try speaking slightly slower

## Display Issues

### Screen Blank
**Problem:** Display shows nothing

**Solutions:**
1. Check battery level
2. Press power button to wake
3. Restart device
4. Check LCD cable connection

### Text Cut Off
**Problem:** Text doesn't fit on screen

**Solutions:**
1. This is normal - use scroll buttons
2. Button B to scroll up
3. PWR to scroll down
4. Text wraps automatically at word boundaries

### Can't Read Text
**Problem:** Text too small or wrong color

**Solutions:**
1. Hold Button B to cycle colors
2. Find a high-contrast color
3. Adjust screen brightness (if supported)

### Colors Not Changing
**Problem:** Holding Button B doesn't cycle colors

**Solutions:**
1. Hold button for at least 0.8 seconds
2. Release and hold again
3. Only works when viewing AI response
4. Restart device if stuck

## Button Issues

### Button Not Responding
**Problem:** Buttons don't work

**Solutions:**
1. Press firmly but gently
2. Wait for screen to update
3. Don't press multiple buttons simultaneously
4. Restart device

### Scrolling Not Working
**Problem:** Can't scroll through message

**Solutions:**
1. Tap Button B quickly (< 0.8s) for scroll up
2. Press PWR button for scroll down
3. Only works when message is displayed
4. Restart if buttons stuck

## Memory Issues

### Device Reboots After Response
**Problem:** Device restarts after every answer

**Solutions:**
- This is **normal behavior** (by design)
- Prevents memory crashes
- Takes ~3-4 seconds to restart
- Device ready for next question immediately

### Frequent Crashes
**Problem:** Device freezes or crashes often

**Solutions:**
1. Flash latest firmware version
2. Clear preferences (factory reset)
3. Check power supply stability
4. Reduce ambient temperature

## Performance Issues

### Slow Response Times
**Problem:** Takes long to get answer

**Solutions:**
1. Check WiFi signal strength
2. Verify internet speed
3. Groq API may be under load
4. Try shorter questions

### Battery Drains Quickly
**Problem:** Device runs out of power fast

**Solutions:**
1. Normal - WiFi and mic use power
2. Reduce screen brightness
3. Use USB power for extended sessions
4. Replace battery if old

## Update/Flash Issues

### Flash Failed
**Problem:** Firmware upload fails

**Solutions:**
1. Try lower baud rate: `--baud 115200`
2. Hold Button A during power-on
3. Check USB cable quality (data cable, not charge-only)
4. Try different USB port
5. Install CP210x or CH340 drivers

### Wrong Firmware Flashed
**Problem:** Device not working after flash

**Solutions:**
1. Re-flash with correct MERGED.bin file
2. Flash at offset 0x0 (not 0x10000)
3. Use full erase: `esptool.py erase_flash`
4. Then reflash firmware

### M5Burner Can't Find Device
**Problem:** Device not detected

**Solutions:**
1. Install USB drivers (CP210x/CH340)
2. Try different USB cable
3. Press reset button
4. Check Device Manager (Windows)
5. Try `ls /dev/tty*` on Linux/Mac

## Advanced Troubleshooting

### Factory Reset
If all else fails:

```bash
# Erase entire flash
esptool.py --port /dev/ttyACM0 erase_flash

# Reflash firmware
esptool.py --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x0 M5GroqBot-v1.4-MERGED.bin
```

### Serial Monitor Debugging
View real-time logs:

```bash
# PlatformIO
pio device monitor -b 115200

# Or screen
screen /dev/ttyACM0 115200
```

Look for error messages and report them when seeking help.

### Check Memory Usage
Monitor in serial output:
- Free heap
- Free PSRAM
- Look for memory warnings

## Getting Help

If problems persist:

1. **Check GitHub Issues:** [github.com/Coreymillia/M5GroqBot/issues](https://github.com/Coreymillia/M5GroqBot/issues)
2. **Create New Issue:** Include:
   - Firmware version
   - Error messages
   - Serial monitor output
   - Steps to reproduce
3. **M5Stack Forum:** [community.m5stack.com](https://community.m5stack.com)
4. **Groq Support:** For API-related issues

## Common Error Codes

| Error | Meaning | Solution |
|-------|---------|----------|
| `Error: No API key` | API key not configured | Enter in setup |
| `Error: Connection failed` | Can't reach Groq servers | Check WiFi |
| `Error: Parse failed` | JSON parsing error | Retry/restart |
| `Error: Recording too short` | Audio < 512 bytes | Speak longer |
| `Error: Memory allocation failed` | Out of RAM | Restart device |

---

**Still having issues?** Open a GitHub issue with details!

# Getting Your Free Groq API Key

M5GroqBot requires a **free Groq API key** to function. This guide will walk you through getting your key and configuring your device.

## Step 1: Get Your Free API Key

### Create a Groq Account

1. **Open your web browser** on your computer or phone
2. **Navigate to the Groq Console:**
   ```
   https://console.groq.com/keys
   ```
3. **Sign up** for a new Groq account (or log in if you already have one)
   - The **free tier is automatically available** upon sign-up
   - No credit card required
   - Generous free usage limits

### Generate Your API Key

1. Once logged in, you'll see the **API Keys** page
2. Click the **"Create API Key"** button
3. Give your key a descriptive name (e.g., `M5GroqBot` or `M5StickC-Voice`)
4. Click **"Create"**
5. **⚠️ IMPORTANT:** Copy the key immediately!
   - The key looks like: `gsk_xxxxxxxxxxxxxxxxxxxxxxxxxxxx`
   - You won't be able to see it again after closing the dialog
   - Store it somewhere safe temporarily

## Step 2: Flash Firmware to Your Device

### Using M5Burner (Recommended)

1. **Download M5Burner** from [m5stack.com/pages/download](https://m5stack.com/pages/download)
2. **Connect** your M5StickC Plus2 via USB cable
3. **Open M5Burner** and go to the "Custom" tab
4. **Click "Add"** and select `M5GroqBot-v1.4-MERGED.bin` from the releases folder
5. **Select your device's COM port**
6. **Click "Burn"** and wait for completion (~30 seconds)

### Using Command Line (Alternative)

```bash
esptool.py --chip esp32 --port /dev/ttyACM0 --baud 1500000 \
  write_flash 0x0 M5GroqBot-v1.4-MERGED.bin
```

*Replace `/dev/ttyACM0` with your device's port (COM3 on Windows, etc.)*

## Step 3: Connect to Device WiFi Access Point

After the firmware is flashed and the device restarts:

1. **Look at the M5StickC screen** - it should display:
   ```
   WiFi Setup...
   Connect to:
   M5GroqBot-Setup
   ```

2. **On your phone or computer:**
   - Open WiFi settings
   - Look for network named: **`M5GroqBot-Setup`**
   - Connect to it (no password required)

3. **Wait for setup page** to open automatically
   - If it doesn't auto-open, manually go to: `http://192.168.4.1`

## Step 4: Configure WiFi and API Key

On the setup page you'll see:

1. **WiFi Network** dropdown
   - Select your home/office WiFi network
   - Enter the WiFi password

2. **Groq API Key** field
   - **Paste your Groq API key** here (the one you copied earlier)
   - Make sure there are no extra spaces before or after

3. **Click "Save"**
   - Device will save settings and restart
   - Takes about 5-10 seconds

## Step 5: First Use

After restart, the device will:

1. **Connect to your WiFi** automatically
2. **Display the ready screen:**
   ```
   READY!
   Ask me anything
   ```

3. **You're all set!**
   - Press **Button A** to start recording
   - Speak your question clearly for 2 seconds
   - Get instant AI responses!

## Common Issues During Setup

### Can't See WiFi AP

**Problem:** `M5GroqBot-Setup` network doesn't appear

**Solutions:**
- Wait 30 seconds after device boots
- Hold power button 6 seconds to force restart
- Check device battery level
- Move closer to device

### Setup Page Won't Open

**Problem:** Browser doesn't show configuration page

**Solutions:**
- Manually navigate to `http://192.168.4.1`
- Disable mobile data on phone
- Disable VPN if running
- Try a different browser
- Clear browser cache

### "Error: No API key" After Setup

**Problem:** Device shows error about missing API key

**Solutions:**
- Verify you copied the full key (starts with `gsk_`)
- Check for extra spaces when pasting
- Make sure key is active at console.groq.com
- Re-enter setup mode and try again

### WiFi Connection Failed

**Problem:** Device can't connect to your WiFi

**Solutions:**
- Verify WiFi password is correct
- Ensure WiFi is 2.4GHz (not 5GHz only)
- Move device closer to router
- Check if special characters in SSID/password
- Try a different WiFi network

## API Key Security Notes

⚠️ **Important Security Information:**

- **Keep your API key private** - don't share it publicly
- **Don't commit it to GitHub** or share device backups
- API key is stored in device's secure storage (ESP32 NVS)
- Only you and the device have access to it
- You can regenerate keys anytime at console.groq.com

## Usage Limits (Free Tier)

Groq's free tier (as of 2025) provides:
- Generous request limits per day
- Fast inference speeds
- Access to multiple AI models
- No credit card required

**Check current limits:** [console.groq.com/docs/rate-limits](https://console.groq.com/docs/rate-limits)

If you hit rate limits:
- Wait 60 seconds and try again
- Consider upgrading to paid tier for more requests
- Device will show error if rate limited

## Re-entering Setup Mode

If you need to change WiFi or API key later:

1. **Hold the power button** for 6+ seconds
2. Device will **restart in setup mode**
3. Connect to `M5GroqBot-Setup` AP again
4. Enter new settings

## Getting Help

- 📖 [Main README](README.md)
- 🔧 [Installation Guide](releases/INSTALLATION.md)
- 🐛 [Troubleshooting](docs/TROUBLESHOOTING.md)
- 💬 [GitHub Issues](https://github.com/Coreymillia/M5GroqBot/issues)
- 🌐 [Groq Documentation](https://console.groq.com/docs)

---

**Happy chatting with your AI assistant!** 🎤🤖

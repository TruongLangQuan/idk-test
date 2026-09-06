# M5GroqBot - Voice AI Chatbot v1.4

A standalone voice-activated AI chatbot for M5StickC Plus2, powered by Groq AI.

## Features
- 🎤 Voice-to-text transcription (Groq Whisper)
- 🤖 AI chat responses (Groq LLM)
- 📡 WiFi setup via captive portal
- 📜 Word-wrapped scrollable response viewing
- 🎨 Manual color cycling for text (8 colors)
- 🔘 Fixed button controls (tap vs hold)
- 🔋 Standalone operation (no PC required)

## Quick Start

### 1. Get a Free Groq API Key
- Visit: https://console.groq.com/keys
- Sign up for free account
- Create new API key
- Copy and save your key

### 2. Flash Firmware
- Use M5Burner to flash `M5GroqBot-v1.4-MERGED.bin`
- Or use esptool: `esptool.py write_flash 0x0 M5GroqBot-v1.4-MERGED.bin`

### 3. Setup WiFi & API Key
- On first boot, device creates WiFi AP: `M5GroqBot-Setup`
- Connect to this AP with your phone/computer
- Enter your WiFi credentials
- Paste your Groq API key
- Click Save

### 4. Use Your Chatbot!
- Device shows **READY!** with wave graphics
- **Press Button A** → Speak your question for 2 seconds
- View transcription and AI response
- **Tap Button B** = Scroll up
- **Hold Button B** = Cycle text colors (release to save)
- **Power Button** = Scroll down
- **Press PWR at end** = Reset for next question

## Controls
| Button | Action |
|--------|--------|
| **A** | Record voice question |
| **B (tap)** | Scroll up through response |
| **B (hold)** | Cycle text colors |
| **PWR** | Scroll down / Reset when at end |

## What's New in v1.4
- 🔘 **Fixed button controls** - Tap Button B to scroll, hold (0.8s) to cycle colors
- ✨ **Word wrapping** - Text no longer cuts words mid-character
- 🎨 **Manual color cycling** - 8 colors available (WHITE, CYAN, GREEN, YELLOW, MAGENTA, RED, BLUE, ORANGE)
- 🎯 **Better text layout** - Improved spacing and readability

## Technical Details
- **Platform:** ESP32 (M5StickC Plus2)
- **Audio:** 16kHz, 2-second recording
- **API:** Groq Whisper + LLM (llama-3.3-70b)
- **Memory:** PSRAM enabled
- **Network:** WiFi 2.4GHz
- **Display:** Word-wrapped text with 8 color options

## Troubleshooting

### WiFi Setup Not Appearing
- Hold power button for 6 seconds to reset device
- Device will restart and create AP again

### "Error: No API key"
- Re-enter setup mode by holding PWR button
- Re-enter your Groq API key

### Device Reboots After Response
- This is normal! Device resets after each question
- Prevents memory crashes
- Takes ~3-4 seconds to restart

### No Response from AI
- Check WiFi connection (icon on screen)
- Verify API key is valid at console.groq.com
- Ensure microphone is working (speak clearly)

## Credits
- **Author:** Coreymillia
- **Groq API:** https://groq.com
- **M5Stack:** https://m5stack.com

## Version History
- **v1.4** - Fixed button controls (tap vs hold for Button B)
- **v1.3** - Word wrapping + manual color cycling
- **v1.2** - Added wave graphics, scrollable responses
- **v1.1** - Fixed memory crashes, auto-reboot
- **v1.0** - Initial release, voice transcription working

## License
MIT License - Free to use and modify

---

**Enjoy your AI chatbot! Ask it anything!** 🎤🤖

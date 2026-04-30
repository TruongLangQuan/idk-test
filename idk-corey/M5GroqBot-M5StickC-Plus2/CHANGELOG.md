# Changelog

All notable changes to M5GroqBot project.

## [1.8] - 2024-11-13 - **STABLE RELEASE**

### ✅ Added
- Animated sine wave on ready screen with smooth scrolling effect
- Persistent color selection (saves across reboots)
- 3-second recording time (up from 2 seconds)
- User-friendly error messages ("Message not heard" instead of technical errors)
- Memory diagnostic logging (heap/PSRAM tracking)

### 🔧 Fixed
- Heap corruption crashes during recording
- Memory allocation moved to heap-only (no PSRAM for audio)
- NULL pointer checks after every free()
- Proper audioData cleanup in all error paths

### 🎨 Improved
- Smoother text scrolling
- Better button responsiveness
- More stable multi-message conversations
- Cleaner display transitions

### 📋 Known Issues
- Occasional reboots after 2-3 messages (TLS buffer leak)
- Transcription may fail with background noise

---

## [1.7] - 2024-11-13

### ✅ Added
- Manual color selection via button hold
- Boot-time settings menu (experimental)
- Color persistence across questions

### 🔧 Fixed
- Button debouncing issues
- Text wrapping problems

### ⚠️ Issues
- Settings menu conflicts with ready screen
- Boot delay too long (10 seconds)

### 📝 Notes
- This version was replaced quickly by 1.8
- Boot menu removed in favor of simpler UX

---

## [1.6] - 2024-11-13

### ✅ Added
- Scrollable text display for long responses
- Color cycling for text (auto-rotating)
- Button hold detection

### 🔧 Fixed
- Display overflow issues
- Text cutoff problems

---

## [1.5] - 2024-11-13

### ✅ Added
- Chat history persistence (NVS storage)
- Conversation context (last 5 exchanges)
- WiFi Manager integration

### 🔧 Fixed
- WiFi connection stability
- API key storage

---

## [1.4] - 2024-11-12

### ✅ Added
- Basic Groq Whisper transcription
- Groq chat integration (LLaMA 3.3 70B)
- Voice recording with I²S microphone

### 🔧 Fixed
- Audio capture timing
- WAV header generation

### 📝 Notes
- First working version with full voice → chat pipeline
- 2-second recording time

---

## [1.3] - 2024-11-12

### ✅ Added
- Initial WiFi setup
- Basic display functions
- Button handling

---

## [1.0 - 1.2] - 2024-11-11

### 📝 Notes
- Early experimental versions
- Testing hardware components
- Proof of concept

---

## 🚀 Upcoming (v1.9+ - In Development)

### Planned Features
- ⏳ Settings menu without boot delay
- ⏳ WiFi reconnect without reflash
- ⏳ Recording time adjustment (2-5 sec)
- ⏳ Face animation from Social Parasite project
- ⏳ TTS output via speaker
- ⏳ Memory leak fixes for stable 10+ message sessions

### Under Investigation
- TLS buffer management improvements
- PSRAM utilization for larger buffers
- Multi-language support
- Custom wake word

---

## 📦 Development Bins

Located in `development-bins/` folder:

- `M5GroqBot_v1.9_BOOT_MENU.bin` - Experimental settings menu during boot
- `M5GroqBot_v1.9_TLS_FIX_LATEST.bin` - Latest TLS fixes (unstable)

⚠️ **Use at your own risk** - these are development builds and may be unstable.

---

## 🐛 Bug Reports

Found a bug? Please report:
1. Version number
2. What you were doing
3. Serial monitor output (if possible)
4. Expected vs actual behavior

Submit to: [GitHub Issues](https://github.com/YourUsername/M5StickCPlus2-GroqBot/issues)

---

**Legend:**
- ✅ Added - New features
- 🔧 Fixed - Bug fixes
- 🎨 Improved - Enhancements
- ⚠️ Issues - Known problems
- 📝 Notes - Additional info
- ⏳ Planned - Coming soon

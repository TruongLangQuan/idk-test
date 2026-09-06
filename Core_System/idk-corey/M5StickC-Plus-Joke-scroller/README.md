# M5 Sarcastic Joke Scroller - NON Portal Hater Edition

## 🎭 **The Ultimate M5StickC Entertainment Device**

**Transform your M5StickC into a premium sarcastic entertainment device with 3,542 hilariously brutal jokes, random auto-scroll, cycling font colors, and perfect readability!**

![M5StickC](https://img.shields.io/badge/Device-M5StickC%20Plus-brightgreen)
![Jokes](https://img.shields.io/badge/Jokes-3,542-blue)
![Universal](https://img.shields.io/badge/Firmware-Universal-orange)
![Version](https://img.shields.io/badge/Version-2.0.0-red)

---

## ⚡ **Quick Start with M5Burner**

### **🔥 One-Click Installation**
1. **Download** [M5Burner](https://m5stack.com/pages/download)
2. **Flash** `releases/v2.0-Final/M5Burner_Package/M5Jokes_Universal_M5StickC-v2.0-COMPLETE-MERGED.bin`
3. **Enjoy** 3,542 sarcastic jokes instantly!

**✅ Universal Firmware** - Works on both Plus1.1 and Plus2!

---

## 🎮 **Controls**

| Button | Function |
|--------|----------|
| **A** | Next joke → |
| **B** | Toggle random auto-scroll |
| **C** | ← Previous joke |
| **Power** | Cycle font colors 🌈 |

---

## ✨ **Premium Features**

### 🎲 **Smart Random Auto-Scroll**
- **Random joke selection** every 5 seconds
- **Never repeats** same joke consecutively  
- **True hardware randomness**
- Toggle ON/OFF with Button B

### 🌈 **10 Beautiful Font Colors**
- **Cycle through:** White, Cyan, Yellow, Green, Magenta, Red, Orange, Lime, Purple, Pink
- **Visual feedback:** Screen flashes in new color
- **Persistent memory:** Remembers your favorite

### 📖 **Optimized Reading Experience**
- **Perfect font size** (size 2) for long jokes
- **Smart word wrapping** for 240x135 display
- **Clean UI** with joke counter and controls
- **3,542 unique jokes** - hours of entertainment

### 💾 **Memory Persistence**
- **Remembers** last joke position
- **Saves** preferred font color
- **No setup** needed after reboot

---

## 🎭 **Sample Jokes**

> *"Your brain has buffering issues."*

> *"You're the human equivalent of a loading screen that never finishes."*

> *"I bet you ask GPS for confirmation even when you know the way."*

> *"You scroll like your soul depends on it."*

**And 3,538 more equally brutal and hilarious one-liners!**

---

## 🔧 **Technical Specs**

- **Memory Usage:** 7.7% RAM (25KB), 49.9% Flash (654KB)
- **Display:** 240x135 with optimized rendering
- **Font:** Size 2 with 16px line height
- **Storage:** PROGMEM for reliable joke access
- **Compatibility:** Universal Plus1.1 & Plus2 firmware
- **Randomization:** Hardware-seeded true randomness

---

## 🚀 **Perfect For**

- ☕ **Coffee breaks** - Random entertainment while you sip
- ⏰ **Waiting rooms** - Offline entertainment anywhere
- 😂 **Ice breakers** - Share brutal jokes with friends  
- 🎯 **Tech demos** - Show off advanced M5StickC features
- 🎁 **Gifts** - Ultimate geek entertainment device

---

## 📦 **Installation Options**

### **Option 1: M5Burner (Recommended)**
```bash
# Download and use M5Burner GUI
File: releases/v2.0-Final/M5Burner_Package/M5Jokes_Universal_M5StickC-v2.0-COMPLETE-MERGED.bin
Address: 0x0
```

### **Option 2: ESPTool Command Line**
```bash
esptool --port /dev/ttyACM0 --baud 921600 write_flash 0x0 M5Jokes_Universal_M5StickC-v2.0-COMPLETE-MERGED.bin
```

### **Option 3: Build from Source**
```bash
git clone https://github.com/coreymillia/M5Jokes
cd M5Jokes
pio run --target upload
```

---

## 🏆 **What Makes This Special**

✅ **Complete Collection** - All 3,542 Portal Hater jokes  
✅ **Universal Firmware** - One binary for Plus1.1 and Plus2  
✅ **Advanced Features** - Random mode, colors, memory  
✅ **Perfect UX** - Optimized font and smart wrapping  
✅ **Professional Code** - Clean architecture and compatibility layer  
✅ **M5Burner Ready** - Professional package with JSON config  

---

## 🛠️ **Development**

### **Architecture**
- **Universal Compatibility Layer** - Works seamlessly on both M5StickC variants
- **Efficient PROGMEM Storage** - All jokes stored in flash memory
- **Smart Random Algorithm** - True hardware-seeded randomization
- **Persistent Preferences** - NVS storage for user settings

### **Memory Optimization**
- **Jokes:** Stored efficiently in PROGMEM
- **Code:** Minimal RAM footprint (25KB)
- **Settings:** Compressed preferences storage
- **Display:** Optimized rendering pipeline

### **Build Requirements**
- **PlatformIO** with ESP32 platform
- **M5Unified** library (auto-installed)
- **ESP32 Arduino** framework 2.0.11+

---

## 📁 **Project Structure**

```
M5Jokes/
├── src/
│   ├── main.cpp           # Main application logic
│   ├── jokes.cpp          # Complete joke collection
│   └── jokes.h            # Joke array declarations
├── releases/
│   └── v2.0-Final/
│       └── M5Burner_Package/
│           ├── M5Jokes_Universal_M5StickC-v2.0-COMPLETE-MERGED.bin
│           ├── m5burner_config.json
│           └── README.md
├── platformio.ini         # Build configuration
└── README.md             # This file
```

---

## 🎉 **Version History**

- **v2.0.0** - Complete collection with random auto-scroll, color cycling, perfect fonts
- **v1.3.0** - Final font size optimization for long jokes
- **v1.2.0** - Perfect font size testing iterations
- **v1.1.0** - Double font size experiments
- **v1.0.0** - Initial M5StickC port from CYD project

---

## 🏅 **Achievement Unlocked**

**First complete sarcastic joke collection with advanced features for M5StickC!** 

Perfect blend of humor, technology, and user experience. From CYD touch interface to M5StickC button controls in record time, with professional packaging and M5Burner compatibility.

---

## 📸 **Screenshots**

*Device displaying colorful sarcastic jokes with perfect readability and clean UI*

---

## 🤝 **Contributing**

Want to add more jokes or features? 
1. Fork the repo
2. Add your jokes to `jokes.cpp`
3. Test on hardware  
4. Submit PR with humor intact!

---

## 📄 **License**

Open source for maximum sarcasm distribution. Use responsibly (or not).

---

## 👨‍💻 **Credits**

**Created by:** coreymillia + GitHub Copilot CLI  
**Achievement:** Fastest CYD→M5StickC conversion ever (under 20 minutes!)  
**Special Thanks:** The M5Stack community and everyone who appreciates good sarcasm  

---

**Ready to laugh?** Flash the firmware and join the 3,542-joke sarcasm revolution! 🎭✨

*Perfect entertainment for developers, makers, and anyone with a sense of humor.*
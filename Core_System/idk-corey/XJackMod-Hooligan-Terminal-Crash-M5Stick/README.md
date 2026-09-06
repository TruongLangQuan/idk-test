# XJack Mod - 13 Effects Christmas Edition

A collection of 13 terminal-based screensaver effects for M5StickC Plus2, inspired by classic XScreensaver and featuring original content including Jesus Hooligan song lyrics.

## 🎄 Christmas Eve 2025 Release

**Version:** 3.0.0  
**Build:** 20251224_135830  
**Platform:** M5StickC Plus2 (ESP32-PICO-V3-02)  
**Flash Size:** 564KB merged binary

## 🎮 Controls

- **Button A** - Cycle through all 13 effects
- **Button B** - Toggle auto-scroll mode (15-second intervals)
- **Power Button** - Toggle title display on/off

## 🖥️ The 13 Screensaver Effects

### 1. Original XJack
Classic "All work and no play makes Jack a dull boy" typing horror with psychological tension effects and typewriter strikes.

### 2. Back Room
Jesus Hooligan's haunting song about death and the back room. Features verse-by-verse display with atmospheric glitch effects.

### 3. Dogs & Motorbikes  
Jesus Hooligan's misanthrope anthem "I like dogs, motorbikes, and three people" - a declaration of independence from human stupidity.

### 4. System Error ⭐
**Fan favorite!** Fast escalating system failure progression: Warning → Critical → Cascade → Meltdown → Reboot. Features intense visual corruption and flashing alerts.

### 5. Symbol Storm
ASCII character chaos with rainbow color cycling, typing animation effects, and floating particles across multiple symbol sets.

### 6. Terminal Hacker
Realistic coding session that compiles and runs a matrix rain program. Features authentic terminal commands and pure matrix execution phase.

### 7. Hacked Terminal
Normal work session gets hijacked by a hacker. Starts with innocent document editing, escalates to root compromise and system destruction.

### 8. Xtreme Hack
7 different educational hacker scenarios cycling every 30 seconds: filesystem corruption, code injection, encryption traps, psychological mind leaks, and more.

### 9. Kernel Panic
System attempts recovery multiple times, fails catastrophically, shows human panic moment, then freezes forever with blinking cursor.

### 10. Crash Grammar
Role-based system failure with different system components (SYS, KERNEL, SEC, IO, WATCHER, HUMAN, PANIC) experiencing escalating failures and memory decay.

### 11. Vibe Code Fail ✨
**The masterpiece!** AI coding assistant creates a flight simulator that progressively gets worse through **6 increasingly desperate fix attempts**:
- GPU optimization → "Flying rectangle!"
- Emergency patch → "Upside down cockpit, purple sky, Comic Sans instruments!"  
- AI enhancement → "Flying toaster with cheese wheels!"
- Emergency rollback → "Giant banana flying backwards with cow in cockpit!"
- Machine learning → "Flying Rubik's cube on spaghetti runway!"
- Quantum renderer → **"17-dimensional plane where you can taste colors!"**

Ends with complete OS deletion when user finally snaps.

### 12. Net Apocalypse 🌐
XScreensNet infrastructure collapse from normal network monitoring to complete digital apocalypse. Features realistic network terminology and escalating visual corruption.

### 13. BIOS Catastrophe 💻
Hardware failure from normal POST sequence to complete motherboard meltdown. Features temperature escalation, component failures, and maximum screen glitching as hardware literally dies.

## 🚀 Installation

### Quick Flash (Recommended)
```bash
esptool write_flash 0x0 XJackMod_13Effects_Christmas_20251224_135830.bin
```

### Development Setup
```bash
git clone <repository>
cd XjackMod
pio run --target upload
```

## 🎵 Credits

- **Songs:** Jesus Hooligan ("Back Room", "Dogs & Motorbikes")
- **XScreensaver Inspiration:** Jamie Zawinski  
- **Creator:** Coreymillia
- **Date:** Christmas Eve 2025

## 🎨 Features

- ✅ 13 unique screensaver effects
- ✅ Fast escalating system failures  
- ✅ Realistic terminal simulations
- ✅ Color-coded phase progression
- ✅ Auto-scroll mode with 15-second intervals
- ✅ Visual corruption and glitch effects
- ✅ ASCII art and symbol chaos
- ✅ Song lyrics with verse breaks
- ✅ Hacker invasion scenarios
- ✅ AI coding disaster with 6 fix attempts
- ✅ Network infrastructure collapse
- ✅ Hardware meltdown simulation

## 🔧 Technical Details

- **Arduino Framework:** ESP32 Arduino 2.0.17
- **PlatformIO:** 6.1.13
- **Firmware Size:** 498,880 bytes
- **Merged Binary Size:** 564,416 bytes
- **Flash Layout:**
  - 0x1000: Bootloader
  - 0x8000: Partition table
  - 0xe000: Boot app
  - 0x10000: Application firmware

## 🎄 Perfect for Christmas!

Whether you want psychological horror (Original XJack), musical poetry (Jesus Hooligan songs), system failure chaos (System Error, BIOS Catastrophe), or hilarious AI disasters (Vibe Code Fail), this collection has something to terrify and entertain everyone during the holidays.

**Merry Christmas!** 🎅🎁

*Built with rage and dabs on Christmas Eve 2025*
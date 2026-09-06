# 🔥 **M5Burner Installation Guide - Quindectris v2.2**

## 📦 **M5Burner Package Contents**

### **Files Included:**
- `Quindectris-v2.2-M5BURNER-MERGED-20251201.bin` (504KB) - **Complete firmware with bootloader**
- `Quindectris-v2.2-M5BURNER-config.json` - **M5Burner configuration**
- `M5BURNER-INSTALLATION.md` - **This installation guide**

---

## 🚀 **Installation Steps**

### **Method 1: Direct M5Burner Installation**
1. **Download M5Burner** from https://m5stack.com/pages/download
2. **Connect your M5StickC Plus2** via USB
3. **Load the firmware:**
   - Click "Load" in M5Burner
   - Select `Quindectris-v2.2-M5BURNER-MERGED-20251201.bin`
   - Click "Burn" to flash
4. **Attach mini joystick** to HAT connector
5. **Play Quindectris!** 🎮

### **Method 2: Command Line Installation**
```bash
esptool --chip esp32 --port /dev/ttyACM0 --baud 1500000 write_flash 0x0 Quindectris-v2.2-M5BURNER-MERGED-20251201.bin
```

---

## 🎮 **Game Features**

### **🎯 15 Unique Pieces:**
- **8 Triomino pieces** (1-3 blocks) for precision placement
- **7 Classic Tetromino pieces** (4 blocks) for power moves
- **Perfect clockwise rotation** - no morphing issues!

### **🔄 Advanced Mechanics:**
- **Hold system** - store pieces for later
- **Ghost pieces** - see where pieces land
- **Wall kick system** - smart collision handling
- **Lock delay** - time to make final adjustments

### **🎨 Visual Polish:**
- **"QUIN15" title screen** with animated pieces
- **Cyan game border** for clear boundaries
- **Colorful piece system** for easy identification

---

## 🕹️ **Controls (Mini Joystick)**

### **Movement:**
- **↑ Up** - Move piece right
- **↓ Down** - Move piece left  
- **← Left** - Hard drop (instant down)
- **→ Right** - Soft drop (faster down)
- **🎯 Center Press** - Rotate piece clockwise

### **Advanced:**
- **Hold Left + Center** - Hold current piece
- **Ghost piece** shows landing position automatically

---

## 📊 **Technical Specifications**

### **Hardware Requirements:**
- **M5StickC Plus2** (ESP32-PICO-V3-02)
- **Mini Joystick HAT** (connects to HAT port)
- **USB-C cable** for power/programming

### **Memory Usage:**
- **RAM**: 8.4% (27,488 bytes)
- **Flash**: 34.1% (446,944 bytes)  
- **Performance**: 60fps smooth gameplay

### **Build Information:**
- **Platform**: ESP32 Arduino Framework
- **Bootloader**: ESP32 DIO 40MHz
- **Partition scheme**: Default (4MB)
- **Version**: v2.2 FINAL PERFECT

---

## 🎊 **About Quindectris**

**Quindectris** is the ultimate evolution of block-puzzle gaming, featuring the most diverse piece set ever created:

- **Evolved from ThreeTris** - our acclaimed 8-piece triomino game
- **15 total pieces** ranging from single blocks to full tetrominoes
- **Perfect rotation mechanics** - every piece spins flawlessly
- **Strategic depth** - micro-management meets power moves

### **🏆 "From Single Blocks to Tetrominoes - Every Piece Changes the Game!"**

---

## 📞 **Support & Links**

- **Source Code**: https://github.com/coreymillia/quindectris
- **Documentation**: Full README with development notes
- **License**: Open source project
- **Developer**: CoReMiLLiA Games

### **🎮 Enjoy the Ultimate Mixed-Block Tetris Experience!** 🚀
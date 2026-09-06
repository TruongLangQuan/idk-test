# M5StickVectrex - Quick Start

## 🎯 Current Status
✅ **Phase 1 Complete** - Project builds successfully  
📋 **Phase 2 Next** - Display adaptation (1-2 hours)

---

## 🚀 Build & Flash

```bash
cd /home/coreymillia/Documents/Coremaddor64/M5StickVectrex
pio run                    # Build
pio run --target upload    # Flash to M5StickC Plus2
pio device monitor         # View serial output
```

---

## 📝 What to Do Next

### Option 1: Read First (Recommended)
1. Read `PHASE2_IMPLEMENTATION.md` - Detailed guide
2. Read `STATUS.md` - Current state
3. Follow implementation steps

### Option 2: Jump In
1. Create `src/display.h` and `src/display.cpp`
2. Copy code from PHASE2_IMPLEMENTATION.md
3. Modify `src/main.cpp` and `src/osint.cpp`
4. Build, flash, test!

---

## 🎮 What You're Building

**Vectrex emulator** - 1982 vector graphics game console on M5StickC Plus2

- 71 games available (Mine Storm built-in)
- Vector graphics → LCD conversion
- Target: 20-30 FPS, playable controls

---

## 📊 Key Numbers

- **Flash:** 303KB used / 2MB total (14.5%)
- **RAM:** 86KB used / 320KB total (26.2%)
- **Framebuffer:** 64KB (240×135 RGB565)
- **Games:** 71 ROMs ready

---

## 🔧 Quick File Reference

| File | Purpose |
|------|---------|
| `src/main.cpp` | Setup & main loop |
| `src/e6809.cpp` | CPU emulator ✅ |
| `src/vecx.cpp` | Vectrex system ✅ |
| `src/osint.cpp` | Platform interface ⚠️ |
| `src/display.h` | Framebuffer API 📝 NEW |
| `src/display.cpp` | Framebuffer impl 📝 NEW |

---

## 🎯 Phase 2 Goals

1. Create framebuffer system
2. Replace DAC with LCD
3. See Vectrex BIOS logo on M5 screen
4. Time estimate: 1-2 hours

---

## 📚 Full Documentation

- `README.md` - Overview & architecture
- `PROJECT_PLAN.md` - Complete 6-phase plan
- `PHASE2_IMPLEMENTATION.md` - Display rewrite guide ⭐
- `STATUS.md` - Detailed current state

---

**Ready? See PHASE2_IMPLEMENTATION.md for step-by-step guide!** 🎮

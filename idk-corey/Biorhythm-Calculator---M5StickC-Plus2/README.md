# 📊 M5StickC Plus2 Biorhythm Calculator

## 🎯 Project Status: ✅ FUNCTIONAL PROTOTYPE COMPLETE!

A fully working biorhythm calculator that displays physical, emotional, and intellectual cycles as sine waves on your M5StickC Plus2.

## 🚀 **What Works Now:**
- **✅ Complete biorhythm calculation engine** 
- **✅ Birth date input and storage** (saved to EEPROM)
- **✅ Multi-screen UI** with navigation
- **✅ Real-time biorhythm values** with color-coded percentages
- **✅ Mini graph display** showing all three cycles
- **✅ Detailed graph view** with 30-day curves
- **✅ Date navigation** (scroll through days)
- **✅ Battery display** 

## 🧮 **Biorhythm Math Implemented:**
```cpp
Physical Cycle    = sin(2π × days_alive / 23)   // 23 days - RED
Emotional Cycle   = sin(2π × days_alive / 28)   // 28 days - YELLOW  
Intellectual Cycle = sin(2π × days_alive / 33)  // 33 days - CYAN

// Where days_alive = current_date - birth_date
```

## 🎮 **Controls:**
- **Button A**: Navigate between screens / Select field
- **Button B**: Increment values / Next day

## 📱 **Screens:**
1. **Welcome Screen** - Shows status and battery
2. **Birth Date Input** - Set your birth date (saved permanently)
3. **Biorhythm Display** - Current values + mini graphs
4. **Graph View** - Detailed 30-day curves for each cycle

## 🎨 **Display Features:**
- **Color-coded values**: Green (high+), Yellow (low+), Orange (low-), Red (high-)
- **Mini timeline graphs** showing all cycles together
- **Detailed individual curves** in graph mode
- **Battery percentage** always visible
- **Date navigation** to explore past/future

## 🔧 **Installation:**
1. Connect M5StickC Plus2 to computer
2. Run: `pio run --target upload`
3. Set your birth date on first run
4. Enjoy your biorhythms!

## 📈 **Example Display:**
```
┌─────────────────┐
│   BIORHYTHM     │
│   2024/12/10    │
│ Days alive: 12707│
├─────────────────┤
│ Physical:  +85% │ ← GREEN (strong)
│ Emotional: -23% │ ← ORANGE (low neg)
│ Intellect: +67% │ ← YELLOW (moderate)
├─────────────────┤
│ ∿∿∿∿∿∿∿∿∿∿∿∿∿ │ ← Real sine waves
│ Controls: A=Graph B=Next
└─────────────────┘
```

## 🎊 **SUCCESS!** 
This is exactly what we set out to build - a complete biorhythm calculator using the display functions from your Halloween Tetris project and the mathematical formulas you researched. The M5StickC Plus2 is perfect for this kind of personal data visualization!

**Have fun exploring your biorhythms! 📈✨**
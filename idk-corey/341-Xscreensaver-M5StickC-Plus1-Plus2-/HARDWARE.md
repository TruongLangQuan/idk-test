# 🔧 Hardware Setup Guide

Complete hardware setup instructions for the ESP32 Screensaver Universe project.

## 📋 **Hardware Requirements**

### **Core Components**

| Component | Specification | Notes |
|-----------|--------------|-------|
| **Microcontroller** | ESP32-D0WD-V3 | Any ESP32 variant works |
| **Display** | 2.8" ILI9341 TFT | 320x240 resolution |
| **Touch Controller** | XPT2046 | Resistive touch |
| **Memory** | 4MB Flash, 320KB RAM | Standard ESP32 specs |
| **Power Supply** | 3.3V/5V | USB or external |

### **Recommended Development Boards**

#### **🌟 ESP32-2432S028 (Recommended)**
- **All-in-one solution** with integrated display and touch
- **Built-in components**: ESP32, 2.8" TFT, touch controller, microSD
- **Price**: ~$15-20 USD
- **Advantage**: No wiring required, ready to use

#### **🔧 Custom ESP32 + Display Setup**
- **ESP32 DevKit** + **ILI9341 Display Module**
- **More flexibility** for custom projects
- **Price**: ~$10-15 USD total
- **Advantage**: Educational, customizable

## 🔌 **Pin Connections**

### **ESP32-2432S028 (Pre-wired)**
If using the ESP32-2432S028 board, all connections are pre-made. Just upload the code!

### **Custom Wiring (ESP32 DevKit + ILI9341)**

#### **TFT Display (ILI9341) Connections**
```
ESP32 Pin → ILI9341 Pin
------------------------
3.3V      → VCC
GND       → GND
GPIO 15   → CS (Chip Select)
GPIO 2    → DC (Data/Command)
GPIO 13   → MOSI (SDA)
GPIO 14   → SCK (SCL)
GPIO 12   → MISO
EN        → RST (Reset)
```

#### **Touch Controller (XPT2046) Connections**
```
ESP32 Pin → XPT2046 Pin
-----------------------
GPIO 33   → CS (T_CS)
GPIO 36   → IRQ (T_IRQ)
GPIO 13   → MOSI (T_DIN)
GPIO 14   → CLK (T_CLK)
GPIO 12   → MISO (T_DO)
```

#### **Optional: MicroSD Card**
```
ESP32 Pin → SD Card Pin
----------------------
GPIO 5    → CS
GPIO 13   → MOSI
GPIO 14   → CLK
GPIO 12   → MISO
```

## ⚡ **Power Requirements**

### **Power Specifications**
- **Operating Voltage**: 3.3V (ESP32) / 3.3V-5V (Display)
- **Current Consumption**: 
  - ESP32: ~240mA (active), ~10µA (deep sleep)
  - Display: ~100mA (backlight on), ~20mA (backlight off)
  - **Total**: ~340mA maximum
- **Power Source**: USB (5V) or 3.3V regulated supply

### **Power Supply Options**

#### **🔌 USB Power (Recommended)**
```
USB 5V → ESP32 Development Board → 3.3V Regulator → Components
```
- Most convenient for development and permanent installation
- Stable power with good current capability
- Built-in voltage regulation on most dev boards

#### **🔋 Battery Power**
```
Li-Po Battery (3.7V) → ESP32 → Components
```
- **Recommended**: 18650 Li-ion battery (2000mAh+)
- **Runtime**: 4-6 hours typical usage
- **Add**: Battery management circuit for safety

#### **🏠 Wall Adapter**
```
Wall Adapter (5V 1A) → ESP32 Development Board
```
- Best for permanent installations
- Provides stable, high-current power
- Use quality adapters to avoid noise

## 🛠️ **Assembly Instructions**

### **Method 1: ESP32-2432S028 (Plug & Play)**

1. **Unbox the ESP32-2432S028**
2. **Connect USB cable**
3. **Upload the code** using PlatformIO
4. **Done!** - No assembly required

### **Method 2: Custom Assembly**

#### **Step 1: Prepare the ESP32**
1. Flash the ESP32 with the screensaver firmware
2. Test basic functionality with serial monitor
3. Verify pin assignments match your board variant

#### **Step 2: Connect the Display**
1. **Wire the TFT display** according to pin diagram above
2. **Double-check connections** - incorrect wiring can damage components
3. **Test display** with a simple graphics test

#### **Step 3: Add Touch Controller**
1. **Wire the touch controller** following the pin diagram
2. **Test touch response** with calibration code
3. **Adjust sensitivity** if needed

#### **Step 4: Final Assembly**
1. **Secure all connections** with proper connectors or soldering
2. **Test all functions** - display, touch, effects
3. **Mount in enclosure** if desired

## 🧪 **Testing & Calibration**

### **Display Test**
```cpp
// Basic display test code
#include <Arduino_GFX_Library.h>

Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(2, 14, 13, 15, GFX_NOT_DEFINED, 40000000);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, GFX_NOT_DEFINED, 0, false);

void setup() {
  gfx->begin();
  gfx->fillScreen(RED);
  delay(1000);
  gfx->fillScreen(GREEN);
  delay(1000);
  gfx->fillScreen(BLUE);
}
```

### **Touch Calibration**
```cpp
// Touch test code
#include <XPT2046_Touchscreen.h>

XPT2046_Touchscreen ts(33, 36);  // CS, IRQ pins

void setup() {
  Serial.begin(115200);
  ts.begin();
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.printf("Touch: X=%d, Y=%d\n", p.x, p.y);
  }
}
```

### **Performance Test**
Upload the full screensaver code and verify:
- **All 341 effects** display correctly
- **Smooth 20Hz framerate** maintained
- **Touch response** works reliably
- **No memory errors** or crashes

## 🔍 **Troubleshooting**

### **Display Issues**

#### **Blank/White Screen**
- ✅ Check power connections (3.3V to display)
- ✅ Verify SPI pin connections (MOSI, CLK, CS, DC)
- ✅ Ensure correct pin definitions in code
- ✅ Try different SPI speed (reduce from 40MHz to 20MHz)

#### **Garbled Display**
```cpp
// Try reducing SPI speed
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(2, 14, 13, 15, GFX_NOT_DEFINED, 20000000);
```

#### **Inverted Colors**
```cpp
// Add display inversion
gfx->invertDisplay(true);
```

### **Touch Issues**

#### **No Touch Response**
- ✅ Check touch controller wiring (CS, IRQ pins)
- ✅ Verify touch controller power (3.3V)
- ✅ Test with touch calibration code
- ✅ Check for loose connections

#### **Inaccurate Touch**
```cpp
// Adjust touch mapping
#define TOUCH_MAP_X1 200
#define TOUCH_MAP_X2 3700
#define TOUCH_MAP_Y1 240  
#define TOUCH_MAP_Y2 3800
```

### **Power Issues**

#### **Random Resets**
- ✅ Check power supply current capability (need 500mA+)
- ✅ Add decoupling capacitors (100µF near ESP32)
- ✅ Use shorter, thicker power wires
- ✅ Avoid USB ports with insufficient current

#### **Display Flickering**
- ✅ Improve power supply filtering
- ✅ Check for loose connections
- ✅ Reduce display brightness if battery powered
- ✅ Add ferrite beads on power lines

### **Performance Issues**

#### **Slow/Choppy Animation**
- ✅ Check CPU frequency (should be 240MHz)
- ✅ Reduce effect complexity for testing
- ✅ Monitor serial output for error messages
- ✅ Verify adequate free heap memory

## 📐 **Mechanical Considerations**

### **Mounting Options**

#### **Desktop Stand**
- 3D print or laser cut a simple stand
- Angle for optimal viewing (15-30 degrees)
- Include access to USB port for power

#### **Wall Mount**
- VESA-style mounting bracket
- Consider heat dissipation
- Cable management for clean appearance

#### **Enclosure Design**
- Ventilation for heat management
- Access to programming ports
- Clear window for display
- Touch accessibility

### **Thermal Management**
- ESP32 generates ~500mW heat under full load
- Display backlight adds ~300mW
- Ensure adequate airflow for continuous operation
- Consider heat sinks for high ambient temperatures

## 🛒 **Parts List & Suppliers**

### **Core Components**

| Part | Quantity | Approximate Cost | Supplier |
|------|----------|------------------|----------|
| ESP32-2432S028 Board | 1 | $15-20 | AliExpress, Amazon |
| OR ESP32 DevKit | 1 | $5-8 | Various |
| OR ILI9341 2.8" TFT | 1 | $8-12 | Various |
| USB Cable (Micro/Type-C) | 1 | $3-5 | Various |

### **Optional Components**

| Part | Purpose | Cost |
|------|---------|------|
| Enclosure/Case | Protection | $5-15 |
| SD Card (16GB+) | Future expansion | $5-10 |
| Breadboard/PCB | Prototyping | $2-5 |
| Jumper Wires | Connections | $2-5 |

### **Tools Required**
- **Soldering iron** (if making custom connections)
- **Multimeter** (for troubleshooting)
- **Computer** with USB port
- **PlatformIO** or Arduino IDE

## 🔧 **Advanced Modifications**

### **Display Upgrades**
- **Larger displays**: 3.5" or 4.3" TFT modules
- **Higher resolution**: 480x320 displays (requires more memory)
- **OLED displays**: Better contrast, different interface requirements

### **Performance Enhancements**
- **ESP32-S3**: More RAM, better graphics performance
- **PSRAM**: External memory for complex effects
- **DMA**: Hardware acceleration for SPI transfers

### **Additional Features**
- **Audio output**: Add speaker for sound effects
- **WiFi connectivity**: Network-based effects or control
- **Sensors**: Accelerometer, light sensor for interactive effects
- **RGB LEDs**: Ambient lighting synchronized with effects

---

## 💡 **Pro Tips**

1. **Start Simple**: Begin with the ESP32-2432S028 for easiest setup
2. **Test Early**: Verify each connection before final assembly  
3. **Quality Components**: Use good quality jumper wires and connectors
4. **Power First**: Ensure adequate power supply before troubleshooting other issues
5. **Document Changes**: Keep notes of any modifications for future reference

**Happy building! Your 341-effect screensaver awaits!** 🚀✨
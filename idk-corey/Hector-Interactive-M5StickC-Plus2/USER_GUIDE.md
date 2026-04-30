# User Guide - Hector Interactive v4.0

## 🎮 Getting Started

### First Boot
1. Power on your M5StickC Plus2
2. Device will display loading screen then start in **FLAT** mode
3. You'll see a flat grid with "FLAT" in the top-right corner

### Basic Controls
- **Button A** (side): Cycle display styles
- **Button B** (front): Cycle interactive modes

## 🎨 Display Styles (Button A)

Press Button A to cycle through visual rendering styles:

### 1. 🕸️ GRID (Default)
- Shows wireframe mesh lines
- Classic mathematical grid visualization
- Best for seeing surface structure

### 2. 🔺 SOLID  
- Filled triangular surface patches
- Creates solid 3D appearance
- Great for depth perception

### 3. 🦓 ZEBRA
- Alternating stripe patterns
- Shows wave propagation clearly
- Artistic striped effect

### 4. ⬛ CHECKBOARD
- Alternating filled squares with connecting lines
- Unique dotted pattern style
- Good for pattern analysis

## 🌟 Interactive Modes (Button B)

Press Button B to cycle through 10 different mathematical experiences:

### 1. 🏁 FLAT - Baseline Reference
- **What it shows**: Perfect flat horizontal grid
- **Purpose**: Reference point for comparison
- **Best with**: Any display style to see pure grid

### 2. 🎯 TILT - Motion Control  
- **What it shows**: Surface tilts and moves with device motion
- **Try this**: Tilt device forward/back, left/right, rotate
- **Magic**: Real MPU6886 IMU data controls the surface
- **Best with**: Grid or Solid styles

### 3. 🎵 SOUND - Audio Visualization
- **What it shows**: Waves created by microphone input
- **Try this**: Speak, clap, snap fingers, play music
- **Debug info**: Shows audio level (0.00-0.80) and power
- **Silent room**: Flat surface (0.00 level)
- **Active sound**: Responsive waves (0.10-0.80 level)
- **Best with**: Any style, Grid shows waves clearly

### 4. 🌀 SPIRAL - Mathematical Spirals
- **What it shows**: Rotating spiral patterns
- **Experience**: Hypnotic mathematical beauty
- **Math**: Polar coordinate transformations
- **Best with**: Grid or Zebra for spiral structure

### 5. 🌊 INTERFERENCE - Wave Physics
- **What it shows**: Multiple wave sources creating interference
- **Experience**: Complex overlapping patterns
- **Math**: Wave superposition and interference
- **Best with**: Solid or Grid to see pattern complexity

### 6. ⛰️ MOUNTAIN - Landscape Generation
- **What it shows**: Dynamic mountain range terrain
- **Experience**: Organic landscape that evolves
- **Math**: Multiple sine/cosine layers with noise
- **Best with**: Solid for realistic landscape feel

### 7. 💧 RIPPLE - Multi-Source Ripples
- **What it shows**: Multiple water droplet sources
- **Experience**: Realistic ripple tank simulation
- **Math**: Multiple point source wave equations
- **Best with**: Grid to see ripple propagation

### 8. ⚡ PLASMA - Energy Field
- **What it shows**: Flowing plasma-like energy patterns
- **Experience**: Dynamic energy visualization
- **Math**: Multi-frequency plasma equations
- **Best with**: Solid or Zebra for energy flow

### 9. 📊 SINE - Classic Waves (Original)
- **What it shows**: Pure mathematical sine waves
- **Experience**: Classic ESP32-Hector beauty
- **Math**: Traditional sine wave mathematics
- **Best with**: Grid for mathematical purity

### 10. 💧 DRIP - Water Droplets (Original)
- **What it shows**: Water droplet ripple effects
- **Experience**: Organic water simulation
- **Math**: Radial wave equations with decay
- **Best with**: Any style, especially Grid

## 🎯 Pro Tips

### Getting the Best Experience
1. **Start with FLAT** to understand the baseline
2. **Try SOUND mode** in a quiet room, then make noise
3. **Use TILT mode** while moving the device around
4. **Combine modes with different styles** (40 total combinations!)

### Audio Mode Tips
- **Quiet room**: Should show 0.00 and flat surface
- **Normal speech**: Look for 0.10-0.30 levels
- **Clap/snap**: Should spike to 0.40-0.80
- **Music**: Continuous wave response matching beats

### Motion Mode Tips
- **Gentle movements**: Small surface changes
- **Tilting**: Surface follows device orientation
- **Rotation**: Creates spinning effects
- **Acceleration**: Affects wave intensity

## 📊 Understanding the Display

### Status Indicators
- **Top-left**: FPS counter (performance indicator)
- **Top-right**: Current mode name
- **SOUND mode**: Additional audio level and power readings

### Performance
- **Normal FPS**: 30-60 depending on complexity
- **Low FPS**: Try simpler display style (Grid vs Solid)
- **Smooth operation**: Device is optimized for efficiency

## 🔧 Troubleshooting

### No Audio Response in SOUND Mode
- Check if you see "0.00" (normal for quiet room)
- Make noise - should see numbers change
- Try clapping loudly near the device

### No Motion Response in TILT Mode  
- Move device more dramatically
- Try rotating instead of just tilting
- Surface responds to acceleration changes

### Performance Issues
- Switch to GRID display style (fastest)
- Some modes are more complex (PLASMA vs FLAT)
- Reset device if performance degrades

### Display Issues
- Check device orientation (should be landscape)
- Try different mode/style combinations
- Power cycle if display glitches

## 🎨 Creative Usage Ideas

### Art Installation
- Mount device to show continuous patterns
- Use PLASMA or INTERFERENCE for ambient visuals
- Cycle through modes for variety

### Music Visualization
- Use SOUND mode with music playing
- Try different music genres for different patterns
- Combine with party lighting

### Educational Demonstration
- Start with FLAT to show baseline
- Progress through mathematical concepts
- Use SINE and DRIP to show wave physics

### Interactive Display
- Let people control with TILT mode
- Demonstrate cause-and-effect with motion
- Show real-time mathematics in action

---

**Enjoy exploring the mathematical universe with your M5StickC Plus2!** 🎨✨
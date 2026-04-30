# Technical Documentation - Hector Interactive v4.0

## 🏗️ Architecture Overview

### Core Components
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   I2S Audio     │    │   MPU6886 IMU   │    │  Mathematical   │
│   Processing    │────┤   6-Axis Data    │────┤  Engine Core    │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                       │                        │
         └───────────────────────┼────────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   3D Projection     │
                    │   & Rendering       │
                    └─────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   M5Display         │
                    │   135×240 TFT       │
                    └─────────────────────┘
```

## 🎵 Audio System (I2S Implementation)

### Hardware Configuration
```cpp
#define I2S_SAMPLE_RATE 44100
#define I2S_SAMPLE_BITS 16
#define I2S_READ_LEN    512
#define I2S_MIC_SERIAL_CLOCK_PIN 0
#define I2S_MIC_SERIAL_DATA_PIN 34
```

### I2S Driver Setup
```cpp
i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
  .sample_rate = I2S_SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
  .communication_format = I2S_COMM_FORMAT_I2S,
  .dma_buf_count = 2,
  .dma_buf_len = 256,
};
```

### Audio Processing Pipeline
1. **Raw I2S Data**: 16-bit signed samples at 44.1kHz
2. **DC Offset Removal**: Eliminates bias in microphone signal
3. **RMS Calculation**: True audio power measurement
4. **Noise Gate**: Filters out background noise (<1% threshold)
5. **Smoothing**: 80% previous + 20% new (prevents jitter)
6. **Scaling**: Normalizes to 0.0-0.8 range for visualization

### Performance Metrics
- **Latency**: <50ms from sound to visual response
- **Sample Rate**: 44.1kHz (CD quality)
- **Bit Depth**: 16-bit (professional audio standard)
- **Update Rate**: 20Hz (every 50ms)

## 🎯 IMU Integration (MPU6886)

### Sensor Configuration
```cpp
// 6-axis data: accelerometer + gyroscope
float imu_accX, imu_accY, imu_accZ;    // Acceleration (g-force)
float imu_gyroX, imu_gyroY, imu_gyroZ; // Angular velocity (dps)
```

### Data Processing
```cpp
// Apply smoothing filter to reduce noise
smoothAccX = smoothAccX * 0.8 + imu_accX * 0.2;
smoothAccY = smoothAccY * 0.8 + imu_accY * 0.2;

// Use for camera angle and surface tilt
float ah = imu_accY * 2.0; // Horizontal rotation
float av = imu_accX * 1.5; // Vertical rotation
```

### Motion Mapping
- **Accelerometer**: Device tilt and orientation
- **Gyroscope**: Rotation speed and dynamic movement
- **Combined**: Creates natural surface response to device motion

## 🧮 Mathematical Engine

### Wave Equation Framework
Each mode implements a function: `f(x, y, k) → z`
- **x, y**: 2D coordinate on surface grid
- **k**: Time variable for animation
- **z**: Height value for 3D surface

### Core Algorithms

#### 1. Sine Wave (Classic)
```cpp
float sinwave(float x, float y, float k) {
  float r = 0.001 * (x² + y²);
  return 100 * cos(-k + r) / (2 + r);
}
```

#### 2. Water Drip (Original)
```cpp
float dripwave(float x, float y, float k) {
  float r = 1.5 * sqrt(x² + y²);
  float a = 200.0;
  float b = (amplitude - fmod(k/3, amplitude)) - amplitude/2;
  return (a / (1 + r)) * cos((b / log(r + 2)) * r);
}
```

#### 3. Sound Reactive (I2S)
```cpp
float soundwave(float x, float y, float k) {
  if (soundLevel < 0.01) return 0; // Noise gate
  float r = sqrt(x² + y²) * 0.1;
  float amplitude = soundLevel * 50;
  return amplitude * cos(-k * 2 + r * 3);
}
```

### 3D Projection System
```cpp
// Camera transformation
float x1 = x * cos(ah) - y * sin(ah);
float y1 = x * sin(ah) + y * cos(ah);
float z2 = z * cos(av) - x1 * sin(av);

// Perspective projection  
float s = size / (tsize - x1);
screen_x = screenHalfWidth - (zoom * (y1 * s));
screen_y = screenHalfHeight - (zoom * (z2 * s));
```

## 🎨 Rendering System

### Grid Structure
```cpp
#define SIZE 60
#define STEP 2
#define GRID_SIZE (SIZE/STEP)  // 30×30 grid
```

### Display Modes Implementation
```cpp
switch (displayStyle) {
  case DISPLAY_GRID:
    // Draw wireframe lines
    M5.Display.drawLine(x0, y0, x1, y1, color);
    break;
    
  case DISPLAY_SOLID:
    // Draw filled triangles
    M5.Display.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    break;
    
  case DISPLAY_ZEBRA:
    // Alternating pattern
    if (scan_y % 2 == pathindex % 2) {
      M5.Display.drawLine(x0, y0, x1, y1, color);
    }
    break;
}
```

### Color System
```cpp
// RGB color mapping based on position and height
red = maxrangecolor - (green - minrangecolor);
green = map(y, -halfsize, halfsize, minrangecolor, maxrangecolor);
blue = map(x, -halfsize, halfsize, minrangecolor, maxrangecolor);

// Brightness based on height
float brightnessfactor = map(z, -50, 50, 100, 20) / 100.0;
uint16_t color = M5.Display.color565(red * brightnessfactor, 
                                     green * brightnessfactor, 
                                     blue * brightnessfactor);
```

## ⚡ Performance Optimization

### Memory Management
- **Flash Usage**: 35.6% (467KB / 1.3MB)
- **RAM Usage**: 9.4% (30KB / 320KB)
- **Grid Storage**: Static allocation for coordinate caching
- **Color Palette**: Pre-calculated for fast lookup

### Rendering Optimization
```cpp
// Coordinate caching
struct Coords {
  int16_t x = -1;
  int16_t y = -1;
  uint16_t color = 0;
};
static Coords HectorGrid[GRID_SIZE + 1][GRID_SIZE + 1];
```

### Frame Rate Management
- **Target**: 30-60 FPS depending on complexity
- **VSync**: 30ms delay between frames
- **Early exit**: Skip invalid coordinates
- **Bounds checking**: Prevent off-screen drawing

## 🔧 System Configuration

### PlatformIO Settings
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 460800

lib_deps = 
    m5stack/M5StickCPlus2@^1.0.2

build_flags = 
    -DCORE_DEBUG_LEVEL=0
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_MODE=0
```

### Hardware Pinout
```
M5StickC Plus2 Pinout:
├── I2S Microphone
│   ├── CLK:  GPIO 0
│   └── DATA: GPIO 34
├── IMU (MPU6886)
│   ├── SDA: GPIO 21
│   └── SCL: GPIO 22
└── Display (ST7789V2)
    ├── Width:  240px
    └── Height: 135px
```

## 🐛 Debugging Features

### Audio Debug Information
```cpp
// Visual indicators in SOUND mode
M5.Display.printf("%.2f", soundLevel);    // Normalized level
M5.Display.printf("%.0f", soundPower/100); // Raw power
```

### Performance Monitoring
```cpp
// FPS calculation and display
if (nowmillis - fstart >= 1000) {
  fps = (framecount * 1000) / (nowmillis - fstart);
  M5.Display.printf("FPS:%2d", fps);
  framecount = 0;
}
```

### Error Handling
- **I2S Read Failure**: Graceful fallback to silent mode
- **IMU Error**: Continue with last known values
- **Memory Overflow**: Bounds checking prevents crashes
- **Invalid Coordinates**: Skip rendering invalid points

## 🔮 Future Enhancement Ideas

### Possible Improvements
1. **FFT Audio Analysis**: Frequency-based visualization
2. **WiFi Connectivity**: Remote control via web interface
3. **Accelerated Graphics**: GPU-based rendering
4. **Custom Wave Functions**: User-defined equations
5. **Multi-device Sync**: Synchronized displays
6. **Touch Integration**: Interactive surface manipulation
7. **Battery Optimization**: Dynamic frequency scaling

### Performance Upgrades
1. **Assembly Optimization**: Critical math functions
2. **Dual Core Usage**: Parallel processing
3. **DMA Graphics**: Faster display updates
4. **Compressed Textures**: Reduced memory usage

---

This documentation provides the technical foundation for understanding, modifying, and extending the Hector Interactive system.
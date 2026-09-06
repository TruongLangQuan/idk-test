/*
  ESP32-Hector adaptation for M5StickC Plus2
  
  Original ESP32-Hector is placed under the MIT license
  Copyleft (c+) 2020 tobozo 
  M5StickC Plus2 adaptation by Assistant

  This project is heavily inspired from the work of Gerard Ferrandez
  http://codepen.io/ge1doot/details/eWzQBm/
*/

#include <M5StickCPlus2.h>
#include <math.h>
#include <driver/i2s.h> // NEW: For proper I2S microphone reading

// Remove conflicting definitions
#ifdef PI
#undef PI
#endif
#ifdef TWO_PI
#undef TWO_PI
#endif

#define PI 3.14159265359
#define TWO_PI 6.28318530718

float romsin(float x) { return sin(x); }
float romcos(float x) { return cos(x); }
float romsqrt(float x) { return sqrt(x); }
float romlog(float x) { return log(x); }
float rompow(float x) { return x * x; }

// Scaled down for M5StickC Plus2's display (135x240)
#define SIZE 60
#define STEP 2
#define GRID_SIZE (SIZE/STEP)

// 3D animation parameters (scaled for smaller screen)
static float size = SIZE;
static float step = STEP * 1.2;
static float doublestep = STEP * 2;
static float speed = 0.15;
static float tsize = 0.85 * size;
static float halfsize = size * 0.5;
static float zoom = 1.33;
static float k = 0;
static float romcosav, romsinav, romcosah, romsinah;

static int num = GRID_SIZE;
static int fps = 0;

static uint16_t screenWidth = 240;  // Landscape width (was 135)
static uint16_t screenHeight = 135; // Landscape height (was 200)
static uint16_t screenHalfWidth = screenWidth / 2;
static uint16_t screenHalfHeight = screenHeight / 2;

static uint8_t maxrangecolor = 255;
static uint8_t minrangecolor = 0;
static uint8_t green, red, blue;

static unsigned long framecount = 0;
static uint32_t fstart = 0;

bool sound = false;
bool paused = false;

struct Coords {
  int16_t x = -1;
  int16_t y = -1;
  uint16_t color = 0;
};

static Coords HectorGrid[GRID_SIZE + 1][GRID_SIZE + 1];

enum DisplayStyle {
  DISPLAY_GRID,
  DISPLAY_SOLID,
  DISPLAY_ZEBRA,
  DISPLAY_CHECKBOARD
};

enum WaveStyle {
  DRIP_WAVE,
  SIN_WAVE,
  FLAT_GRID,      // Flat horizontal surface
  TILT_REACTIVE,  // Responds to device tilt
  SOUND_REACTIVE, // Responds to microphone
  SPIRAL_WAVE,    // NEW: Spiral pattern
  INTERFERENCE,   // NEW: Interference patterns
  MOUNTAIN_RANGE, // NEW: Mountain landscape
  RIPPLE_TANK,    // NEW: Multiple ripple sources
  PLASMA_FIELD    // NEW: Plasma-like energy field
};

WaveStyle waveStyle = FLAT_GRID;  // Start with flat grid
WaveStyle oldWaveStyle = SIN_WAVE;
DisplayStyle displayStyle = DISPLAY_GRID;

// IMU and sound variables - UPDATED FOR I2S
float imu_accX = 0, imu_accY = 0, imu_accZ = 0;
float imu_gyroX = 0, imu_gyroY = 0, imu_gyroZ = 0;
float soundLevel = 0;
float soundPower = 0;  // NEW: RMS power measurement
unsigned long lastIMURead = 0;
unsigned long lastSoundRead = 0;

// I2S Configuration for microphone - NEW
#define I2S_SAMPLE_RATE 44100
#define I2S_SAMPLE_BITS 16
#define I2S_READ_LEN    512
#define I2S_CHANNEL_NUM 1
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_SERIAL_CLOCK_PIN 0
#define I2S_MIC_SERIAL_DATA_PIN 34

// Manual control variables - ADDED FOR BUTTON CONTROL
bool autoMode = false;  // Start in manual mode
unsigned long lastStyleChange = 0;
unsigned long styleChangeInterval = 5000; // 5 seconds for auto mode

// Function declarations
void setupScale();
void resetCoords();
void drawPath(int scan_y);
void project(float x, float y, float z, int scan_x, int scan_y);
void checkButtons();
void updateIMU();      // Read IMU data
void updateSound();    // Read microphone data
void initI2S();        // NEW: Initialize I2S for microphone

// Function pointer to sine wave function
float (*surfaceFunction)(float x, float y, float k);

// f(x,y) equation for sin wave
float sinwave(float x, float y, float k) {
  float r = 0.001 * (rompow(x) + rompow(y));
  return 100 * romcos(-k + r) / (2 + r);
}

// f(x,y) equation for water drip wave
float dripwave(float x, float y, float k) {
  float r = 1.5 * romsqrt(rompow(x) + rompow(y));
  const float amplitude = 2.5;
  const float a = 200.0;
  const float b = (amplitude - fmod(k / 3, amplitude)) - amplitude / 2;
  return (a / (1 + r)) * romcos((b / romlog(r + 2)) * r);
}

// NEW: f(x,y) equation for flat grid (baseline)
float flatgrid(float x, float y, float k) {
  return 0; // Perfectly flat surface
}

// NEW: f(x,y) equation for tilt-reactive surface
float tiltwave(float x, float y, float k) {
  // Use IMU data to create tilted plane with ripples
  float tiltX = imu_accX * 20; // Scale tilt effect
  float tiltY = imu_accY * 20;
  float baseHeight = (x * tiltX + y * tiltY) * 0.5; // Tilted plane
  
  // Add subtle ripples based on gyro movement
  float gyroEffect = (imu_gyroX + imu_gyroY) * 10;
  float r = romsqrt(rompow(x) + rompow(y)) * 0.1;
  float ripple = gyroEffect * romcos(k + r) * 0.3;
  
  return baseHeight + ripple;
}

// NEW: f(x,y) equation for sound-reactive surface - FIXED
float soundwave(float x, float y, float k) {
  if (soundLevel < 0.01) {
    return 0; // Silent = flat surface
  }
  
  // Create waves based on sound level
  float r = romsqrt(rompow(x) + rompow(y)) * 0.1;
  float soundAmplitude = soundLevel * 50; // Reduced amplitude
  
  // Single primary wave with sound
  float wave = soundAmplitude * romcos(-k * 2 + r * 3);
  
  // Add subtle harmonics only if sound is strong enough
  if (soundLevel > 0.1) {
    wave += soundAmplitude * 0.3 * romcos(-k * 3 + r * 2);
  }
  
  return wave / (1 + r * 0.05);
}

// NEW: Spiral wave pattern
float spiralwave(float x, float y, float k) {
  float r = romsqrt(rompow(x) + rompow(y));
  float theta = atan2(y, x);
  float spiral = romsin(theta * 3 + r * 0.2 - k * 2) * 30;
  return spiral / (1 + r * 0.1);
}

// NEW: Interference pattern
float interferencewave(float x, float y, float k) {
  // Multiple wave sources creating interference
  float wave1 = romsin(romsqrt(rompow(x + 20) + rompow(y + 20)) * 0.3 - k) * 25;
  float wave2 = romsin(romsqrt(rompow(x - 20) + rompow(y - 20)) * 0.3 - k * 1.2) * 25;
  float wave3 = romsin(romsqrt(rompow(x) + rompow(y + 30)) * 0.25 - k * 0.8) * 20;
  return (wave1 + wave2 + wave3) * 0.6;
}

// NEW: Mountain range landscape
float mountainwave(float x, float y, float k) {
  // Create mountain-like terrain
  float mountain1 = 40 * romcos((x + k * 5) * 0.1) * romcos(y * 0.15);
  float mountain2 = 25 * romsin((x - k * 3) * 0.08) * romsin((y + 10) * 0.12);
  float noise = 10 * romsin(x * 0.3 + k) * romcos(y * 0.25 + k * 0.7);
  return mountain1 + mountain2 + noise;
}

// NEW: Ripple tank (multiple sources)
float ripplewave(float x, float y, float k) {
  // Multiple ripple sources
  float r1 = romsqrt(rompow(x + 15) + rompow(y + 10));
  float r2 = romsqrt(rompow(x - 20) + rompow(y - 15));
  float r3 = romsqrt(rompow(x - 10) + rompow(y + 25));
  
  float ripple1 = 30 * romcos(-k * 2 + r1 * 0.3) / (1 + r1 * 0.1);
  float ripple2 = 25 * romcos(-k * 2.5 + r2 * 0.28) / (1 + r2 * 0.12);
  float ripple3 = 20 * romcos(-k * 1.8 + r3 * 0.35) / (1 + r3 * 0.08);
  
  return ripple1 + ripple2 + ripple3;
}

// NEW: Plasma field energy
float plasmawave(float x, float y, float k) {
  // Plasma-like energy field
  float plasma1 = romsin(x * 0.2 + k) + romcos(y * 0.25 + k * 1.3);
  float plasma2 = romsin((x + y) * 0.15 + k * 0.8) + romcos((x - y) * 0.18 + k * 1.1);
  float plasma3 = romsin(romsqrt(x * x + y * y) * 0.1 + k * 0.6);
  
  return (plasma1 + plasma2 + plasma3) * 15;
}

// NEW: Initialize I2S for proper microphone reading
void initI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_MIC_CHANNEL,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 256,
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num   = I2S_PIN_NO_CHANGE,
    .ws_io_num    = I2S_MIC_SERIAL_CLOCK_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_MIC_SERIAL_DATA_PIN,
  };
  
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}
// NEW: Update IMU data
void updateIMU() {
  if (millis() - lastIMURead > 50) { // Update every 50ms
    lastIMURead = millis();
    
    // Read accelerometer and gyroscope
    M5.Imu.getAccelData(&imu_accX, &imu_accY, &imu_accZ);
    M5.Imu.getGyroData(&imu_gyroX, &imu_gyroY, &imu_gyroZ);
    
    // Apply smoothing filter to reduce noise
    static float smoothAccX = 0, smoothAccY = 0;
    static float smoothGyroX = 0, smoothGyroY = 0;
    
    smoothAccX = smoothAccX * 0.8 + imu_accX * 0.2;
    smoothAccY = smoothAccY * 0.8 + imu_accY * 0.2;
    smoothGyroX = smoothGyroX * 0.8 + imu_gyroX * 0.2;
    smoothGyroY = smoothGyroY * 0.8 + imu_gyroY * 0.2;
    
    imu_accX = smoothAccX;
    imu_accY = smoothAccY;
    imu_gyroX = smoothGyroX;
    imu_gyroY = smoothGyroY;
  }
}

// NEW: Update sound level from I2S microphone - PROPER IMPLEMENTATION
void updateSound() {
  if (millis() - lastSoundRead > 50) { // Update every 50ms
    lastSoundRead = millis();
    
    // Read I2S audio data
    int16_t i2s_data[I2S_READ_LEN];
    size_t bytes_read;
    
    esp_err_t result = i2s_read(I2S_NUM_0, (char*)i2s_data, I2S_READ_LEN * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    
    if (result == ESP_OK && bytes_read > 0) {
      int samples_read = bytes_read / sizeof(int16_t);
      
      // Calculate DC offset
      int32_t dc_offset = 0;
      for (int i = 0; i < samples_read; i++) {
        dc_offset += i2s_data[i];
      }
      dc_offset /= samples_read;
      
      // Calculate RMS power (proper audio analysis)
      float power = 0;
      for (int i = 0; i < samples_read; i++) {
        int32_t sample = i2s_data[i] - dc_offset;
        power += (float)(sample * sample);
      }
      
      if (samples_read > 0) {
        soundPower = sqrt(power / samples_read);
        
        // Convert to normalized level (0.0 to 1.0)
        soundLevel = soundPower / 32768.0; // 16-bit max value
        
        // Apply noise gate - ignore very quiet sounds  
        if (soundLevel < 0.01) {
          soundLevel = 0;
        }
        
        // Apply heavy smoothing to prevent jitter
        static float smoothSound = 0;
        smoothSound = smoothSound * 0.8 + soundLevel * 0.2;
        soundLevel = smoothSound;
        
        // Scale for better visual effect but keep reasonable
        soundLevel = min(0.8f, soundLevel * 3.0f);
      }
    }
  }
}

void resetCoords() {
  for (uint16_t x = 0; x < GRID_SIZE + 1; x++) {
    for (uint16_t y = 0; y < GRID_SIZE + 1; y++) {
      HectorGrid[x][y].x = -1;
      HectorGrid[x][y].y = -1;
      HectorGrid[x][y].color = 0;
    }
  }
}

void setupScale() {
  num = GRID_SIZE;
  doublestep = step * 2;
  speed = 0.15;
  tsize = 0.85 * size;
  halfsize = size * 0.5;
  zoom = 1.33;
  resetCoords();
}

// projection
void project(float x, float y, float z, int scan_x, int scan_y) {
  if (scan_x >= GRID_SIZE + 1 || scan_y >= GRID_SIZE + 1) return;
  
  float x1 = x * romcosah - y * romsinah;
  float y1 = x * romsinah + y * romcosah;
  float z2 = z * romcosav - x1 * romsinav;
  if ((tsize - x1) == 0) return;
  float s = size / (tsize - x1);
  HectorGrid[scan_x][scan_y].x = screenHalfWidth - (zoom * (y1 * s));
  HectorGrid[scan_x][scan_y].y = screenHalfHeight - (zoom * (z2 * s));
}

void drawPath(int scan_y) {
  if (scan_y == 0) return;
  
  for (int pathindex = 1; pathindex < num && pathindex < GRID_SIZE; pathindex++) {
    if (scan_y >= GRID_SIZE) break;
    
    int16_t x0 = HectorGrid[pathindex-1][scan_y].x;
    int16_t y0 = HectorGrid[pathindex-1][scan_y].y;
    int16_t x1 = HectorGrid[pathindex][scan_y].x;
    int16_t y1 = HectorGrid[pathindex][scan_y].y;
    int16_t x2 = HectorGrid[pathindex][scan_y-1].x;
    int16_t y2 = HectorGrid[pathindex][scan_y-1].y;
    
    uint16_t color = HectorGrid[pathindex][scan_y].color;
    
    if (color == 0 || x1 < 0 || y1 < 0) continue;
    
    // Draw based on display style with proper coordinate bounds checking
    switch (displayStyle) {
      case DISPLAY_GRID:
        // Draw wireframe lines
        if (x0 >= 0 && y0 >= 0) {
          M5.Display.drawLine(constrain(x0, 0, 239), constrain(y0 + 10, 10, 130), 
                             constrain(x1, 0, 239), constrain(y1 + 10, 10, 130), color);
        }
        if (x2 >= 0 && y2 >= 0) {
          M5.Display.drawLine(constrain(x1, 0, 239), constrain(y1 + 10, 10, 130), 
                             constrain(x2, 0, 239), constrain(y2 + 10, 10, 130), color);
        }
        break;
        
      case DISPLAY_SOLID:
        // Draw filled triangles for solid surface effect
        if (pathindex > 1 && scan_y > 1) {
          int16_t x3 = HectorGrid[pathindex-1][scan_y-1].x;
          int16_t y3 = HectorGrid[pathindex-1][scan_y-1].y;
          
          if (x3 >= 0 && y3 >= 0) {
            // Draw triangulated surface
            M5.Display.fillTriangle(
              constrain(x0, 0, 239), constrain(y0 + 10, 10, 130),
              constrain(x1, 0, 239), constrain(y1 + 10, 10, 130), 
              constrain(x2, 0, 239), constrain(y2 + 10, 10, 130), color);
            M5.Display.fillTriangle(
              constrain(x0, 0, 239), constrain(y0 + 10, 10, 130),
              constrain(x2, 0, 239), constrain(y2 + 10, 10, 130),
              constrain(x3, 0, 239), constrain(y3 + 10, 10, 130), color);
          }
        } else {
          // Fallback to thicker lines for edge cases
          M5.Display.drawLine(constrain(x0, 0, 239), constrain(y0 + 10, 10, 130), 
                             constrain(x1, 0, 239), constrain(y1 + 10, 10, 130), color);
        }
        break;
        
      case DISPLAY_ZEBRA:
        // Draw lines only for alternating patterns (zebra stripes)
        if (scan_y % 2 == pathindex % 2) {
          if (x0 >= 0 && y0 >= 0) {
            M5.Display.drawLine(constrain(x0, 0, 239), constrain(y0 + 10, 10, 130), 
                               constrain(x1, 0, 239), constrain(y1 + 10, 10, 130), color);
          }
          if (x2 >= 0 && y2 >= 0) {
            M5.Display.drawLine(constrain(x1, 0, 239), constrain(y1 + 10, 10, 130), 
                               constrain(x2, 0, 239), constrain(y2 + 10, 10, 130), color);
          }
        }
        break;
        
      case DISPLAY_CHECKBOARD:
        // Draw filled rectangles in checkboard pattern
        if ((pathindex + scan_y) % 2 == 0) {
          // Draw a visible rectangle instead of just a pixel
          int rectSize = 4; // Make it clearly visible
          int centerX = constrain(x1, rectSize/2, 239-rectSize/2);
          int centerY = constrain(y1 + 10, 10+rectSize/2, 130-rectSize/2);
          M5.Display.fillRect(centerX - rectSize/2, centerY - rectSize/2, rectSize, rectSize, color);
          
          // Also add connecting lines for structure
          if (x0 >= 0 && y0 >= 0) {
            M5.Display.drawLine(constrain(x0, 0, 239), constrain(y0 + 10, 10, 130), 
                               centerX, centerY, color);
          }
        }
        break;
    }
  }
}

void sinLoop() {
  if (paused) return;

  k += speed;
  
  // Update sensors for interactive modes
  updateIMU();
  updateSound();

  setupScale();

  // Use real IMU data instead of simulated gyro for camera angle
  float ah = imu_accY * 2.0; // Use real tilt for camera
  float av = imu_accX * 1.5;

  romcosav = romcos(av);
  romsinav = romsin(av);
  romcosah = romcos(ah);
  romsinah = romsin(ah);

  M5.Display.fillScreen(BLACK);

  int scan_y = 0;

  for (float x = halfsize; x >= -halfsize; x -= doublestep) {
    blue = map(x, -halfsize, halfsize, minrangecolor, maxrangecolor);

    int scan_x = 0;

    for (float y = -halfsize; y <= halfsize; y += step) {
      if (scan_x >= GRID_SIZE || scan_y >= GRID_SIZE) break;

      float z = surfaceFunction(x, y, k);
      green = map(y, -halfsize, halfsize, minrangecolor, maxrangecolor);
      float brightnessfactor = float(map(int(z), -50, 50, 100, 20)) / 100.0;
      red = maxrangecolor - (green - minrangecolor);
      green *= brightnessfactor;
      red *= brightnessfactor;
      blue *= brightnessfactor;

      HectorGrid[scan_x][scan_y].color = M5.Display.color565(red, green, blue);
      project(x, y, z * 1.2, scan_x, scan_y);
      scan_x++;
    }
    
    // Draw the path for this scan line
    drawPath(scan_y);
    scan_y++;
  }

  // FPS display
  unsigned long nowmillis = millis();
  if (nowmillis - fstart >= 1000) {
    fps = (framecount * 1000) / (nowmillis - fstart);
    fstart = nowmillis;
    M5.Display.setCursor(5, 5);
    M5.Display.setTextColor(WHITE);
    M5.Display.printf("FPS:%2d", fps);
    framecount = 0;
  } else {
    framecount++;
  }
  
  // Show current mode info with sound level for debugging
  M5.Display.setCursor(180, 5);
  M5.Display.setTextColor(GREEN);
  switch(waveStyle) {
    case FLAT_GRID: M5.Display.printf("FLAT"); break;
    case TILT_REACTIVE: M5.Display.printf("TILT"); break;
    case SOUND_REACTIVE: 
      M5.Display.printf("SND");
      // Show sound level and power for debugging
      M5.Display.setCursor(180, 15);
      M5.Display.setTextColor(soundLevel > 0.05 ? RED : YELLOW);
      M5.Display.printf("%.2f", soundLevel);
      M5.Display.setCursor(180, 25);
      M5.Display.setTextColor(CYAN);
      M5.Display.printf("%.0f", soundPower / 100);
      break;
    case SPIRAL_WAVE: M5.Display.printf("SPRL"); break;
    case INTERFERENCE: M5.Display.printf("INTRF"); break;
    case MOUNTAIN_RANGE: M5.Display.printf("MNTN"); break;
    case RIPPLE_TANK: M5.Display.printf("RIPP"); break;
    case PLASMA_FIELD: M5.Display.printf("PLSM"); break;
    case SIN_WAVE: M5.Display.printf("SINE"); break;
    case DRIP_WAVE: M5.Display.printf("DRIP"); break;
  }

  // Only auto-change if in auto mode (keeping the existing auto feature)
  if (autoMode) {
    int32_t segment = nowmillis % 8000;
    static int lastsegmentpos = -1;
    int segmentpos = map(segment, 0, 8000, 0, 16);
    
    if (lastsegmentpos != segmentpos) {
      lastsegmentpos = segmentpos;
      if (segmentpos % 4 == 0) {
        switch (displayStyle) {
          case DISPLAY_SOLID: displayStyle = DISPLAY_GRID; break;
          case DISPLAY_GRID: displayStyle = DISPLAY_ZEBRA; break;
          case DISPLAY_ZEBRA: displayStyle = DISPLAY_CHECKBOARD; break;
          case DISPLAY_CHECKBOARD: displayStyle = DISPLAY_SOLID; break;
        }
      }
    }
  }
}

void checkButtons() {
  M5.update();
  
  // Button A - Cycle through display styles manually
  if (M5.BtnA.wasPressed()) {
    // Cycle through display styles
    switch (displayStyle) {
      case DISPLAY_GRID: displayStyle = DISPLAY_SOLID; break;
      case DISPLAY_SOLID: displayStyle = DISPLAY_ZEBRA; break;
      case DISPLAY_ZEBRA: displayStyle = DISPLAY_CHECKBOARD; break;
      case DISPLAY_CHECKBOARD: displayStyle = DISPLAY_GRID; break;
    }
    
    // Show current style name
    M5.Display.fillRect(0, 0, 120, 20, BLACK); // Clear status area
    M5.Display.setCursor(5, 5);
    M5.Display.setTextColor(WHITE);
    const char* styleNames[] = {"GRID", "SOLID", "ZEBRA", "CHECK"};
    M5.Display.printf("Style: %s", styleNames[displayStyle]);
  }
  
  // Button B - Cycle through wave types including NEW patterns
  if (M5.BtnB.wasPressed()) {
    // Cycle through all wave types including new ones
    switch (waveStyle) {
      case FLAT_GRID: 
        waveStyle = TILT_REACTIVE;
        surfaceFunction = &tiltwave;
        break;
      case TILT_REACTIVE: 
        waveStyle = SOUND_REACTIVE;
        surfaceFunction = &soundwave;
        break;
      case SOUND_REACTIVE: 
        waveStyle = SPIRAL_WAVE;
        surfaceFunction = &spiralwave;
        break;
      case SPIRAL_WAVE:
        waveStyle = INTERFERENCE;
        surfaceFunction = &interferencewave;
        break;
      case INTERFERENCE:
        waveStyle = MOUNTAIN_RANGE;
        surfaceFunction = &mountainwave;
        break;
      case MOUNTAIN_RANGE:
        waveStyle = RIPPLE_TANK;
        surfaceFunction = &ripplewave;
        break;
      case RIPPLE_TANK:
        waveStyle = PLASMA_FIELD;
        surfaceFunction = &plasmawave;
        break;
      case PLASMA_FIELD:
        waveStyle = SIN_WAVE;
        size = SIZE;
        step = STEP * 1.5;
        setupScale();
        surfaceFunction = &sinwave;
        break;
      case SIN_WAVE: 
        waveStyle = DRIP_WAVE;
        setupScale();
        surfaceFunction = &dripwave;
        break;
      case DRIP_WAVE: 
        waveStyle = FLAT_GRID;
        surfaceFunction = &flatgrid;
        break;
    }
    oldWaveStyle = waveStyle;
    
    // Show current wave type with shorter names for screen space
    M5.Display.fillRect(120, 0, 120, 20, BLACK);
    M5.Display.setCursor(125, 5);
    M5.Display.setTextColor(GREEN);
    switch(waveStyle) {
      case FLAT_GRID: M5.Display.printf("FLAT"); break;
      case TILT_REACTIVE: M5.Display.printf("TILT"); break;
      case SOUND_REACTIVE: M5.Display.printf("SOUND"); break;
      case SPIRAL_WAVE: M5.Display.printf("SPIRAL"); break;
      case INTERFERENCE: M5.Display.printf("INTER"); break;
      case MOUNTAIN_RANGE: M5.Display.printf("MOUNT"); break;
      case RIPPLE_TANK: M5.Display.printf("RIPPLE"); break;
      case PLASMA_FIELD: M5.Display.printf("PLASMA"); break;
      case SIN_WAVE: M5.Display.printf("SINE"); break;
      case DRIP_WAVE: M5.Display.printf("DRIP"); break;
    }
  }
}

void setup() {
  M5.begin();
  
  // Initialize IMU
  M5.Imu.init();
  
  // Initialize I2S microphone - NEW
  initI2S();
  
  // Set up display - CHANGED TO LANDSCAPE MODE
  M5.Display.setRotation(1); // Landscape mode (was 0 for portrait)
  M5.Display.setBrightness(128);
  M5.Display.fillScreen(BLACK);

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 50);
  M5.Display.println("ESP32");
  M5.Display.setCursor(10, 80);
  M5.Display.println("Hector");
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 120);
  M5.Display.println("M5StickC Plus2");
  M5.Display.setCursor(10, 140);
  M5.Display.println("Loading...");

  delay(3000);

  surfaceFunction = &sinwave;
  setupScale();

  fstart = millis() - 1;

  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(5, 115);
  M5.Display.printf("A:Style B:Mode");

  Serial.begin(115200);
  Serial.println("Hector M5StickC Plus2 initialized");
}

void loop() {
  checkButtons();
  sinLoop();
  
  // Small delay to prevent overwhelming the system
  delay(30);
}
/*
  Tilt-Reactive Theremin for M5StickC Plus2
  Combines Hector visuals with theremin sound control via IMU
  
  Controls:
  - Roll (left/right tilt) → Frequency (pitch)
  - Pitch (forward/back tilt) → Volume
  - Button A: Toggle sound on/off
  - Button B: Cycle visual modes
*/

#include <M5StickCPlus2.h>
#include <math.h>

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
float rompow(float x) { return x * x; }

// Scaled for M5StickC Plus2 display (240x135 in landscape)
#define SIZE 60
#define STEP 2
#define GRID_SIZE (SIZE/STEP)

// 3D animation parameters
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

static uint16_t screenWidth = 240;
static uint16_t screenHeight = 135;
static uint16_t screenHalfWidth = screenWidth / 2;
static uint16_t screenHalfHeight = screenHeight / 2;

static uint8_t maxrangecolor = 255;
static uint8_t minrangecolor = 0;
static uint8_t green, red, blue;

struct Coords {
  int16_t x = -1;
  int16_t y = -1;
  uint16_t color = 0;
};

static Coords HectorGrid[GRID_SIZE + 1][GRID_SIZE + 1];

enum DisplayStyle {
  DISPLAY_GRID,
  DISPLAY_SOLID,
  DISPLAY_ZEBRA
};

DisplayStyle displayStyle = DISPLAY_GRID;

// Octave modes for PWR button cycling
enum OctaveMode {
  OCTAVE_LOW,     // 100-600Hz - Deep bass mode
  OCTAVE_MID,     // 200-1200Hz - Standard mode
  OCTAVE_HIGH,    // 400-2400Hz - Soprano mode
  OCTAVE_COSMIC   // 100-2400Hz + vibrato - Full range psychedelic
};

OctaveMode octaveMode = OCTAVE_MID;

// Theremin variables
bool thereminOn = false;
bool isTonePlaying = false;  // Track if tone is currently active
float frequency = 440.0;  // Current frequency in Hz
float lastToneFreq = 440.0;  // Last frequency sent to speaker
float volume = 0.0;       // Current volume (0-255)
float roll = 0.0;         // IMU roll angle (left/right)
float pitch = 0.0;        // IMU pitch angle (forward/back)

// Dynamic frequency ranges based on octave mode
float minFreq = 200.0;
float maxFreq = 1200.0;
#define MIN_ANGLE -45.0
#define MAX_ANGLE 45.0
#define MIN_VOLUME_ANGLE -30.0
#define MAX_VOLUME_ANGLE 30.0

// Speaker PWM configuration
#define SPEAKER_PIN 2  // Built-in speaker on M5StickC Plus2
#define PWM_CHANNEL 0
#define PWM_RESOLUTION 8

unsigned long lastIMURead = 0;
unsigned long lastFreqUpdate = 0;
const int IMU_UPDATE_INTERVAL = 20;    // Read IMU every 20ms
const int FREQ_UPDATE_INTERVAL = 10;   // Update frequency every 10ms

// Function declarations
void setupScale();
void resetCoords();
void drawPath(int scan_y);
void project(float x, float y, float z, int scan_x, int scan_y);
void checkButtons();
void updateIMU();
void updateTheremin();
void thereminWave(float x, float y, float k, float& height);

// Utility functions
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float constrainFloat(float x, float min_val, float max_val) {
  if (x < min_val) return min_val;
  if (x > max_val) return max_val;
  return x;
}

// Calculate tilt angles from accelerometer
void updateIMU() {
  unsigned long now = millis();
  if (now - lastIMURead < IMU_UPDATE_INTERVAL) return;
  lastIMURead = now;
  
  float accX, accY, accZ;
  M5.Imu.getAccel(&accX, &accY, &accZ);
  
  // Calculate roll (left/right tilt) and pitch (forward/back tilt)
  roll = atan2(accY, accZ) * 180.0 / PI;
  pitch = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;
  
  // Simple smoothing
  static float lastRoll = 0, lastPitch = 0;
  roll = lastRoll * 0.7 + roll * 0.3;
  pitch = lastPitch * 0.7 + pitch * 0.3;
  lastRoll = roll;
  lastPitch = pitch;
}

// Update theremin frequency and volume based on tilt
void updateTheremin() {
  unsigned long now = millis();
  if (now - lastFreqUpdate < FREQ_UPDATE_INTERVAL) return;
  lastFreqUpdate = now;
  
  if (!thereminOn) {
    M5.Speaker.end();
    return;
  }
  
  // Map roll angle to frequency (left/right controls pitch)
  float targetFreq = mapFloat(
    constrainFloat(roll, MIN_ANGLE, MAX_ANGLE),
    MIN_ANGLE, MAX_ANGLE,
    minFreq, maxFreq
  );
  
  // Add vibrato/wah effect in COSMIC mode
  if (octaveMode == OCTAVE_COSMIC) {
    float vibrato = romsin(now * 0.005) * 15.0;  // ±15Hz oscillation
    float wah = romcos(now * 0.003) * 30.0;      // ±30Hz slower wave
    targetFreq += vibrato + wah;
  }
  
  // Smooth frequency changes
  frequency = frequency * 0.9 + targetFreq * 0.1;
  
  // Map pitch angle to volume (forward/back controls volume)
  float targetVolume = mapFloat(
    constrainFloat(pitch, MIN_VOLUME_ANGLE, MAX_VOLUME_ANGLE),
    MAX_VOLUME_ANGLE, MIN_VOLUME_ANGLE,
    0, 255
  );
  
  // Smooth volume changes
  volume = volume * 0.8 + targetVolume * 0.2;
  
  // Apply to speaker - only update tone when frequency changes significantly
  int vol = (int)constrainFloat(volume, 0, 255);
  M5.Speaker.setVolume(vol);
  
  if (vol > 10) {
    // Update tone every cycle for smooth continuous sound
    M5.Speaker.tone(frequency, 50);  // Short 50ms bursts for smooth overlap
    lastToneFreq = frequency;
    isTonePlaying = true;
  } else {
    if (isTonePlaying) {  // Only stop if we were actually playing
      M5.Speaker.stop();
      isTonePlaying = false;
    }
  }
}

// Theremin-reactive wave function
void thereminWave(float x, float y, float k, float& height) {
  // Base wave that responds to current frequency
  float r = romsqrt(rompow(x) + rompow(y));
  float freqScale = frequency / 440.0;  // Normalize around A440
  
  // Create wave patterns based on frequency
  float wave1 = 50 * romcos(-k * freqScale + r * 0.1) / (2 + r * 0.05);
  
  // Add volume-based amplitude modulation
  float volumeScale = volume / 128.0;
  height = wave1 * (0.5 + volumeScale * 0.5);
  
  // Add tilt-based warping
  float tiltEffect = (roll * 0.3) + (pitch * 0.2);
  height += tiltEffect;
}

void setupScale() {
  step = doublestep * size / tsize;
  num = size / step;
}

void resetCoords() {
  for (int scan_y = 0; scan_y <= num; scan_y++) {
    for (int scan_x = 0; scan_x <= num; scan_x++) {
      HectorGrid[scan_y][scan_x].x = -1;
      HectorGrid[scan_y][scan_x].y = -1;
    }
  }
}

void project(float x, float y, float z, int scan_x, int scan_y) {
  float zz = 1.0 / (z - zoom);
  float xp = (x * zz - romsinav) * halfsize + screenHalfWidth;
  float yp = (y * zz - romcosav) * halfsize + screenHalfHeight;

  HectorGrid[scan_y][scan_x].x = (int16_t)xp;
  HectorGrid[scan_y][scan_x].y = (int16_t)yp;

  float dist = romsqrt(rompow(x) + rompow(y) + rompow(z));
  uint8_t col = (uint8_t)(maxrangecolor - ((dist - 200) / 1.5));
  
  // Color based on frequency when theremin is on
  if (thereminOn) {
    float hue = mapFloat(frequency, minFreq, maxFreq, 0, 240);
    red = (uint8_t)(romsin((hue + 0) * PI / 180.0) * 127 + 128);
    green = (uint8_t)(romsin((hue + 120) * PI / 180.0) * 127 + 128);
    blue = (uint8_t)(romsin((hue + 240) * PI / 180.0) * 127 + 128);
  } else {
    red = col;
    green = col / 2;
    blue = col / 3;
  }
  
  HectorGrid[scan_y][scan_x].color = M5.Display.color565(red, green, blue);
}

void drawPath(int scan_y) {
  if (scan_y >= num) return;

  for (int scan_x = 0; scan_x < num; scan_x++) {
    int16_t x0 = HectorGrid[scan_y][scan_x].x;
    int16_t y0 = HectorGrid[scan_y][scan_x].y;
    int16_t x1 = HectorGrid[scan_y][scan_x + 1].x;
    int16_t y1 = HectorGrid[scan_y][scan_x + 1].y;
    int16_t x2 = HectorGrid[scan_y + 1][scan_x].x;
    int16_t y2 = HectorGrid[scan_y + 1][scan_x].y;
    int16_t x3 = HectorGrid[scan_y + 1][scan_x + 1].x;
    int16_t y3 = HectorGrid[scan_y + 1][scan_x + 1].y;

    if (x0 >= 0 && x1 >= 0 && x2 >= 0 && x3 >= 0) {
      uint16_t color = HectorGrid[scan_y][scan_x].color;

      switch (displayStyle) {
        case DISPLAY_GRID:
          M5.Display.drawLine(x0, y0, x1, y1, color);
          M5.Display.drawLine(x0, y0, x2, y2, color);
          if (scan_x == num - 1) M5.Display.drawLine(x1, y1, x3, y3, color);
          if (scan_y == num - 1) M5.Display.drawLine(x2, y2, x3, y3, color);
          break;
        case DISPLAY_SOLID:
          M5.Display.fillTriangle(x0, y0, x1, y1, x2, y2, color);
          M5.Display.fillTriangle(x1, y1, x2, y2, x3, y3, color);
          break;
        case DISPLAY_ZEBRA:
          if ((scan_x + scan_y) % 2 == 0) {
            M5.Display.fillTriangle(x0, y0, x1, y1, x2, y2, color);
            M5.Display.fillTriangle(x1, y1, x2, y2, x3, y3, color);
          }
          break;
      }
    }
  }
}

void sinLoop() {
  k += speed;
  
  float av = k * 0.5;
  float ah = k * 0.25;
  romcosav = romcos(av);
  romsinav = romsin(av);
  romcosah = romcos(ah);
  romsinah = romsin(ah);

  resetCoords();

  for (int scan_y = 0; scan_y <= num; scan_y++) {
    float cy = step * (scan_y - num * 0.5);
    float cy_cosah = cy * romcosah;
    float cy_sinah = cy * romsinah;

    for (int scan_x = 0; scan_x <= num; scan_x++) {
      float cx = step * (scan_x - num * 0.5);
      float x = cx * romcosah - cy_sinah;
      float y = cx * romsinah + cy_cosah;
      
      float height;
      thereminWave(x, y, k, height);
      float z = height;

      float xx = x * romcosav + z * romsinav;
      float zz = z * romcosav - x * romsinav;

      project(xx, y, zz, scan_x, scan_y);
    }
  }

  for (int scan_y = 0; scan_y <= num; scan_y++) {
    drawPath(scan_y);
  }

  // Display theremin status
  M5.Display.fillRect(0, 0, 240, 15, BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(5, 5);
  
  if (thereminOn) {
    M5.Display.setTextColor(GREEN);
    M5.Display.printf("ON %.0fHz V:%d", frequency, (int)volume);
  } else {
    M5.Display.setTextColor(RED);
    M5.Display.printf("OFF - Press A");
  }
  
  // Display octave mode on bottom
  M5.Display.fillRect(0, 120, 240, 15, BLACK);
  M5.Display.setCursor(5, 122);
  M5.Display.setTextSize(1);
  
  const char* modeName = "";
  uint16_t modeColor = YELLOW;
  switch (octaveMode) {
    case OCTAVE_LOW: 
      modeName = "BASS"; 
      modeColor = BLUE;
      break;
    case OCTAVE_MID: 
      modeName = "MID"; 
      modeColor = GREEN;
      break;
    case OCTAVE_HIGH: 
      modeName = "SOPRANO"; 
      modeColor = ORANGE;
      break;
    case OCTAVE_COSMIC: 
      modeName = "COSMIC WAH"; 
      modeColor = MAGENTA;
      break;
  }
  M5.Display.setTextColor(modeColor);
  M5.Display.printf("%s %.0f-%.0fHz", modeName, minFreq, maxFreq);
}

void checkButtons() {
  M5.update();
  
  // Button A - Toggle theremin on/off
  if (M5.BtnA.wasPressed()) {
    thereminOn = !thereminOn;
    
    if (thereminOn) {
      M5.Speaker.begin();
      M5.Speaker.setVolume(128);
      Serial.println("Theremin ON");
    } else {
      M5.Speaker.end();
      Serial.println("Theremin OFF");
    }
  }
  
  // Button B - Cycle through visual styles
  if (M5.BtnB.wasPressed()) {
    switch (displayStyle) {
      case DISPLAY_GRID: 
        displayStyle = DISPLAY_SOLID; 
        Serial.println("Display: SOLID");
        break;
      case DISPLAY_SOLID: 
        displayStyle = DISPLAY_ZEBRA; 
        Serial.println("Display: ZEBRA");
        break;
      case DISPLAY_ZEBRA: 
        displayStyle = DISPLAY_GRID; 
        Serial.println("Display: GRID");
        break;
    }
  }
  
  // Button PWR - Cycle through octave modes
  if (M5.BtnPWR.wasPressed()) {
    switch (octaveMode) {
      case OCTAVE_LOW:
        octaveMode = OCTAVE_MID;
        minFreq = 200.0;
        maxFreq = 1200.0;
        Serial.println("Octave: MID (200-1200Hz)");
        break;
      case OCTAVE_MID:
        octaveMode = OCTAVE_HIGH;
        minFreq = 400.0;
        maxFreq = 2400.0;
        Serial.println("Octave: HIGH (400-2400Hz)");
        break;
      case OCTAVE_HIGH:
        octaveMode = OCTAVE_COSMIC;
        minFreq = 100.0;
        maxFreq = 2400.0;
        Serial.println("Octave: COSMIC (100-2400Hz + wah)");
        break;
      case OCTAVE_COSMIC:
        octaveMode = OCTAVE_LOW;
        minFreq = 100.0;
        maxFreq = 600.0;
        Serial.println("Octave: LOW (100-600Hz)");
        break;
    }
  }
  
  // Button PWR held - show help
  if (M5.BtnPWR.pressedFor(1000)) {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 10);
    M5.Display.println("THEREMIN CONTROLS");
    M5.Display.setCursor(10, 30);
    M5.Display.setTextColor(WHITE);
    M5.Display.println("Roll L/R: Frequency");
    M5.Display.println("Pitch F/B: Volume");
    M5.Display.println("");
    M5.Display.println("Btn A: Sound On/Off");
    M5.Display.println("Btn B: Visual Mode");
    M5.Display.println("Btn PWR: Octave Mode");
    M5.Display.println("");
    M5.Display.setTextColor(MAGENTA);
    M5.Display.println("BASS/MID/HIGH/COSMIC");
    delay(3000);
    M5.Display.fillScreen(BLACK);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  Serial.println("Tilt-Reactive Theremin Starting...");
  
  // Initialize IMU
  M5.Imu.init();
  Serial.println("IMU initialized");
  
  // Set up display in landscape mode
  M5.Display.setRotation(1);
  M5.Display.setBrightness(128);
  M5.Display.fillScreen(BLACK);

  // Show splash screen
  M5.Display.setTextColor(CYAN);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(30, 40);
  M5.Display.println("THEREMIN");
  M5.Display.setTextSize(1);
  M5.Display.setCursor(40, 70);
  M5.Display.setTextColor(WHITE);
  M5.Display.println("Tilt-Reactive");
  M5.Display.setCursor(50, 90);
  M5.Display.println("M5StickC Plus2");
  
  delay(2000);
  
  // Initialize speaker (but don't start tone yet)
  auto spk_cfg = M5.Speaker.config();
  M5.Speaker.config(spk_cfg);
  
  setupScale();
  
  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(5, 120);
  M5.Display.setTextColor(WHITE);
  M5.Display.printf("A:Sound B:Visual");
  
  Serial.println("Ready! Press A to enable theremin");
  Serial.println("Tilt left/right for pitch, forward/back for volume");
}

void loop() {
  updateIMU();
  checkButtons();
  updateTheremin();
  sinLoop();
  
  delay(10);
}

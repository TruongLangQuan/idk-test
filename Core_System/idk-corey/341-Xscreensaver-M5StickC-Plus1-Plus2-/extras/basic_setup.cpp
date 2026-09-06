/*
 * ESP32 Screensaver Universe - Basic Setup Example
 * 
 * This example demonstrates the minimal setup required to run
 * a single screensaver effect on ESP32 with ILI9341 display.
 * 
 * Perfect for learning the basics or testing your hardware setup.
 */

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <XPT2046_Touchscreen.h>

// Pin Definitions for ESP32-2432S028 (adjust for your board)
#define TFT_CS    15
#define TFT_RST   -1  // Connected to EN pin
#define TFT_DC    2
#define TFT_MOSI  13  
#define TFT_CLK   14
#define TFT_MISO  12

#define TOUCH_CS  33
#define TOUCH_IRQ 36

// Display and Touch objects
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_CLK, TFT_MOSI, TFT_MISO, VSPI, true);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, TFT_RST, 0 /* rotation */, false);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// Animation variables
float animationTime = 0;
unsigned long lastUpdate = 0;
const unsigned long frameDelay = 50; // 20 FPS

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Screensaver - Basic Setup Example");
  
  // Initialize display
  gfx->begin();
  gfx->fillScreen(BLACK);
  
  // Initialize touch (optional)
  ts.begin();
  ts.setRotation(1);
  
  Serial.println("Setup complete! Starting screensaver...");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Maintain frame rate
  if (currentTime - lastUpdate >= frameDelay) {
    lastUpdate = currentTime;
    
    // Run the basic screensaver effect
    drawBasicEffect();
    
    // Handle touch input (optional)
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      Serial.printf("Touch detected at X=%d, Y=%d\n", p.x, p.y);
      
      // Reset animation on touch
      animationTime = 0;
    }
    
    // Update animation time
    animationTime += 0.05;
  }
}

/*
 * Basic Screensaver Effect - Bouncing Ball
 * 
 * This demonstrates a simple animated effect that showcases:
 * - Graphics drawing
 * - Animation timing
 * - Color cycling
 * - Physics simulation
 */
void drawBasicEffect() {
  // Clear screen with dark blue background
  gfx->fillScreen(gfx->color565(0, 0, 30));
  
  // Calculate ball position using sine waves for smooth motion
  int centerX = gfx->width() / 2;
  int centerY = gfx->height() / 2;
  
  // Ball bounces around the screen
  int ballX = centerX + sin(animationTime * 0.8) * (centerX - 30);
  int ballY = centerY + cos(animationTime * 1.2) * (centerY - 30);
  
  // Ball size pulses
  int ballSize = 15 + sin(animationTime * 3) * 5;
  
  // Color cycles through rainbow
  uint8_t red = 128 + 127 * sin(animationTime * 0.7);
  uint8_t green = 128 + 127 * sin(animationTime * 0.7 + 2.1);
  uint8_t blue = 128 + 127 * sin(animationTime * 0.7 + 4.2);
  
  uint16_t ballColor = gfx->color565(red, green, blue);
  
  // Draw ball with trail effect
  for (int trail = 3; trail >= 0; trail--) {
    float trailTime = animationTime - trail * 0.1;
    
    int trailX = centerX + sin(trailTime * 0.8) * (centerX - 30);
    int trailY = centerY + cos(trailTime * 1.2) * (centerY - 30);
    int trailSize = (15 + sin(trailTime * 3) * 5) - trail * 2;
    
    // Fade trail
    uint8_t trailBrightness = 255 - trail * 60;
    uint16_t trailColor = gfx->color565(
      (red * trailBrightness) >> 8,
      (green * trailBrightness) >> 8,
      (blue * trailBrightness) >> 8
    );
    
    if (trailSize > 0) {
      gfx->fillCircle(trailX, trailY, trailSize, trailColor);
    }
  }
  
  // Draw main ball
  gfx->fillCircle(ballX, ballY, ballSize, ballColor);
  
  // Add highlight to ball for 3D effect
  gfx->fillCircle(ballX - ballSize/3, ballY - ballSize/3, ballSize/3, WHITE);
  
  // Draw some decorative elements
  drawStarField();
  
  // Display effect name and info
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(10, 10);
  gfx->printf("ESP32 Screensaver");
  
  gfx->setCursor(10, 25);
  gfx->printf("Basic Bouncing Ball");
  
  gfx->setCursor(10, gfx->height() - 20);
  gfx->printf("Time: %.1fs", animationTime);
  
  // Show memory usage
  gfx->setCursor(10, gfx->height() - 35);
  gfx->printf("Free RAM: %d bytes", ESP.getFreeHeap());
}

/*
 * Helper function - Draw animated starfield background
 */
void drawStarField() {
  // Draw some twinkling stars in background
  for (int star = 0; star < 20; star++) {
    // Use star index to create pseudo-random positions
    int starX = (star * 17) % gfx->width();
    int starY = (star * 23) % gfx->height();
    
    // Twinkle based on time
    float twinkle = sin(animationTime * 2 + star * 0.5);
    
    if (twinkle > 0.3) {
      uint8_t brightness = 100 + twinkle * 155;
      uint16_t starColor = gfx->color565(brightness, brightness, brightness);
      gfx->drawPixel(starX, starY, starColor);
      
      // Bigger stars occasionally
      if (twinkle > 0.8) {
        gfx->drawPixel(starX + 1, starY, starColor);
        gfx->drawPixel(starX, starY + 1, starColor);
      }
    }
  }
}

/*
 * Performance monitoring function
 * Call this periodically to check system health
 */
void printSystemInfo() {
  Serial.println("=== System Information ===");
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
  Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
  Serial.println("===========================");
}

/*
 * Hardware test functions
 * Use these to verify your hardware setup
 */
void testDisplay() {
  Serial.println("Testing display...");
  
  // Test basic colors
  gfx->fillScreen(RED);
  delay(500);
  gfx->fillScreen(GREEN);
  delay(500);
  gfx->fillScreen(BLUE);
  delay(500);
  gfx->fillScreen(BLACK);
  
  // Test text rendering
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->setCursor(50, 100);
  gfx->printf("Display OK!");
  
  Serial.println("Display test complete.");
}

void testTouch() {
  Serial.println("Testing touch controller...");
  Serial.println("Touch the screen to see coordinates:");
  
  unsigned long testStart = millis();
  while (millis() - testStart < 10000) { // 10 second test
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      Serial.printf("Touch: X=%d, Y=%d, Pressure=%d\n", p.x, p.y, p.z);
      
      // Visual feedback on screen
      gfx->fillCircle(map(p.x, 0, 4095, 0, gfx->width()), 
                      map(p.y, 0, 4095, 0, gfx->height()), 
                      5, WHITE);
      delay(100);
    }
    delay(50);
  }
  
  Serial.println("Touch test complete.");
}

/*
 * Usage Instructions:
 * 
 * 1. Connect your ESP32 and display according to the pin definitions above
 * 2. Upload this code using PlatformIO or Arduino IDE
 * 3. Open serial monitor at 115200 baud to see debug info
 * 4. Watch the bouncing ball animation with starfield background
 * 5. Touch the screen to reset animation and see touch coordinates
 * 
 * This example demonstrates:
 * - Basic graphics operations (circles, pixels, text)
 * - Smooth animation with proper timing
 * - Color manipulation and RGB565 format
 * - Touch input handling
 * - Performance monitoring
 * - Hardware testing functions
 * 
 * Modify this code to experiment with your own effects!
 */
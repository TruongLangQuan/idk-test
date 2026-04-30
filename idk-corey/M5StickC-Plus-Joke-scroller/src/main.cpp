// Universal M5StickC Joke Scroller - Portal Hater Edition
// Universal compatibility layer preserves all original CYD functionality

#include <M5Unified.h>
#include <WiFi.h>
#include <Preferences.h>
#include "jokes.h"  // External joke collection

// Universal M5StickC compatibility layer (from our proven screensaver template)
class GFXCompatibility {
public:
  int width() { return 240; }
  int height() { return 135; }
  
  // Core drawing functions - M5Unified uses M5.Display
  void fillScreen(uint16_t color) { M5.Display.fillScreen(color); }
  void drawPixel(int16_t x, int16_t y, uint16_t color) { M5.Display.drawPixel(x, y, color); }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { M5.Display.drawLine(x0, y0, x1, y1, color); }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { M5.Display.drawRect(x, y, w, h, color); }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { M5.Display.fillRect(x, y, w, h, color); }
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { M5.Display.drawCircle(x0, y0, r, color); }
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { M5.Display.fillCircle(x0, y0, r, color); }
  
  // Color conversion function
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return M5.Display.color565(r, g, b);
  }
  
  // Display control - M5Unified uses M5.Display
  void setRotation(uint8_t r) { M5.Display.setRotation(r); }
  void setCursor(int16_t x, int16_t y) { M5.Display.setCursor(x, y); }
  void setTextColor(uint16_t c) { M5.Display.setTextColor(c); }
  void setTextColor(uint16_t c, uint16_t bg) { M5.Display.setTextColor(c, bg); }
  void setTextSize(uint8_t s) { M5.Display.setTextSize(s); }
  void print(const char* text) { M5.Display.print(text); }
  void println(const char* text) { M5.Display.println(text); }
  void println() { M5.Display.println(); }
  void write(uint8_t c) { M5.Display.write(c); }
  void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    M5.Display.print(buffer);
  }
};

GFXCompatibility gfxCompat;
GFXCompatibility* gfx = &gfxCompat; // All original gfx-> calls now work!

Preferences preferences;

// Configuration - jokes now loaded from external file
const int numJokes = JOKE_COUNT;
const unsigned long TOUCH_DEBOUNCE = 500;
const unsigned long AUTO_SCROLL_INTERVAL = 5000; // 5 seconds per joke

// State variables  
int currentJokeIndex = 0;
unsigned long lastTouchTime = 0;
unsigned long lastAutoScrollTime = 0;
bool autoScrollEnabled = false;

// Color definitions
#define BLACK   0x0000
#define WHITE   0xFFFF  
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFD20
#define LIME    0x07E0
#define PURPLE  0x780F
#define PINK    0xF81F

// Font color cycling
uint16_t fontColors[] = {WHITE, CYAN, YELLOW, GREEN, MAGENTA, RED, ORANGE, LIME, PURPLE, PINK};
int currentColorIndex = 0;
const int numColors = sizeof(fontColors) / sizeof(fontColors[0]);

// Screen flash effects (replaces hardware LEDs that don't exist on M5StickC)
void flashScreen(uint16_t color, int duration = 100) {
  gfx->fillScreen(color);
  delay(duration);
}

void displayCurrentJoke() {
  gfx->fillScreen(BLACK);
  gfx->setCursor(5, 5);
  gfx->setTextColor(fontColors[currentColorIndex]);
  gfx->setTextSize(1);
  
  // Display joke counter
  gfx->printf("Joke %d/%d", currentJokeIndex + 1, numJokes);
  gfx->setCursor(5, 20);
  
  // Get joke from PROGMEM - direct string access
  char jokeBuffer[200];
  strcpy_P(jokeBuffer, jokes[currentJokeIndex]);
  
  // Word wrap for M5StickC screen (240x135)  
  gfx->setTextSize(2); // Smaller font for long jokes - half the size of previous size 3
  gfx->setTextColor(fontColors[currentColorIndex]); // Use current color for main text too
  int x = 5, y = 35;
  int maxWidth = 230; // Leave margins
  int lineHeight = 16; // Adjusted line height for size 2 text
  
  char* word = strtok(jokeBuffer, " ");
  String currentLine = "";
  
  while (word != NULL) {
    String testLine = currentLine + (currentLine.length() > 0 ? " " : "") + word;
    
    // Estimate text width (approximate - 12 pixels per character at size 2)
    if (testLine.length() * 12 > maxWidth && currentLine.length() > 0) {
      // Print current line and start new one
      gfx->setCursor(x, y);
      gfx->print(currentLine.c_str());
      y += lineHeight;
      currentLine = word;
    } else {
      currentLine = testLine;
    }
    word = strtok(NULL, " ");
  }
  
  // Print final line
  if (currentLine.length() > 0) {
    gfx->setCursor(x, y);
    gfx->print(currentLine.c_str());
  }
  
  // Display controls at bottom
  gfx->setCursor(5, 120);
  gfx->setTextSize(1);
  gfx->setTextColor(CYAN);
  gfx->print("A:Next B:Auto C:Prev");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Universal M5StickC Joke Scroller - Portal Hater Edition");
  
  // Initialize M5Unified - auto-detects Plus1.1 vs Plus2
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // Initialize random seed for random joke selection
  randomSeed(analogRead(0) + millis());
  
  // Auto-detect device type and report
  const char* deviceName;
  switch (M5.getBoard()) {
    case m5::board_t::board_M5StickCPlus:
      deviceName = "M5StickC Plus1.1";
      break;
    case m5::board_t::board_M5StickCPlus2:
      deviceName = "M5StickC Plus2";
      break;
    default:
      deviceName = "M5StickC Unknown";
      break;
  }
  Serial.printf("Device detected: %s\n", deviceName);
  
  gfx->setRotation(3); // Landscape mode for both Plus1 and Plus2
  gfx->fillScreen(BLACK);
  
  Serial.printf("Display size: %dx%d\n", gfx->width(), gfx->height());
  
  // Initialize preferences
  preferences.begin("jokes", false);
  currentJokeIndex = preferences.getInt("jokeIndex", 0);
  currentColorIndex = preferences.getInt("colorIndex", 0);
  
  // Ensure valid indices
  if (currentJokeIndex >= numJokes) {
    currentJokeIndex = 0;
  }
  if (currentColorIndex >= numColors) {
    currentColorIndex = 0;
  }
  
  Serial.printf("Starting with joke %d of %d\n", currentJokeIndex + 1, numJokes);
  Serial.printf("Total jokes loaded: %d\n", JOKE_COUNT);
  Serial.println("Controls:");
  Serial.println("  Button A: Next joke");
  Serial.println("  Button B: Toggle auto-scroll");
  Serial.println("  Button C: Previous joke");
  Serial.println("  Power Button: Cycle font colors");
  
  // Show startup screen flash (green = ready)
  flashScreen(GREEN, 200);
  
  displayCurrentJoke();
  lastAutoScrollTime = millis();
}

void loop() {
  M5.update(); // Update M5 button states
  
  // Button A: Next joke (replaces touch)
  if (M5.BtnA.wasPressed()) {
    if (millis() - lastTouchTime > TOUCH_DEBOUNCE) {
      // Flash green screen on interaction
      flashScreen(GREEN, 50);
      
      // Next joke
      currentJokeIndex = (currentJokeIndex + 1) % numJokes;
      displayCurrentJoke();
      
      // Save position
      preferences.putInt("jokeIndex", currentJokeIndex);
      
      lastTouchTime = millis();
      lastAutoScrollTime = millis(); // Reset auto-scroll timer
    }
  }
  
  // Button B: Toggle auto-scroll (replaces boot button hold)
  if (M5.BtnB.wasPressed()) {
    autoScrollEnabled = !autoScrollEnabled;
    lastAutoScrollTime = millis();
    
    Serial.printf("Auto-scroll %s\n", autoScrollEnabled ? "ON" : "OFF");
    
    // Flash blue for auto-scroll toggle
    flashScreen(autoScrollEnabled ? BLUE : MAGENTA, 200);
    
    // Show status briefly
    gfx->fillRect(0, 0, 240, 20, BLACK);
    gfx->setCursor(5, 5);
    gfx->setTextColor(YELLOW);
    gfx->setTextSize(1);
    gfx->printf("Auto-scroll: %s", autoScrollEnabled ? "ON" : "OFF");
    delay(1000);
    
    displayCurrentJoke();
  }
  
  // Button C: Previous joke (bonus functionality)
  if (M5.BtnC.wasPressed()) {
    if (millis() - lastTouchTime > TOUCH_DEBOUNCE) {
      // Flash cyan screen on reverse
      flashScreen(CYAN, 50);
      
      // Previous joke
      currentJokeIndex = (currentJokeIndex - 1 + numJokes) % numJokes;
      displayCurrentJoke();
      
      // Save position
      preferences.putInt("jokeIndex", currentJokeIndex);
      
      lastTouchTime = millis();
      lastAutoScrollTime = millis(); // Reset auto-scroll timer
    }
  }
  
  // Power button: Cycle font colors
  if (M5.BtnPWR.wasPressed()) {
    // Cycle to next color
    currentColorIndex = (currentColorIndex + 1) % numColors;
    
    // Flash screen with new color to show change
    flashScreen(fontColors[currentColorIndex], 100);
    
    // Redisplay joke with new color
    displayCurrentJoke();
    
    // Save color preference
    preferences.putInt("colorIndex", currentColorIndex);
    
    Serial.printf("Font color changed to index %d\n", currentColorIndex);
  }
  
  // Auto-scroll functionality
  if (autoScrollEnabled && (millis() - lastAutoScrollTime >= AUTO_SCROLL_INTERVAL)) {
    // Flash yellow for auto-advance
    flashScreen(YELLOW, 50);
    
    // Random joke selection for auto-scroll
    int newJokeIndex;
    do {
      newJokeIndex = random(0, numJokes);
    } while (newJokeIndex == currentJokeIndex && numJokes > 1); // Avoid same joke
    
    currentJokeIndex = newJokeIndex;
    displayCurrentJoke();
    
    // Save position
    preferences.putInt("jokeIndex", currentJokeIndex);
    
    lastAutoScrollTime = millis();
    Serial.printf("Auto-scrolled randomly to joke %d\n", currentJokeIndex + 1);
  }
  
  delay(50); // Small delay for responsiveness
}
#include <M5StickCPlus2.h>
#include <Preferences.h>
#include "UNIT_MiniJoyC.h"

// ========== DISPLAY SETUP ==========
#define Disp M5.Lcd

// ========== JOY C POSITION CONSTANTS ==========
#define POS_X 0
#define POS_Y 1

// ========== PAINT COLORS ==========
#define PAINT_BLACK     BLACK
#define PAINT_WHITE     WHITE
#define PAINT_RED       0xF800
#define PAINT_GREEN     0x07E0
#define PAINT_BLUE      0x001F
#define PAINT_YELLOW    0xFFE0
#define PAINT_CYAN      0x07FF
#define PAINT_MAGENTA   0xF81F
#define PAINT_ORANGE    0xFC00
#define PAINT_PURPLE    0x8010
#define PAINT_PINK      0xF81F
#define PAINT_GRAY      0x8410

// ========== APP STATES ==========
enum AppState {
  WELCOME_SCREEN,
  PAINT_MODE,
  COLOR_MENU,
  BRUSH_MENU,
  // Add your app-specific states here
};

// ========== GLOBAL VARIABLES ==========
AppState currentState = WELCOME_SCREEN;
Preferences preferences;
unsigned long lastUpdate = 0;
int selectedMenuItem = 0;

// ========== PAINT VARIABLES ==========
UNIT_JOYC Joystick;
int cursorX = 67;  // Center of 135px width
int cursorY = 120; // Center of 240px height
int brushSize = 2;
uint16_t brushColor = PAINT_RED;
uint16_t backgroundColor = PAINT_WHITE;
bool drawMode = false;
bool drawEnabled = false; // New: tracks if drawing is enabled
unsigned long lastCursorUpdate = 0;
bool showCursor = true;
bool wasDrawing = false;
int lastDrawX = -1;
int lastDrawY = -1;
unsigned long lastJoyPress = 0;
bool joyButtonPressed = false;
// Cursor erase tracking
int lastCursorX = -1;
int lastCursorY = -1;
// Button timing for different actions
unsigned long joyPressStartTime = 0;
bool joyPressHandled = false;

// Color palette (now includes background colors)
uint16_t colorPalette[] = {
  PAINT_BLACK, PAINT_WHITE, PAINT_RED, PAINT_GREEN, 
  PAINT_BLUE, PAINT_YELLOW, PAINT_CYAN, PAINT_MAGENTA,
  PAINT_ORANGE, PAINT_PURPLE, PAINT_PINK, PAINT_GRAY
};
int colorCount = 12;
int selectedColor = 2; // Start with red

// Background colors
uint16_t backgroundPalette[] = {
  PAINT_WHITE, PAINT_BLACK, 0x18C3, 0xF79E, 0x07FF, 0xFFE0
};
int backgroundColorCount = 6;
int selectedBackgroundColor = 0; // Start with white

// Brush sizes
int brushSizes[] = {1, 2, 3, 4, 5, 8};
int brushSizeCount = 6;
int selectedBrushSize = 1; // Start with size 2

// ========== FUNCTION DECLARATIONS ==========
void drawWelcomeScreen();
void drawPaintMode();
void drawColorMenu();
void drawBrushMenu();
void handleWelcomeScreen();
void handlePaintMode();
void handleColorMenu();
void handleBrushMenu();
void saveSettings();
void loadSettings();
void drawText(String text, int x, int y, uint16_t color, int size = 1);
void drawCenteredText(String text, int y, uint16_t color, int size = 1);
void updateCursor();
void drawLine();
void clearCanvas();
void showColorPalette();
void showBrushSizes();
void cycleColor();
void drawThinCursor();
void drawDemoArtwork();

// ========== JOY C HELPER FUNCTIONS ==========
bool isJoyUp();
bool isJoyDown();
bool isJoyLeft();
bool isJoyRight();
bool isJoyPressed();

// ========== SETUP FUNCTION ==========
void setup() {
  // M5StickC Plus2 configuration - PROVEN WORKING
  auto cfg = M5.config();
  
  // Power management - prevents button/display issues
  cfg.clear_display = true;
  cfg.output_power = false;
  cfg.internal_imu = false;  
  cfg.internal_rtc = false;
  cfg.internal_spk = false;
  cfg.internal_mic = false;
  cfg.serial_baudrate = 0; // Disable serial
  
  M5.begin(cfg);
  
  // ESSENTIAL: Portrait mode for M5StickC Plus2 (135x240)
  Disp.setRotation(0);
  Disp.fillScreen(PAINT_BLACK);
  
  // Initialize preferences for data persistence
  preferences.begin("paint_app", false);
  
  // Initialize JoyC
  Disp.setTextColor(PAINT_WHITE);
  Disp.setTextSize(1);
  Disp.drawString("Initializing Joy C...", 10, 100);
  
  while (!(Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL))) {
    delay(100);
    // Serial.println("Joy C I2C Error!");
  }
  
  // Set Joy C LED to green to indicate successful connection
  Joystick.setLEDColor(0, 0x00FF00);
  
  // Load saved settings
  loadSettings();
  
  // Display welcome screen
  drawWelcomeScreen();
}

// ========== MAIN LOOP ==========
void loop() {
  M5.update(); // ALWAYS call first
  
  // Handle different app states
  switch(currentState) {
    case WELCOME_SCREEN:
      handleWelcomeScreen();
      break;
    case PAINT_MODE:
      handlePaintMode();
      break;
    case COLOR_MENU:
      handleColorMenu();
      break;
    case BRUSH_MENU:
      handleBrushMenu();
      break;
  }
  
  delay(30); // Smoother for painting
}

// ========== UI DRAWING FUNCTIONS ==========

void drawWelcomeScreen() {
  Disp.fillScreen(PAINT_BLACK);
  
  // Title
  drawCenteredText("PAINT PROGRAM", 30, PAINT_CYAN, 2);
  drawCenteredText("v1.3", 55, PAINT_WHITE, 1);
  
  // Instructions
  drawText("M5: Start Paint", 10, 80, PAINT_GREEN, 1);
  drawText("A: Brush Size", 10, 95, PAINT_YELLOW, 1);
  drawText("B: Colors", 10, 110, PAINT_YELLOW, 1);
  drawText("Joy: Move/Color", 10, 125, PAINT_WHITE, 1);
  drawText("Hold Joy: Draw", 10, 140, PAINT_WHITE, 1);
  
  // Battery indicator
  drawText("Battery:", 10, 8, PAINT_GRAY, 1);
  drawText(String(M5.Power.getBatteryLevel()) + "%", 70, 8, PAINT_WHITE, 1);
  
  // Draw demo artwork - simple shapes to show capabilities
  drawDemoArtwork();
}

void drawDemoArtwork() {
  // Small demo artwork at bottom to show paint program capabilities
  int centerX = 67;
  int baseY = 200;
  
  // Draw a simple flower in a pot
  // Pot (brown rectangle)
  uint16_t brown = 0x8200;
  Disp.fillRect(centerX - 8, baseY - 5, 16, 10, brown);
  
  // Stem (green line)
  Disp.drawLine(centerX, baseY - 5, centerX, baseY - 20, PAINT_GREEN);
  
  // Flower petals (red circles around center)
  uint16_t pink = 0xF81F;
  int flowerX = centerX;
  int flowerY = baseY - 20;
  
  // 5 petals around center
  Disp.fillCircle(flowerX - 3, flowerY - 2, 2, pink);     // Left
  Disp.fillCircle(flowerX + 3, flowerY - 2, 2, pink);     // Right  
  Disp.fillCircle(flowerX, flowerY - 4, 2, pink);         // Top
  Disp.fillCircle(flowerX - 2, flowerY + 1, 2, pink);     // Bottom left
  Disp.fillCircle(flowerX + 2, flowerY + 1, 2, pink);     // Bottom right
  
  // Flower center (yellow)
  Disp.fillCircle(flowerX, flowerY, 1, PAINT_YELLOW);
  
  // Leaves (small green circles)
  Disp.fillCircle(centerX - 4, baseY - 12, 1, PAINT_GREEN);
  Disp.fillCircle(centerX + 4, baseY - 15, 1, PAINT_GREEN);
  
  // Small decorative shapes to show brush capabilities
  // Small hearts or stars around the sides
  drawText("♥", 20, baseY - 15, PAINT_RED, 1);
  drawText("★", 100, baseY - 12, PAINT_YELLOW, 1);
  drawText("♦", 25, baseY - 5, PAINT_BLUE, 1);
  drawText("●", 110, baseY - 5, PAINT_CYAN, 1);
}

void drawPaintMode() {
  // Don't clear screen in paint mode - preserve artwork
  // Just draw status bar
  Disp.fillRect(0, 0, 135, 18, PAINT_BLACK);
  
  // Status bar
  drawText("Paint", 5, 5, PAINT_WHITE, 1);
  drawText(drawEnabled ? "DRAW" : "MOVE", 40, 5, drawEnabled ? PAINT_GREEN : PAINT_GRAY, 1);
  drawText("A:Size B:Color", 70, 5, PAINT_GRAY, 1);
  
  // Draw brush preview at bottom with size info
  Disp.fillRect(0, 220, 135, 20, PAINT_BLACK);
  Disp.fillCircle(15, 230, brushSize, brushColor);
  drawText("Size:" + String(brushSize), 25, 225, PAINT_GRAY, 1);
  drawText(String(M5.Power.getBatteryLevel()) + "%", 100, 225, PAINT_WHITE, 1);
}

void drawColorMenu() {
  Disp.fillScreen(PAINT_BLACK);
  
  // Title
  drawCenteredText("SELECT COLOR", 20, PAINT_CYAN, 1);
  
  // Draw color palette in a grid
  int cols = 4;
  int rows = 3;
  int startX = 15;
  int startY = 50;
  int colorSize = 20;
  int spacing = 25;
  
  for(int i = 0; i < colorCount; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = startX + col * spacing;
    int y = startY + row * spacing;
    
    // Draw color square
    Disp.fillRect(x, y, colorSize, colorSize, colorPalette[i]);
    
    // Draw selection indicator
    if(i == selectedColor) {
      Disp.drawRect(x-2, y-2, colorSize+4, colorSize+4, PAINT_WHITE);
      Disp.drawRect(x-3, y-3, colorSize+6, colorSize+6, PAINT_WHITE);
    }
  }
  
  // Controls
  drawText("Joy: Select  M5: OK  B: Back", 5, 200, PAINT_GRAY, 1);
}

void drawBrushMenu() {
  Disp.fillScreen(PAINT_BLACK);
  
  // Title
  drawCenteredText("BRUSH & BACKGROUND", 20, PAINT_CYAN, 1);
  
  // Draw brush sizes
  drawText("BRUSH SIZE:", 10, 45, PAINT_WHITE, 1);
  int startY = 65;
  for(int i = 0; i < brushSizeCount; i++) {
    int y = startY + i * 20;
    
    // Draw brush preview
    Disp.fillCircle(30, y + 6, brushSizes[i], brushColor);
    
    // Draw size text
    String sizeText = "Size " + String(brushSizes[i]);
    uint16_t textColor = (i == selectedBrushSize) ? PAINT_GREEN : PAINT_WHITE;
    drawText((i == selectedBrushSize ? "> " : "  ") + sizeText, 50, y, textColor, 1);
  }
  
  // Draw background colors
  drawText("BACKGROUND:", 10, 180, PAINT_WHITE, 1);
  int bgStartX = 10;
  int bgY = 195;
  for(int i = 0; i < backgroundColorCount; i++) {
    int x = bgStartX + i * 20;
    
    // Draw background color square
    Disp.fillRect(x, bgY, 15, 15, backgroundPalette[i]);
    
    // Draw selection indicator
    if(i == selectedBackgroundColor) {
      Disp.drawRect(x-1, bgY-1, 17, 17, PAINT_WHITE);
    }
  }
  
  // Controls
  drawText("Joy: Select  M5: OK  B: Back", 5, 220, PAINT_GRAY, 1);
}

// ========== INPUT HANDLING ==========

void handleWelcomeScreen() {
  if (M5.BtnA.wasClicked()) {
    currentState = PAINT_MODE;
    Disp.fillScreen(backgroundColor); // Start with selected background color
    drawPaintMode();
  }
  
  if (M5.BtnB.wasClicked()) {
    // Could add settings or exit functionality
  }
  
  if (M5.BtnPWR.wasClicked()) {
    currentState = COLOR_MENU;
    drawColorMenu();
  }
}

void handlePaintMode() {
  // Update cursor position based on joystick
  updateCursor();
  
  // Handle joystick button with timing for different actions
  bool currentJoyPressed = isJoyPressed();
  
  if (currentJoyPressed && !joyButtonPressed) {
    // Button just pressed - start timing
    joyPressStartTime = millis();
    joyPressHandled = false;
    joyButtonPressed = true;
  }
  else if (!currentJoyPressed && joyButtonPressed) {
    // Button just released - determine action based on press duration
    unsigned long pressDuration = millis() - joyPressStartTime;
    
    if (!joyPressHandled) {
      if (pressDuration < 800) { // Short press
        // Short press - cycle color
        cycleColor();
      }
      // Long press action was already handled during the hold
    }
    joyButtonPressed = false;
  }
  else if (currentJoyPressed && joyButtonPressed) {
    // Button being held - check for long press
    unsigned long pressDuration = millis() - joyPressStartTime;
    
    if (pressDuration >= 800 && !joyPressHandled) { // Long press
      // Long press detected - toggle draw mode
      drawEnabled = !drawEnabled;
      joyPressHandled = true;
      
      if (drawEnabled) {
        // Starting to draw - reset position and set LED to brush color
        lastDrawX = -1;
        lastDrawY = -1;
        Joystick.setLEDColor(0, (brushColor >> 8) | ((brushColor & 0xFF) << 8));
      } else {
        // Stopped drawing - set LED to dim white
        Joystick.setLEDColor(0, 0x101010); // Dim white
      }
    }
  }
  
  // Draw lines when drawing is enabled
  drawLine();
  
  // Button controls - NEW LAYOUT
  if (M5.BtnA.wasClicked()) {
    // BtnA now cycles brush size
    selectedBrushSize = (selectedBrushSize + 1) % brushSizeCount;
    brushSize = brushSizes[selectedBrushSize];
  }
  
  if (M5.BtnB.wasClicked()) {
    // BtnB opens color menu
    currentState = COLOR_MENU;
    drawColorMenu();
  }
  
  if (M5.BtnPWR.wasClicked()) {
    // PWR clears canvas
    clearCanvas();
    drawPaintMode();
  }
  
  // Draw cursor with draw mode indication
  drawThinCursor();
}

void handleColorMenu() {
  static unsigned long lastJoyMove = 0;
  
  if (millis() - lastJoyMove > 150) { // Debounce joystick
    if (isJoyLeft() && selectedColor > 0) {
      selectedColor--;
      drawColorMenu();
      lastJoyMove = millis();
    }
    else if (isJoyRight() && selectedColor < colorCount - 1) {
      selectedColor++;
      drawColorMenu();
      lastJoyMove = millis();
    }
    else if (isJoyUp() && selectedColor >= 4) {
      selectedColor -= 4;
      drawColorMenu();
      lastJoyMove = millis();
    }
    else if (isJoyDown() && selectedColor < colorCount - 4) {
      selectedColor += 4;
      drawColorMenu();
      lastJoyMove = millis();
    }
  }
  
  if (M5.BtnA.wasClicked() || isJoyPressed()) {
    brushColor = colorPalette[selectedColor];
    Joystick.setLEDColor(0, (brushColor >> 8) | ((brushColor & 0xFF) << 8));
    currentState = PAINT_MODE;
    // Don't redraw paint mode to preserve canvas
  }
  
  if (M5.BtnB.wasClicked()) {
    currentState = PAINT_MODE;
    // Don't redraw paint mode to preserve canvas
  }
}

void handleBrushMenu() {
  static unsigned long lastJoyMove = 0;
  
  if (millis() - lastJoyMove > 150) { // Debounce joystick
    if (isJoyUp() && selectedBrushSize > 0) {
      selectedBrushSize--;
      drawBrushMenu();
      lastJoyMove = millis();
    }
    else if (isJoyDown() && selectedBrushSize < brushSizeCount - 1) {
      selectedBrushSize++;
      drawBrushMenu();
      lastJoyMove = millis();
    }
    else if (isJoyLeft() && selectedBackgroundColor > 0) {
      selectedBackgroundColor--;
      drawBrushMenu();
      lastJoyMove = millis();
    }
    else if (isJoyRight() && selectedBackgroundColor < backgroundColorCount - 1) {
      selectedBackgroundColor++;
      drawBrushMenu();
      lastJoyMove = millis();
    }
  }
  
  if (M5.BtnA.wasClicked() || isJoyPressed()) {
    brushSize = brushSizes[selectedBrushSize];
    backgroundColor = backgroundPalette[selectedBackgroundColor];
    currentState = PAINT_MODE;
    // Don't redraw paint mode to preserve canvas
  }
  
  if (M5.BtnB.wasClicked()) {
    currentState = PAINT_MODE;
    // Don't redraw paint mode to preserve canvas
  }
}

// ========== UTILITY FUNCTIONS ==========

void drawText(String text, int x, int y, uint16_t color, int size) {
  Disp.setTextColor(color);
  Disp.setTextSize(size);
  Disp.drawString(text, x, y);
}

void drawCenteredText(String text, int y, uint16_t color, int size) {
  Disp.setTextColor(color);
  Disp.setTextSize(size);
  int textWidth = text.length() * 6 * size; // Approximate
  int x = (135 - textWidth) / 2;
  Disp.drawString(text, x, y);
}

// ========== PAINT SPECIFIC FUNCTIONS ==========

void updateCursor() {
  // Read joystick values (12-bit ADC: 0-4095)
  uint16_t joyX = Joystick.getADCValue(POS_X);
  uint16_t joyY = Joystick.getADCValue(POS_Y);
  
  // Define center position and dead zone
  const int CENTER = 2048;  // 12-bit center
  const int DEAD_ZONE = 150; // Larger dead zone for stability
  const float SENSITIVITY = 0.04; // Reduced from 0.08 for slower movement
  
  // Calculate offset from center
  int offsetX = joyX - CENTER;
  int offsetY = joyY - CENTER;
  
  // Apply dead zone
  if (abs(offsetX) < DEAD_ZONE) offsetX = 0;
  if (abs(offsetY) < DEAD_ZONE) offsetY = 0;
  
  // Convert to movement with smooth scaling
  float deltaX = offsetX * SENSITIVITY;
  float deltaY = -offsetY * SENSITIVITY; // Invert Y for natural movement
  
  // Apply gentler non-linear scaling for better control
  // Reduced the multiplier for slower max speed
  if (abs(deltaX) > 1) {
    deltaX = deltaX > 0 ? deltaX * deltaX * 0.005 + 0.5 : deltaX * deltaX * -0.005 - 0.5;
  }
  if (abs(deltaY) > 1) {
    deltaY = deltaY > 0 ? deltaY * deltaY * 0.005 + 0.5 : deltaY * deltaY * -0.005 - 0.5;
  }
  
  // Update cursor position with smooth movement
  cursorX += (int)deltaX;
  cursorY += (int)deltaY;
  
  // Constrain to screen bounds (leave room for UI)
  cursorX = constrain(cursorX, brushSize + 2, 135 - brushSize - 2);
  cursorY = constrain(cursorY, 20 + brushSize + 2, 215 - brushSize - 2);
}

void drawLine() {
  // Only draw if drawing is enabled
  if (!drawEnabled) {
    // Reset drawing position when not drawing
    lastDrawX = -1;
    lastDrawY = -1;
    return;
  }
  
  // Simple, working line drawing - back to basics
  if (lastDrawX != -1 && lastDrawY != -1) {
    // Draw line from last position to current position
    if (brushSize == 1) {
      Disp.drawLine(lastDrawX, lastDrawY, cursorX, cursorY, brushColor);
    } else {
      // For thicker brushes, draw filled circles along the path
      int dx = abs(cursorX - lastDrawX);
      int dy = abs(cursorY - lastDrawY);
      int steps = max(dx, dy);
      
      if (steps > 0 && steps < 20) { // Reasonable step limit
        for (int i = 0; i <= steps; i++) {
          int x = lastDrawX + (cursorX - lastDrawX) * i / steps;
          int y = lastDrawY + (cursorY - lastDrawY) * i / steps;
          Disp.fillCircle(x, y, brushSize / 2, brushColor);
        }
      }
    }
  } else {
    // First point when starting to draw
    if (brushSize == 1) {
      Disp.drawPixel(cursorX, cursorY, brushColor);
    } else {
      Disp.fillCircle(cursorX, cursorY, brushSize / 2, brushColor);
    }
  }
  
  // Update last draw position
  lastDrawX = cursorX;
  lastDrawY = cursorY;
}

void clearCanvas() {
  // Clear the drawing area with background color but preserve UI
  Disp.fillRect(0, 20, 135, 195, backgroundColor);
  // Reset drawing position
  lastDrawX = -1;
  lastDrawY = -1;
}

void cycleColor() {
  // Cycle to next color
  selectedColor = (selectedColor + 1) % colorCount;
  brushColor = colorPalette[selectedColor];
  
  // Update LED color
  Joystick.setLEDColor(0, (brushColor >> 8) | ((brushColor & 0xFF) << 8));
}

void drawThinCursor() {
  // Only show cursor when NOT drawing to avoid cursor artifacts
  if (drawEnabled) {
    // Erase any old cursor when drawing is enabled
    if (lastCursorX != -1 && lastCursorY != -1) {
      // Erase old cursor by redrawing background
      int size = 5;
      Disp.drawLine(lastCursorX - size, lastCursorY, lastCursorX + size, lastCursorY, backgroundColor);
      Disp.drawLine(lastCursorX, lastCursorY - size, lastCursorX, lastCursorY + size, backgroundColor);
      Disp.fillCircle(lastCursorX, lastCursorY, 1, backgroundColor);
    }
    lastCursorX = -1;
    lastCursorY = -1;
    return;
  }
  
  // Erase old cursor position before drawing new one
  if (lastCursorX != -1 && lastCursorY != -1) {
    int size = 3;
    Disp.drawLine(lastCursorX - size, lastCursorY, lastCursorX + size, lastCursorY, backgroundColor);
    Disp.drawLine(lastCursorX, lastCursorY - size, lastCursorX, lastCursorY + size, backgroundColor);
  }
  
  // Draw cursor with blinking
  if (millis() - lastCursorUpdate > 500) {
    showCursor = !showCursor;
    lastCursorUpdate = millis();
  }
  
  if (showCursor) {
    // Choose cursor color - make sure it's visible
    uint16_t cursorColor = PAINT_GRAY;
    if (backgroundColor == PAINT_GRAY) {
      cursorColor = PAINT_WHITE;
    }
    
    // Draw thin crosshair
    int size = 3;
    Disp.drawLine(cursorX - size, cursorY, cursorX + size, cursorY, cursorColor);
    Disp.drawLine(cursorX, cursorY - size, cursorX, cursorY + size, cursorColor);
    
    // Remember this position for erasing
    lastCursorX = cursorX;
    lastCursorY = cursorY;
  } else {
    // Don't remember position when not showing
    lastCursorX = -1;
    lastCursorY = -1;
  }
}

// ========== JOY C HELPER FUNCTIONS ==========

bool isJoyUp() {
  return Joystick.getADCValue(POS_Y) > 3000; // More sensitive threshold
}

bool isJoyDown() {
  return Joystick.getADCValue(POS_Y) < 1200;
}

bool isJoyLeft() {
  return Joystick.getADCValue(POS_X) < 1200;
}

bool isJoyRight() {
  return Joystick.getADCValue(POS_X) > 3000;
}

bool isJoyPressed() {
  return Joystick.getButtonStatus() == 0; // Button is active low
}

// ========== DATA PERSISTENCE ==========

void saveSettings() {
  // Save paint app settings
  preferences.putInt("brushSize", brushSize);
  preferences.putInt("brushColor", brushColor);
  preferences.putInt("backgroundColor", backgroundColor);
  preferences.putInt("selectedColor", selectedColor);
  preferences.putInt("selectedBrushSize", selectedBrushSize);
  preferences.putInt("selectedBackgroundColor", selectedBackgroundColor);
  preferences.putBool("settingsSet", true);
}

void loadSettings() {
  // Load paint app settings
  if (preferences.getBool("settingsSet", false)) {
    brushSize = preferences.getInt("brushSize", 2);
    brushColor = preferences.getInt("brushColor", PAINT_RED);
    backgroundColor = preferences.getInt("backgroundColor", PAINT_WHITE);
    selectedColor = preferences.getInt("selectedColor", 2);
    selectedBrushSize = preferences.getInt("selectedBrushSize", 1);
    selectedBackgroundColor = preferences.getInt("selectedBackgroundColor", 0);
  }
}
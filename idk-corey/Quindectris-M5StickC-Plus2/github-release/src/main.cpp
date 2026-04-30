// main.cpp - ThreeTris Main Entry Point
#include <M5StickCPlus2.h>
#include "display.h"
#include "input.h"
#include "tetris.h"

enum GameState {
  TITLE_SCREEN,
  PLAYING,
  GAME_OVER
};

GameState currentState = TITLE_SCREEN;

void showTitleScreen() {
  static bool firstDraw = true;
  static int animPieces[3]; // Random pieces to show
  static int animColors[3];
  static int pieceY[3] = {100, 100, 100}; // Y positions for animation
  static unsigned long lastAnim = 0;
  
  if (firstDraw) {
    M5.Lcd.fillScreen(0x0010); // Dark blue background
    
    // Draw border
    M5.Lcd.drawRect(2, 2, 131, 236, COLOR_CYAN);
    M5.Lcd.drawRect(3, 3, 129, 234, COLOR_CYAN);
    
    // Pick 3 random pieces to display
    for (int i = 0; i < 3; i++) {
      animPieces[i] = random(0, 8);
      animColors[i] = random(0, 6);
      pieceY[i] = 100 + (i * 10);
    }
    firstDraw = false;
  }
  
  // Animate pieces - make them bounce slightly
  if (millis() - lastAnim > 100) {
    for (int i = 0; i < 3; i++) {
      pieceY[i] += 2;
      if (pieceY[i] > 135) pieceY[i] = 100;
    }
    lastAnim = millis();
  }
  
  // Draw "THREETRIS" vertically in better block letters
  int bs = 5; // block size
  int x = 15; // X position for letters
  int y = 15; // Starting Y
  
  // Draw each letter vertically with better spacing
  // Use simpler 5x5 grid for each letter
  
  // T (row 1)
  M5.Lcd.fillRect(x, y, bs*5, bs, COLOR_CYAN);
  M5.Lcd.fillRect(x+bs*2, y+bs, bs, bs*4, COLOR_CYAN);
  
  // H (row 2)
  y += bs*6;
  M5.Lcd.fillRect(x, y, bs, bs*5, COLOR_GREEN);
  M5.Lcd.fillRect(x+bs*4, y, bs, bs*5, COLOR_GREEN);
  M5.Lcd.fillRect(x, y+bs*2, bs*5, bs, COLOR_GREEN);
  
  // R (row 3)
  y += bs*6;
  M5.Lcd.fillRect(x, y, bs, bs*5, COLOR_RED);
  M5.Lcd.fillRect(x, y, bs*4, bs, COLOR_RED);
  M5.Lcd.fillRect(x+bs*4, y+bs, bs, bs, COLOR_RED);
  M5.Lcd.fillRect(x, y+bs*2, bs*3, bs, COLOR_RED);
  M5.Lcd.fillRect(x+bs*3, y+bs*3, bs, bs, COLOR_RED);
  M5.Lcd.fillRect(x+bs*4, y+bs*4, bs, bs, COLOR_RED);
  
  // E (row 4)
  y += bs*6;
  M5.Lcd.fillRect(x, y, bs, bs*5, COLOR_YELLOW);
  M5.Lcd.fillRect(x, y, bs*5, bs, COLOR_YELLOW);
  M5.Lcd.fillRect(x, y+bs*2, bs*4, bs, COLOR_YELLOW);
  M5.Lcd.fillRect(x, y+bs*4, bs*5, bs, COLOR_YELLOW);
  
  // E (row 5)
  y += bs*6;
  M5.Lcd.fillRect(x, y, bs, bs*5, COLOR_ORANGE);
  M5.Lcd.fillRect(x, y, bs*5, bs, COLOR_ORANGE);
  M5.Lcd.fillRect(x, y+bs*2, bs*4, bs, COLOR_ORANGE);
  M5.Lcd.fillRect(x, y+bs*4, bs*5, bs, COLOR_ORANGE);
  
  // Continue right column for TRIS
  x = 75;
  y = 15;
  
  // T (row 1)
  M5.Lcd.fillRect(x, y, bs*5, bs, COLOR_PURPLE);
  M5.Lcd.fillRect(x+bs*2, y+bs, bs, bs*4, COLOR_PURPLE);
  
  // R (row 2)
  y += bs*6;
  M5.Lcd.fillRect(x, y, bs, bs*5, COLOR_CYAN);
  M5.Lcd.fillRect(x, y, bs*4, bs, COLOR_CYAN);
  M5.Lcd.fillRect(x+bs*4, y+bs, bs, bs, COLOR_CYAN);
  M5.Lcd.fillRect(x, y+bs*2, bs*3, bs, COLOR_CYAN);
  M5.Lcd.fillRect(x+bs*3, y+bs*3, bs, bs, COLOR_CYAN);
  M5.Lcd.fillRect(x+bs*4, y+bs*4, bs, bs, COLOR_CYAN);
  
  // I (row 3)
  y += bs*6;
  M5.Lcd.fillRect(x, y, bs*5, bs, COLOR_GREEN);
  M5.Lcd.fillRect(x+bs*2, y+bs, bs, bs*3, COLOR_GREEN);
  M5.Lcd.fillRect(x, y+bs*4, bs*5, bs, COLOR_GREEN);
  
  // S (row 4)
  y += bs*6;
  M5.Lcd.fillRect(x, y, bs*5, bs, COLOR_RED);
  M5.Lcd.fillRect(x, y+bs, bs, bs*2, COLOR_RED);
  M5.Lcd.fillRect(x, y+bs*2, bs*5, bs, COLOR_RED);
  M5.Lcd.fillRect(x+bs*4, y+bs*3, bs, bs, COLOR_RED);
  M5.Lcd.fillRect(x, y+bs*4, bs*5, bs, COLOR_RED);
  
  // Version badge
  M5.Lcd.fillRect(48, 155, 30, 12, COLOR_WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_BLACK);
  M5.Lcd.setCursor(52, 157);
  M5.Lcd.print("v3.0");
  
  // Draw 3 animated triomino pieces - cascade in center
  uint16_t pieceColors[6] = {COLOR_CYAN, COLOR_YELLOW, COLOR_GREEN, COLOR_RED, COLOR_PURPLE, COLOR_ORANGE};
  
  // Cascade layout: pieces centered, avoiding the letters
  int pieceX[3] = {35, 50, 65}; // Spread across center, below letters
  int adjustY[3] = {-5, 5, 0}; // Staggered heights for cascade effect
  
  for (int p = 0; p < 3; p++) {
    uint16_t color = pieceColors[animColors[p]];
    int px = pieceX[p];
    int py = pieceY[p] + adjustY[p];
    int pbs = 7; // piece block size
    
    // Clear old position
    M5.Lcd.fillRect(px-1, py-3, pbs*3+2, pbs*3+3, 0x0010);
    
    // Draw actual triomino pieces from our game (8 types)
    switch (animPieces[p] % 8) {
      case 0: // I-Triomino (horizontal 3 blocks)
        M5.Lcd.fillRect(px, py, pbs*3, pbs, color);
        M5.Lcd.drawRect(px, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs*2, py, pbs, pbs, COLOR_WHITE);
        break;
      case 1: // L-Triomino (corner right)
        M5.Lcd.fillRect(px, py, pbs, pbs*2, color);
        M5.Lcd.fillRect(px+pbs, py+pbs, pbs, pbs, color);
        M5.Lcd.drawRect(px, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px, py+pbs, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs, py+pbs, pbs, pbs, COLOR_WHITE);
        break;
      case 2: // J-Triomino (corner left - mirror of L)
        M5.Lcd.fillRect(px+pbs, py, pbs, pbs*2, color);
        M5.Lcd.fillRect(px, py+pbs, pbs, pbs, color);
        M5.Lcd.drawRect(px+pbs, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs, py+pbs, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px, py+pbs, pbs, pbs, COLOR_WHITE);
        break;
      case 3: // Single block
        M5.Lcd.fillRect(px+pbs, py, pbs, pbs, color);
        M5.Lcd.drawRect(px+pbs, py, pbs, pbs, COLOR_WHITE);
        break;
      case 4: // Double block
        M5.Lcd.fillRect(px+pbs/2, py, pbs*2, pbs, color);
        M5.Lcd.drawRect(px+pbs/2, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs/2+pbs, py, pbs, pbs, COLOR_WHITE);
        break;
      case 5: // Diagonal Double
        M5.Lcd.fillRect(px, py, pbs, pbs, color);
        M5.Lcd.fillRect(px+pbs, py+pbs, pbs, pbs, color);
        M5.Lcd.drawRect(px, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs, py+pbs, pbs, pbs, COLOR_WHITE);
        break;
      case 6: // Diagonal Triple
        M5.Lcd.fillRect(px, py, pbs, pbs, color);
        M5.Lcd.fillRect(px+pbs, py+pbs, pbs, pbs, color);
        M5.Lcd.fillRect(px+pbs*2, py+pbs*2, pbs, pbs, color);
        M5.Lcd.drawRect(px, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs, py+pbs, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs*2, py+pbs*2, pbs, pbs, COLOR_WHITE);
        break;
      case 7: // T-minus-one (3 blocks, checkerboard pattern)
        M5.Lcd.fillRect(px, py, pbs*3, pbs, color);
        M5.Lcd.fillRect(px+pbs, py+pbs, pbs, pbs, color);
        M5.Lcd.drawRect(px, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs*2, py, pbs, pbs, COLOR_WHITE);
        M5.Lcd.drawRect(px+pbs, py+pbs, pbs, pbs, COLOR_WHITE);
        break;
    }
  }
  
  // Info text
  M5.Lcd.setTextColor(COLOR_WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 175);
  M5.Lcd.print("8 Unique Pieces");
  M5.Lcd.setCursor(8, 188);
  M5.Lcd.print("Endless Challenge");
  
  // Blink prompt
  static unsigned long lastBlink = 0;
  static bool showPrompt = true;
  if (millis() - lastBlink > 600) {
    showPrompt = !showPrompt;
    lastBlink = millis();
    
    // Clear prompt area
    M5.Lcd.fillRect(5, 205, 125, 30, 0x0010);
  }
  
  if (showPrompt) {
    M5.Lcd.fillRect(10, 208, 115, 22, COLOR_GREEN);
    M5.Lcd.drawRect(10, 208, 115, 22, COLOR_YELLOW);
    M5.Lcd.setTextColor(COLOR_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(15, 212);
    M5.Lcd.print("Press JoyC Btn");
    M5.Lcd.setCursor(23, 220);
    M5.Lcd.print("to START!");
  }
}

void setup() {
  // Initialize hardware
  M5.begin();
  Serial.begin(115200);
  
  // Initialize systems
  initDisplay();
  initInput();
  
  Serial.println("ThreeTris v2.5 Started!");
}

void loop() {
  // Update input
  updateInput();
  
  switch (currentState) {
    case TITLE_SCREEN:
      showTitleScreen();
      // Start game on JoyC button press
      if (buttons.joyBtnPressed) {
        M5.Lcd.fillScreen(COLOR_BLACK);
        tetrisGame.init();
        currentState = PLAYING;
      }
      break;
      
    case PLAYING:
      // Check for reset (Button B)
      if (buttons.btnBPressed) {
        M5.Lcd.fillScreen(COLOR_BLACK);
        tetrisGame.init();
      }
      
      // Update and draw game
      if (!tetrisGame.isGameOver()) {
        tetrisGame.update();
        tetrisGame.draw();
      } else {
        currentState = GAME_OVER;
      }
      break;
      
    case GAME_OVER:
      // Clear the play field area only (not entire screen)
      static bool clearedScreen = false;
      if (!clearedScreen) {
        M5.Lcd.fillRect(OFFSET_X, OFFSET_Y, FIELD_WIDTH*BLOCK_SIZE, FIELD_HEIGHT*BLOCK_SIZE, COLOR_BLACK);
        clearedScreen = true;
      }
      
      // Game over screen - stacked text
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(COLOR_RED);
      M5.Lcd.setCursor(35, 50);
      M5.Lcd.print("GAME");
      M5.Lcd.setCursor(35, 68);
      M5.Lcd.print("OVER");
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(COLOR_WHITE);
      M5.Lcd.setCursor(25, 90);
      M5.Lcd.print("Score: ");
      M5.Lcd.print(tetrisGame.getScore());
      M5.Lcd.setCursor(20, 105);
      M5.Lcd.print("B to restart");
      
      // Reset on button B
      if (buttons.btnBPressed) {
        clearedScreen = false;
        M5.Lcd.fillScreen(COLOR_BLACK);
        tetrisGame.init();
        currentState = PLAYING;
      }
      break;
  }
  
  delay(10);
}

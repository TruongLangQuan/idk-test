// main.cpp - M5StickC Plus2 Game System
#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "gamemanager.h"

// Tetromino shapes for splash screen
static const int SHAPES[7][4][2] = {
  {{0,0}, {1,0}, {2,0}, {3,0}},  // I
  {{0,0}, {1,0}, {0,1}, {1,1}},  // O
  {{1,0}, {0,1}, {1,1}, {2,1}},  // T
  {{0,1}, {1,1}, {1,0}, {2,0}},  // S
  {{0,0}, {1,0}, {1,1}, {2,1}},  // Z
  {{0,0}, {1,0}, {2,0}, {2,1}},  // L
  {{0,0}, {1,0}, {2,0}, {0,1}}   // J
};

static const uint16_t SPLASH_COLORS[7] = {
  COLOR_CYAN, COLOR_YELLOW, 0xF81F, COLOR_GREEN, COLOR_RED, COLOR_ORANGE, COLOR_BLUE
};

// TETRIS letter patterns (5x7 grid)
static const uint8_t LETTER_T[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t LETTER_E[7] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
static const uint8_t LETTER_R[7] = {0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001}; // Backwards!
static const uint8_t LETTER_I[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111};
static const uint8_t LETTER_S[7] = {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};

void drawSplashPiece(int x, int y, int type, uint16_t color) {
  for (int i = 0; i < 4; i++) {
    int px = SHAPES[type][i][0];
    int py = SHAPES[type][i][1];
    int bx = x + px * 6;
    int by = y + py * 6;
    M5.Lcd.fillRect(bx, by, 5, 5, color);
  }
}

void drawSplashLetter(int x, int y, const uint8_t* letter, uint16_t color) {
  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 5; col++) {
      if (letter[row] & (1 << (4 - col))) {
        M5.Lcd.fillRect(x + (col * 3), y + (row * 3), 2, 2, color);
      }
    }
  }
}

void showSplash() {
  clearDisplay();
  
  // Draw scattered tetris pieces as decoration
  drawSplashPiece(30, 10, 0, COLOR_CYAN);      // I piece top left
  drawSplashPiece(180, 12, 1, COLOR_YELLOW);   // O piece top right
  drawSplashPiece(25, 40, 4, COLOR_RED);       // Z piece left
  drawSplashPiece(185, 42, 3, COLOR_GREEN);    // S piece right
  drawSplashPiece(35, 95, 5, COLOR_ORANGE);    // L piece bottom left
  drawSplashPiece(175, 98, 6, COLOR_BLUE);     // J piece bottom right
  
  // Draw TETRIS letters in center
  int startX = 60;
  int startY = 50;
  drawSplashLetter(startX, startY, LETTER_T, COLOR_CYAN);
  drawSplashLetter(startX + 20, startY, LETTER_E, COLOR_YELLOW);
  drawSplashLetter(startX + 40, startY, LETTER_T, COLOR_GREEN);
  drawSplashLetter(startX + 60, startY, LETTER_R, COLOR_RED);
  drawSplashLetter(startX + 80, startY, LETTER_I, COLOR_ORANGE);
  drawSplashLetter(startX + 100, startY, LETTER_S, 0xF81F);  // Magenta
  
  delay(1500);
}

void setup() {
  Serial.begin(115200);
  
  initInput();
  initDisplay();
  
  showSplash();
  
  initGameManager();
}

void loop() {
  updateInput();
  updateGameManager();
  delay(16); // ~60 FPS
}

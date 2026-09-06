// gamemanager.cpp - Game manager implementation
#include "gamemanager.h"
#include "display.h"
#include "input.h"

GameManager gameManager;

const char* gameNames[] = {
  "Dodge",
  "Tetris"
};

void initGameManager() {
  gameManager.init();
}

void updateGameManager() {
  gameManager.update();
}

void GameManager::init() {
  currentState = STATE_MENU;
  currentGame = 0;
  menuSelection = 0;
  showMenu();
}

void GameManager::update() {
  switch (currentState) {
    case STATE_MENU:
      handleMenuInput();
      break;
      
    case STATE_PLAYING:
      switch (currentGame) {
        case GAME_TETRIS:
          tetrisGame.update();
          tetrisGame.draw();
          if (tetrisGame.isGameOver()) {
            setState(STATE_GAME_OVER);
          }
          break;
          
        case GAME_DODGE:
          dodgeGame.update();
          dodgeGame.draw();
          if (dodgeGame.isGameOver()) {
            setState(STATE_GAME_OVER);
          }
          break;
      }
      
      // Press B to return to menu
      if (buttons.btnBPressed) {
        setState(STATE_MENU);
      }
      break;
      
    case STATE_GAME_OVER:
      showGameOver();
      if (buttons.btnAPressed) {
        setState(STATE_MENU);
      }
      break;
      
    case STATE_GAME_WON:
      // Similar to game over
      showGameOver();
      if (buttons.btnAPressed) {
        setState(STATE_MENU);
      }
      break;
  }
}

// Tetromino shapes for menu
static const int MENU_SHAPES[7][4][2] = {
  {{0,0}, {1,0}, {2,0}, {3,0}},  // I
  {{0,0}, {1,0}, {0,1}, {1,1}},  // O
  {{1,0}, {0,1}, {1,1}, {2,1}},  // T
  {{0,1}, {1,1}, {1,0}, {2,0}},  // S
  {{0,0}, {1,0}, {1,1}, {2,1}},  // Z
  {{0,0}, {1,0}, {2,0}, {2,1}},  // L
  {{0,0}, {1,0}, {2,0}, {0,1}}   // J
};

static const uint16_t MENU_COLORS[7] = {
  COLOR_CYAN, COLOR_YELLOW, 0xF81F, COLOR_GREEN, COLOR_RED, COLOR_ORANGE, COLOR_BLUE
};

// TETRIS letters (5x7 grid)
static const uint8_t MENU_T[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t MENU_E[7] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
static const uint8_t MENU_R[7] = {0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001}; // Backwards
static const uint8_t MENU_I[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111};
static const uint8_t MENU_S[7] = {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};

void drawMenuPiece(int x, int y, int type, uint16_t color) {
  for (int i = 0; i < 4; i++) {
    int px = MENU_SHAPES[type][i][0];
    int py = MENU_SHAPES[type][i][1];
    M5.Lcd.fillRect(x + px * 7, y + py * 7, 6, 6, color);  // 3/4 bigger (4->7, 3->6)
  }
}

void drawMenuLetter(int x, int y, const uint8_t* letter, uint16_t color) {
  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 5; col++) {
      if (letter[row] & (1 << (4 - col))) {
        M5.Lcd.fillRect(x + (col * 3), y + (row * 3), 2, 2, color);  // 3/4 bigger (2->3, 1->2)
      }
    }
  }
}

void GameManager::showMenu() {
  clearDisplay();
  
  // Draw scattered pieces on the right side - bigger and spread out more
  drawMenuPiece(110, 5, 0, COLOR_CYAN);     // I
  drawMenuPiece(200, 10, 1, COLOR_YELLOW);  // O
  drawMenuPiece(115, 50, 4, COLOR_RED);     // Z
  drawMenuPiece(195, 55, 3, COLOR_GREEN);   // S
  drawMenuPiece(120, 95, 5, COLOR_ORANGE);  // L
  drawMenuPiece(185, 100, 6, COLOR_BLUE);   // J
  
  // Draw TETRIS letters across the right side - bigger
  int letterX = 115;
  int letterY = 28;
  drawMenuLetter(letterX, letterY, MENU_T, COLOR_CYAN);
  drawMenuLetter(letterX + 18, letterY, MENU_E, COLOR_YELLOW);
  drawMenuLetter(letterX + 36, letterY, MENU_T, COLOR_GREEN);
  drawMenuLetter(letterX + 54, letterY, MENU_R, COLOR_RED);
  drawMenuLetter(letterX + 72, letterY, MENU_I, COLOR_ORANGE);
  drawMenuLetter(letterX + 90, letterY, MENU_S, 0xF81F);
  
  // Draw game selection list on left
  int y = 10;
  for (int i = 0; i < 2; i++) {
    if (i == menuSelection) {
      M5.Lcd.fillRect(5, y - 2, 75, 17, COLOR_BLUE);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(COLOR_WHITE);
      M5.Lcd.setCursor(8, y);
      M5.Lcd.println(gameNames[i]);
    } else {
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(COLOR_WHITE);
      M5.Lcd.setCursor(8, y);
      M5.Lcd.println(gameNames[i]);
    }
    y += 17;
  }
}

void GameManager::handleMenuInput() {
  static unsigned long lastInput = 0;
  if (millis() - lastInput < 200) return;
  
  if (buttons.upPressed) {
    menuSelection--;
    if (menuSelection < 0) menuSelection = 1; // Now 2 games
    showMenu();
    lastInput = millis();
  }
  else if (buttons.downPressed) {
    menuSelection++;
    if (menuSelection > 1) menuSelection = 0; // Now 2 games
    showMenu();
    lastInput = millis();
  }
  else if (buttons.btnAPressed) {
    startGame(menuSelection);
    lastInput = millis();
  }
}

void GameManager::startGame(int gameId) {
  currentGame = gameId;
  
  switch (gameId) {
    case GAME_TETRIS:
      tetrisGame.init();
      break;
    case GAME_DODGE:
      dodgeGame.init();
      break;
  }
  
  setState(STATE_PLAYING);
}

void GameManager::showGameOver() {
  clearDisplay();
  
  drawCenteredText("GAME OVER", 40, 3, COLOR_RED);
  
  char scoreText[30];
  int finalScore = 0;
  
  switch (currentGame) {
    case GAME_TETRIS:
      finalScore = tetrisGame.getScore();
      break;
    case GAME_DODGE:
      finalScore = dodgeGame.getScore();
      break;
  }
  
  sprintf(scoreText, "Score: %d", finalScore);
  drawCenteredText(scoreText, 75, 2, COLOR_YELLOW);
  drawCenteredText("Press A for Menu", 110, 1, COLOR_WHITE);
}

void GameManager::setState(GameState state) {
  currentState = state;
  
  if (state == STATE_MENU) {
    M5.Lcd.setRotation(1); // Reset to landscape for menu
    showMenu();
  }
  
  if (state == STATE_GAME_OVER || state == STATE_GAME_WON) {
    M5.Lcd.setRotation(1); // Reset to landscape for game over screen
  }
}

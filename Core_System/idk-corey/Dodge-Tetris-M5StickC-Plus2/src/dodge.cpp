// dodge.cpp - Tetris Dodge implementation
#include "dodge.h"
#include "input.h"

DodgeGame dodgeGame;

// Tetromino shapes (same as Tetris)
static const int SHAPES[7][4][2] = {
  {{0,0}, {1,0}, {2,0}, {3,0}},  // I
  {{0,0}, {1,0}, {0,1}, {1,1}},  // O
  {{1,0}, {0,1}, {1,1}, {2,1}},  // T
  {{0,1}, {1,1}, {1,0}, {2,0}},  // S
  {{0,0}, {1,0}, {1,1}, {2,1}},  // Z
  {{0,0}, {1,0}, {2,0}, {2,1}},  // L
  {{0,0}, {1,0}, {2,0}, {0,1}}   // J
};

static const uint16_t PIECE_COLORS[7] = {
  COLOR_CYAN, COLOR_YELLOW, 0xF81F,  // Magenta = 0xF81F
  COLOR_GREEN, COLOR_RED, COLOR_ORANGE, COLOR_BLUE
};

// Letter patterns for TETRIS (5x7 grid, 1=filled, 0=empty)
static const uint8_t LETTER_T[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t LETTER_E[7] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
static const uint8_t LETTER_T2[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t LETTER_R[7] = {0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001}; // Backwards R!
static const uint8_t LETTER_I[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111};
static const uint8_t LETTER_S[7] = {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};

static const uint8_t* LETTERS[6] = {LETTER_T, LETTER_E, LETTER_T2, LETTER_R, LETTER_I, LETTER_S};

static const uint16_t LETTER_COLORS[6] = {
  0x07FF,      // T - Bright Cyan
  0xFFE0,      // E - Bright Yellow  
  0x07E0,      // T - Bright Green
  0xF800,      // R - Bright Red
  0xFD20,      // I - Bright Orange
  0xF81F       // S - Bright Magenta
};

// Sequential letter tracking
static int nextLetterIndex = 0;

// Wave 3 patterns - various fun shapes (like letters, but more creative!)
// Using 5x5 grids for variety
static const uint8_t SHAPE_PLUS[5] = {0b00100, 0b00100, 0b11111, 0b00100, 0b00100};
static const uint8_t SHAPE_DIAMOND[5] = {0b00100, 0b01010, 0b10001, 0b01010, 0b00100};
static const uint8_t SHAPE_ARROW[5] = {0b00100, 0b01110, 0b11111, 0b00100, 0b00100};
static const uint8_t SHAPE_HEART[5] = {0b01010, 0b11111, 0b11111, 0b01110, 0b00100};
static const uint8_t SHAPE_ZIG[5] = {0b11000, 0b01100, 0b00110, 0b00011, 0b00001};
static const uint8_t SHAPE_CROSS[5] = {0b10001, 0b01010, 0b00100, 0b01010, 0b10001};
static const uint8_t SHAPE_STAIRS[5] = {0b00001, 0b00011, 0b00111, 0b01111, 0b11111};
static const uint8_t SHAPE_DOT[5] = {0b00000, 0b00000, 0b01110, 0b00000, 0b00000};

static const uint8_t* WAVE3_SHAPES[8] = {
  SHAPE_PLUS, SHAPE_DIAMOND, SHAPE_ARROW, SHAPE_HEART, 
  SHAPE_ZIG, SHAPE_CROSS, SHAPE_STAIRS, SHAPE_DOT
};

static const uint16_t WAVE3_COLORS[8] = {
  0xFFE0,  // Yellow
  0x07FF,  // Cyan
  0xF81F,  // Magenta
  0xF800,  // Red
  0x07E0,  // Green
  0xFD20,  // Orange
  0x07FF,  // Cyan
  0xFFFF   // White
};

void DodgeGame::init() {
  // Portrait mode
  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(COLOR_BLACK);
  
  // Player starts at bottom center
  playerX = 50;
  playerY = 200;
  playerSize = 8;
  playerRotation = 0;
  
  // Initialize pieces
  for (int i = 0; i < MAX_PIECES; i++) {
    pieces[i].active = false;
  }
  
  score = 0;
  level = 1;
  gameOver = false;
  gameStartTime = millis();
  currentWave = 1;
  lastSpawnTime = millis();
  spawnInterval = 1500;  // Spawn every 1.5 seconds initially
}

void DodgeGame::spawnPiece() {
  // Find inactive piece slot
  for (int i = 0; i < MAX_PIECES; i++) {
    if (!pieces[i].active) {
      pieces[i].y = -20;
      pieces[i].speed = 1.0 + (level * 0.2);
      pieces[i].active = true;
      
      // Wave 1: Tetris pieces
      if (currentWave == 1) {
        pieces[i].x = random(10, 110);
        pieces[i].type = random(7);
        pieces[i].rotation = random(4);
      }
      // Wave 2: TETRIS Letters - in sequence!
      else if (currentWave == 2) {
        pieces[i].x = random(5, 105);
        pieces[i].type = 7;  // Letter type
        pieces[i].rotation = nextLetterIndex;  // T, E, T, R, I, S in order
        nextLetterIndex = (nextLetterIndex + 1) % 6;  // Cycle through letters
        pieces[i].speed = 1.0 + (level * 0.15);  // Slightly slower for readability
      }
      // Wave 3: Fun block patterns (shapes)
      else if (currentWave == 3) {
        pieces[i].type = 8;  // Shape type
        pieces[i].x = random(5, 105);
        pieces[i].rotation = random(8);  // Which shape (0-7)
        pieces[i].speed = 1.0 + (level * 0.2);
      }
      // Wave 4+: Mix Tetris pieces, Letters, and Shapes
      else {
        int waveType = random(3);
        if (waveType == 0) {
          // Tetris pieces
          pieces[i].x = random(10, 110);
          pieces[i].type = random(7);
          pieces[i].rotation = random(4);
        } else if (waveType == 1) {
          // Letters - sequential
          pieces[i].x = random(5, 105);
          pieces[i].type = 7;
          pieces[i].rotation = nextLetterIndex;
          nextLetterIndex = (nextLetterIndex + 1) % 6;
        } else {
          // Shapes
          pieces[i].type = 8;
          pieces[i].x = random(5, 105);
          pieces[i].rotation = random(8);
        }
      }
      break;
    }
  }
}

void DodgeGame::updateWave() {
  unsigned long elapsed = (millis() - gameStartTime) / 1000;  // seconds
  int newWave = (elapsed / 30) + 1;  // Change wave every 30 seconds
  
  if (newWave != currentWave) {
    currentWave = newWave;
    if (currentWave <= 4) {
      spawnInterval = max(500, (int)(spawnInterval * 0.85));  // Speed up each wave
    }
  }
}

void DodgeGame::updatePieces() {
  updateWave();
  
  for (int i = 0; i < MAX_PIECES; i++) {
    if (pieces[i].active) {
      // Clear old position
      if (pieces[i].type < 7) {
        clearPiece(pieces[i].x, pieces[i].y, pieces[i].type, pieces[i].rotation);
      } else if (pieces[i].type == 7) {
        clearLetter(pieces[i].x, pieces[i].y, pieces[i].rotation);
      } else if (pieces[i].type == 8) {
        clearShape(pieces[i].x, pieces[i].y, pieces[i].rotation);
      } else if (pieces[i].type >= 9) {
        clearObstacle(pieces[i].x, pieces[i].y, pieces[i].width, pieces[i].height);
      }
      
      // Move down
      pieces[i].y += pieces[i].speed;
      
      // Remove if off screen
      if (pieces[i].y > 240) {
        pieces[i].active = false;
        score += 10;  // Points for surviving
        
        // Level up every 100 points
        if (score % 100 == 0 && score > 0) {
          level++;
          spawnInterval = max(500, (int)(spawnInterval * 0.8));  // Spawn faster
        }
      }
      
      // Check collision with player
      if (checkCollision(playerX, playerY, i)) {
        gameOver = true;
      }
    }
  }
  
  // Spawn new pieces
  if (millis() - lastSpawnTime > spawnInterval) {
    spawnPiece();
    lastSpawnTime = millis();
  }
}

bool DodgeGame::checkCollision(int px, int py, int pieceIdx) {
  if (!pieces[pieceIdx].active) return false;
  
  FallingPiece* p = &pieces[pieceIdx];
  
  // For letters (type 7), check pixel-by-pixel collision
  if (p->type == 7) {
    int letterIdx = p->rotation % 6;
    const uint8_t* letter = LETTERS[letterIdx];
    
    // Check each pixel of the letter (5x7 grid, scaled 4x)
    for (int row = 0; row < 7; row++) {
      for (int col = 0; col < 5; col++) {
        if (letter[row] & (1 << (4 - col))) {
          int letterPixelX = p->x + (col * 4);
          int letterPixelY = p->y + (row * 4);
          
          // Check against player T-piece blocks
          for (int j = 0; j < 4; j++) {
            int ppx = SHAPES[2][j][0] - 1;
            int ppy = SHAPES[2][j][1] - 1;
            int prx, pry;
            switch(playerRotation % 4) {
              case 0: prx = ppx; pry = ppy; break;
              case 1: prx = -ppy; pry = ppx; break;
              case 2: prx = -ppx; pry = -ppy; break;
              case 3: prx = ppy; pry = -ppx; break;
            }
            int pbx = px + (prx + 1) * 8;
            int pby = py + (pry + 1) * 8;
            
            // Check if letter pixel overlaps with player block
            if (letterPixelX >= pbx && letterPixelX < pbx + 7 &&
                letterPixelY >= pby && letterPixelY < pby + 7) {
              return true;
            }
          }
        }
      }
    }
    return false;
  }
  
  // For shapes (type 8), check pixel-by-pixel collision (5x5 grid)
  if (p->type == 8) {
    int shapeIdx = p->rotation % 8;
    const uint8_t* shape = WAVE3_SHAPES[shapeIdx];
    
    // Check each pixel of the shape (5x5 grid, scaled 4x)
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 5; col++) {
        if (shape[row] & (1 << (4 - col))) {
          int shapePixelX = p->x + (col * 4);
          int shapePixelY = p->y + (row * 4);
          
          // Check against player T-piece blocks
          for (int j = 0; j < 4; j++) {
            int ppx = SHAPES[2][j][0] - 1;
            int ppy = SHAPES[2][j][1] - 1;
            int prx, pry;
            switch(playerRotation % 4) {
              case 0: prx = ppx; pry = ppy; break;
              case 1: prx = -ppy; pry = ppx; break;
              case 2: prx = -ppx; pry = -ppy; break;
              case 3: prx = ppy; pry = -ppx; break;
            }
            int pbx = px + (prx + 1) * 8;
            int pby = py + (pry + 1) * 8;
            
            // Check if shape pixel overlaps with player block
            if (shapePixelX >= pbx && shapePixelX < pbx + 7 &&
                shapePixelY >= pby && shapePixelY < pby + 7) {
              return true;
            }
          }
        }
      }
    }
    return false;
  }
  
  // For rectangles (type >= 9), use collision
  if (p->type >= 9) {
    int playerLeft = px;
    int playerRight = px + 24;
    int playerTop = py;
    int playerBottom = py + 16;
    
    int obstacleLeft = p->x;
    int obstacleRight = p->x + p->width;
    int obstacleTop = p->y;
    int obstacleBottom = p->y + p->height;
    
    return !(playerRight < obstacleLeft || 
             playerLeft > obstacleRight ||
             playerBottom < obstacleTop ||
             playerTop > obstacleBottom);
  }
  
  // Precise block-to-block collision for tetromino pieces
  // Check each block of the falling piece against each block of the player
  for (int i = 0; i < 4; i++) {
    // Get falling piece block position with rotation
    int fpx = SHAPES[p->type][i][0] - 1;
    int fpy = SHAPES[p->type][i][1] - 1;
    int frx, fry;
    switch(p->rotation % 4) {
      case 0: frx = fpx; fry = fpy; break;
      case 1: frx = -fpy; fry = fpx; break;
      case 2: frx = -fpx; fry = -fpy; break;
      case 3: frx = fpy; fry = -fpx; break;
    }
    int fbx = p->x + (frx + 1) * 8;
    int fby = p->y + (fry + 1) * 8;
    
    // Check against each player block
    for (int j = 0; j < 4; j++) {
      int ppx = SHAPES[2][j][0] - 1;  // Player is type 2 (T piece)
      int ppy = SHAPES[2][j][1] - 1;
      int prx, pry;
      switch(playerRotation % 4) {
        case 0: prx = ppx; pry = ppy; break;
        case 1: prx = -ppy; pry = ppx; break;
        case 2: prx = -ppx; pry = -ppy; break;
        case 3: prx = ppy; pry = -ppx; break;
      }
      int pbx = px + (prx + 1) * 8;
      int pby = py + (pry + 1) * 8;
      
      // Check if blocks overlap (within 7 pixels for block size)
      if (abs(fbx - pbx) < 7 && abs(fby - pby) < 7) {
        return true;
      }
    }
  }
  
  return false;
}

void DodgeGame::clearObstacle(int x, int y, int width, int height) {
  M5.Lcd.fillRect(x, y, width, height, COLOR_BLACK);
}

void DodgeGame::clearLetter(int x, int y, int letterIdx) {
  // Clear 5x7 letter grid (scaled 4x = 20x28 pixels)
  M5.Lcd.fillRect(x, y, 20, 28, COLOR_BLACK);
}

void DodgeGame::drawLetter(int x, int y, int letterIdx, uint16_t color) {
  letterIdx = letterIdx % 6;
  const uint8_t* letter = LETTERS[letterIdx];
  
  // Draw 5x7 letter grid, each pixel is 4x4
  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 5; col++) {
      if (letter[row] & (1 << (4 - col))) {
        M5.Lcd.fillRect(x + (col * 4), y + (row * 4), 3, 3, color);
      }
    }
  }
}

void DodgeGame::clearShape(int x, int y, int shapeIdx) {
  // Clear 5x5 shape grid (scaled 4x = 20x20 pixels)
  M5.Lcd.fillRect(x, y, 20, 20, COLOR_BLACK);
}

void DodgeGame::drawShape(int x, int y, int shapeIdx, uint16_t color) {
  shapeIdx = shapeIdx % 8;
  const uint8_t* shape = WAVE3_SHAPES[shapeIdx];
  
  // Draw 5x5 shape grid, each pixel is 4x4
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 5; col++) {
      if (shape[row] & (1 << (4 - col))) {
        M5.Lcd.fillRect(x + (col * 4), y + (row * 4), 3, 3, color);
      }
    }
  }
}

void DodgeGame::clearPiece(int x, int y, int type, int rotation) {
  // Draw black squares over piece with rotation around center block (1,1)
  for (int i = 0; i < 4; i++) {
    int px = SHAPES[type][i][0] - 1;  // Shift to rotate around (1,1)
    int py = SHAPES[type][i][1] - 1;
    
    // Apply rotation
    int rx, ry;
    switch(rotation % 4) {
      case 0: rx = px; ry = py; break;
      case 1: rx = -py; ry = px; break;
      case 2: rx = -px; ry = -py; break;
      case 3: rx = py; ry = -px; break;
    }
    
    int bx = x + (rx + 1) * 8;  // Shift back
    int by = y + (ry + 1) * 8;
    M5.Lcd.fillRect(bx, by, 8, 8, COLOR_BLACK);
  }
}

void DodgeGame::drawObstacle(int x, int y, int width, int height, uint16_t color) {
  M5.Lcd.fillRect(x, y, width, height, color);
  M5.Lcd.drawRect(x, y, width, height, COLOR_WHITE);
}

void DodgeGame::drawPiece(int x, int y, int type, int rotation, uint16_t color) {
  // Draw the tetromino with rotation around center block (1,1)
  for (int i = 0; i < 4; i++) {
    int px = SHAPES[type][i][0] - 1;  // Shift to rotate around (1,1)
    int py = SHAPES[type][i][1] - 1;
    
    // Apply rotation
    int rx, ry;
    switch(rotation % 4) {
      case 0: rx = px; ry = py; break;
      case 1: rx = -py; ry = px; break;  // 90 degrees
      case 2: rx = -px; ry = -py; break; // 180 degrees
      case 3: rx = py; ry = -px; break;  // 270 degrees
    }
    
    int bx = x + (rx + 1) * 8;  // Shift back
    int by = y + (ry + 1) * 8;
    M5.Lcd.fillRect(bx, by, 7, 7, color);
    M5.Lcd.drawRect(bx, by, 7, 7, COLOR_WHITE);
  }
}

void DodgeGame::drawPlayer() {
  // Draw player as T piece (type 2) with rotation in bright color
  drawPiece(playerX, playerY, 2, playerRotation, COLOR_YELLOW);
}

void DodgeGame::update() {
  if (gameOver) return;
  
  handleInput();
  updatePieces();
}

void DodgeGame::draw() {
  // Draw active pieces
  for (int i = 0; i < MAX_PIECES; i++) {
    if (pieces[i].active) {
      if (pieces[i].type < 7) {
        drawPiece(pieces[i].x, pieces[i].y, pieces[i].type, 
                  pieces[i].rotation, PIECE_COLORS[pieces[i].type]);
      } else if (pieces[i].type == 7) {
        int letterIdx = pieces[i].rotation % 6;
        drawLetter(pieces[i].x, pieces[i].y, letterIdx, LETTER_COLORS[letterIdx]);
      } else if (pieces[i].type == 8) {
        int shapeIdx = pieces[i].rotation % 8;
        drawShape(pieces[i].x, pieces[i].y, shapeIdx, WAVE3_COLORS[shapeIdx]);
      } else if (pieces[i].type >= 9) {
        drawObstacle(pieces[i].x, pieces[i].y, pieces[i].width, pieces[i].height, COLOR_PURPLE);
      }
    }
  }
  
  // Draw player
  drawPlayer();
  
  // Draw UI
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_WHITE, COLOR_BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.print("S:");
  M5.Lcd.print(score);
  M5.Lcd.setCursor(60, 5);
  M5.Lcd.print("Lv:");
  M5.Lcd.print(level);
  M5.Lcd.setCursor(95, 5);
  M5.Lcd.print("W:");
  M5.Lcd.print(currentWave);
}

void DodgeGame::handleInput() {
  // Portrait controls (same as Tetris/Breakout)
  if (buttons.down && playerX > 0) {
    // Clear old position
    clearPiece(playerX, playerY, 2, playerRotation);
    playerX -= 3;
  }
  
  if (buttons.up && playerX < 100) {
    clearPiece(playerX, playerY, 2, playerRotation);
    playerX += 3;
  }
  
  if (buttons.left && playerY > 20) {
    clearPiece(playerX, playerY, 2, playerRotation);
    playerY -= 3;
  }
  
  if (buttons.right && playerY < 210) {
    clearPiece(playerX, playerY, 2, playerRotation);
    playerY += 3;
  }
  
  // Rotate with joystick button press
  static bool rotateHeld = false;
  if (buttons.joyBtnPressed && !rotateHeld) {
    clearPiece(playerX, playerY, 2, playerRotation);
    playerRotation = (playerRotation + 1) % 4;
    rotateHeld = true;
  } else if (!buttons.joyBtn) {
    rotateHeld = false;
  }
}

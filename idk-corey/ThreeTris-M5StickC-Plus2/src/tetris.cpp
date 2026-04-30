// tetris.cpp - ThreeTris with 3-block pieces (Trominoes)
#include "tetris.h"
#include "display.h"
#include "input.h"

TetrisGame tetrisGame;

void TetrisGame::init() {
  // Set landscape mode for better gameplay
  M5.Lcd.setRotation(0);
  
  // Initialize field
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      field[y][x] = 0;
    }
  }
  
  // 8 pieces: I, L, J, Single, Double, Diagonal Double, Diagonal Triple, T-minus
  int tempPieces[8][4][2][3] = {
    // I-Triomino
    {
      // R0 (horizontal)
      { {  0,  0,  0 }, { -1,  0,  1 } },
      // R90 (vertical)
      { { -1,  0,  1 }, {  0,  0,  0 } },
      // R180 (horizontal)
      { {  0,  0,  0 }, { -1,  0,  1 } },
      // R270 (vertical)
      { { -1,  0,  1 }, {  0,  0,  0 } }
    },

    // L-Triomino - CLOCKWISE rotation
    // R0: up with corner right, R90: right with corner down, R180: down with corner left, R270: left with corner up
    {
      // R0: (-1,0), (0,0), (0,1) - vertical up, corner right
      { { -1,  0,  0 }, {  0,  0,  1 } },

      // R90: (0,1), (0,0), (1,0) - horizontal right, corner down
      { {  0,  0,  1 }, {  1,  0,  0 } },

      // R180: (1,0), (0,0), (0,-1) - vertical down, corner left
      { {  1,  0,  0 }, {  0,  0, -1 } },

      // R270: (0,-1), (0,0), (-1,0) - horizontal left, corner up
      { {  0,  0, -1 }, { -1,  0,  0 } }
    },

    // J-Triomino (mirrored L) - CLOCKWISE rotation
    // R0: up with corner left, R90: right with corner up, R180: down with corner right, R270: left with corner down
    {
      // R0: (-1,0), (0,0), (0,-1) - vertical up, corner left
      { { -1,  0,  0 }, {  0,  0, -1 } },

      // R90: (0,1), (0,0), (-1,0) - horizontal right, corner up
      { {  0,  0, -1 }, {  1,  0,  0 } },

      // R180: (1,0), (0,0), (0,1) - vertical down, corner right
      { {  1,  0,  0 }, {  0,  0,  1 } },

      // R270: (0,-1), (0,0), (1,0) - horizontal left, corner down
      { {  0,  0,  1 }, { -1,  0,  0 } }
    },

    // Single Block (just 1 block at center, duplicated to fill array)
    // All rotations are the same
    {
      // R0
      { {  0,  0,  0 }, {  0,  0,  0 } },
      // R90
      { {  0,  0,  0 }, {  0,  0,  0 } },
      // R180
      { {  0,  0,  0 }, {  0,  0,  0 } },
      // R270
      { {  0,  0,  0 }, {  0,  0,  0 } }
    },

    // Double Block (Domino - 2 blocks side by side, horizontal/vertical)
    {
      // R0 (horizontal)
      { {  0,  0,  0 }, { -1,  0,  0 } },
      // R90 (vertical)
      { { -1,  0,  0 }, {  0,  0,  0 } },
      // R180 (horizontal)
      { {  0,  0,  0 }, { -1,  0,  0 } },
      // R270 (vertical)
      { { -1,  0,  0 }, {  0,  0,  0 } }
    },

    // Diagonal Double Block (2 blocks diagonal - / and \)
    {
      // R0 (/ diagonal: bottom-left to top-right)
      { { -1,  0,  0 }, {  1,  0,  0 } },
      // R90 (\ diagonal: top-left to bottom-right)
      { {  0,  1,  0 }, {  0,  1,  0 } },
      // R180 (same as R0)
      { { -1,  0,  0 }, {  1,  0,  0 } },
      // R270 (same as R90)
      { {  0,  1,  0 }, {  0,  1,  0 } }
    },

    // Diagonal Triple (3 blocks diagonal - / and \)
    {
      // R0 (/ diagonal: bottom-left to top-right)
      { { -1,  0,  1 }, { -1,  0,  1 } },
      // R90 (\ diagonal: top-left to bottom-right)
      { { -1,  0,  1 }, {  1,  0, -1 } },
      // R180 (same as R0)
      { { -1,  0,  1 }, { -1,  0,  1 } },
      // R270 (same as R90)
      { { -1,  0,  1 }, {  1,  0, -1 } }
    },

    // T-minus-one piece (FIXED: proper rotation around pivot)
    {
      // R0: .#. / ### - blocks at (-1,0), (0,-1), (0,1)
      {{ -1,  0,  0 }, {  0, -1,  1 }},
      
      // R90: rotated clockwise - blocks at (0,1), (-1,0), (1,0)
      {{  0, -1,  1 }, {  1,  0,  0 }},
      
      // R180: rotated 180° - blocks at (1,0), (0,1), (0,-1)
      {{  1,  0,  0 }, {  0,  1, -1 }},
      
      // R270: rotated 270° - blocks at (0,-1), (1,0), (-1,0)
      {{  0,  1, -1 }, { -1,  0,  0 }}
    }
  };
  memcpy(pieces, tempPieces, sizeof(pieces));
  
  // Colors for 8 pieces
  pieceColors[0] = COLOR_CYAN;    // I
  pieceColors[1] = COLOR_BLUE;    // L
  pieceColors[2] = COLOR_ORANGE;  // J
  pieceColors[3] = COLOR_YELLOW;  // Single Block
  pieceColors[4] = COLOR_GREEN;   // Double Block
  pieceColors[5] = COLOR_RED;     // Diagonal Double
  pieceColors[6] = COLOR_PURPLE;  // Diagonal Triple
  pieceColors[7] = COLOR_WHITE;   // T-minus-one
  
  score = 0;
  level = 1;
  linesCleared = 0;
  dropSpeed = 500;
  gameOver = false;
  needsRedraw = true;  // Force border redraw on init
  lastDropTime = 0;
  lockDelayActive = false;
  
  // Modern features
  heldPiece = -1;
  nextPiece = random(0, 8);  // 3 pieces
  canHold = true;
  
  newPiece(false);
}

void TetrisGame::update() {
  handleInput();
  
  if (millis() - lastDropTime > dropSpeed) {
    posY++;
    if (test(posY, posX, currentPiece, currentRot)) {
      posY--;
      // Start lock delay when piece hits bottom
      if (!lockDelayActive) {
        lockDelayActive = true;
        lockDelayStart = millis();
      }
      
      // Check if lock delay has expired (progressive based on level)
      int lockDelayTime = 300; // Default for levels 1-5
      if (level >= 16) lockDelayTime = 600;
      else if (level >= 11) lockDelayTime = 500;
      else if (level >= 6) lockDelayTime = 400;
      
      if (millis() - lockDelayStart >= lockDelayTime) {
        placePiece();
        clearLines();
        newPiece(true);
        if (test(posY, posX, currentPiece, currentRot)) {
          gameOver = true;
        }
      }
    } else {
      lockDelayActive = false;
    }
    lastDropTime = millis();
  }
}

void TetrisGame::draw() {
  static int lastScore = -1;
  
  // Redraw border if needed (on init or restart)
  if (needsRedraw) {
    clearDisplay();
    M5.Lcd.drawRect(OFFSET_X-1, OFFSET_Y-1, FIELD_WIDTH*BLOCK_SIZE+1, FIELD_HEIGHT*BLOCK_SIZE+1, COLOR_WHITE);
    needsRedraw = false;
  }
  
  // Draw field blocks
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      int px = OFFSET_X + x * BLOCK_SIZE;
      int py = OFFSET_Y + y * BLOCK_SIZE;
      if (field[y][x] > 0) {
        M5.Lcd.fillRect(px, py, BLOCK_SIZE-1, BLOCK_SIZE-1, pieceColors[field[y][x]-1]);
        M5.Lcd.drawRect(px, py, BLOCK_SIZE-1, BLOCK_SIZE-1, COLOR_WHITE);
      } else {
        M5.Lcd.fillRect(px, py, BLOCK_SIZE-1, BLOCK_SIZE-1, COLOR_BLACK);
      }
    }
  }
  
  // Draw ghost piece
  drawGhostPiece();
  
  // Draw current piece (3 blocks!)
  for (int i = 0; i < 3; i++) {
    int x = posX + pieces[currentPiece][currentRot][1][i];
    int y = posY + pieces[currentPiece][currentRot][0][i];
    if (y >= 0 && y < FIELD_HEIGHT && x >= 0 && x < FIELD_WIDTH) {
      int px = OFFSET_X + x * BLOCK_SIZE;
      int py = OFFSET_Y + y * BLOCK_SIZE;
      M5.Lcd.fillRect(px, py, BLOCK_SIZE-1, BLOCK_SIZE-1, pieceColors[currentPiece]);
      M5.Lcd.drawRect(px, py, BLOCK_SIZE-1, BLOCK_SIZE-1, COLOR_WHITE);
    }
  }
  
  // Draw hold and next pieces
  drawHoldPiece();
  drawNextPiece();
  
  // Only update score when it changes
  if (score != lastScore) {
    M5.Lcd.fillRect(110, 5, 50, 10, COLOR_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(COLOR_WHITE);
    M5.Lcd.setCursor(110, 5);
    M5.Lcd.print("S:");
    M5.Lcd.print(score);
    lastScore = score;
  }
  
  // Draw level below NEXT box
  static int lastLevel = -1;
  if (level != lastLevel) {
    M5.Lcd.fillRect(108, 120, 50, 10, COLOR_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(COLOR_WHITE);
    M5.Lcd.setCursor(108, 120);
    M5.Lcd.print("LV:");
    M5.Lcd.setTextColor(COLOR_GREEN);
    M5.Lcd.print(level);
    lastLevel = level;
  }
}

void TetrisGame::handleInput() {
  static unsigned long lastMove = 0;
  static unsigned long lastLeftRight = 0;
  static bool buttonHeld = false;
  
  // Left movement
  if (buttons.down && millis() - lastLeftRight > 80) {
    posX--;
    if (test(posY, posX, currentPiece, currentRot)) posX++;
    else if (lockDelayActive) lockDelayStart = millis();
    lastLeftRight = millis();
  }
  // Right movement
  else if (buttons.up && millis() - lastLeftRight > 80) {
    posX++;
    if (test(posY, posX, currentPiece, currentRot)) posX--;
    else if (lockDelayActive) lockDelayStart = millis();
    lastLeftRight = millis();
  }
  
  if (millis() - lastMove < 100) return;
  
  // Hard drop
  if (buttons.leftPressed && !buttonHeld) {
    while(!test(posY + 1, posX, currentPiece, currentRot)) {
      posY++;
      score += 2;
    }
    lockDelayActive = false;
    lastDropTime = 0;
    lastMove = millis();
    buttonHeld = true;
  }
  // Soft drop
  else if (buttons.right) {
    posY++;
    if (test(posY, posX, currentPiece, currentRot)) {
      posY--;
    } else {
      score++;
      lockDelayActive = false;
    }
    lastMove = millis();
    buttonHeld = true;
  }
  // Rotate - CLOCKWISE (to the right) using +1
  else if (buttons.joyBtnPressed && !buttonHeld) {
    int oldRot = currentRot;
    int newRot = (currentRot + 1) % 4;  // Clockwise rotation: R0→R1→R2→R3
    
    // test() returns true if collision detected
    if (!test(posY, posX, currentPiece, newRot)) {
      currentRot = newRot;
      if (lockDelayActive) lockDelayStart = millis();
    }
    // else: collision, keep oldRot
    
    lastMove = millis();
    buttonHeld = true;
  }
  // Hold piece
  else if (buttons.btnAPressed && !buttonHeld) {
    holdPiece();
    lastMove = millis();
    buttonHeld = true;
  }
  else {
    buttonHeld = false;
  }
}

bool TetrisGame::test(int y, int x, int piece, int rot) {
  for (int i = 0; i < 3; i++) {  // Changed from 4 to 3!
    int px = x + pieces[piece][rot][1][i];
    int py = y + pieces[piece][rot][0][i];
    
    if (px < 0 || px >= FIELD_WIDTH || py >= FIELD_HEIGHT) return true;
    if (py >= 0 && field[py][px] > 0) return true;
  }
  return false;
}

void TetrisGame::placePiece() {
  for (int i = 0; i < 3; i++) {  // Changed from 4 to 3!
    int x = posX + pieces[currentPiece][currentRot][1][i];
    int y = posY + pieces[currentPiece][currentRot][0][i];
    if (y >= 0 && y < FIELD_HEIGHT && x >= 0 && x < FIELD_WIDTH) {
      field[y][x] = currentPiece + 1;
    }
  }
}

void TetrisGame::clearLines() {
  int linesThisClear = 0;
  
  for (int y = FIELD_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (field[y][x] == 0) {
        fullLine = false;
        break;
      }
    }
    
    if (fullLine) {
      linesThisClear++;
      score += 100;
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
          field[yy][x] = field[yy-1][x];
        }
      }
      for (int x = 0; x < FIELD_WIDTH; x++) {
        field[0][x] = 0;
      }
      y++;
    }
  }
  
  // Update lines cleared and check for level up
  if (linesThisClear > 0) {
    linesCleared += linesThisClear;
    
    // Level up every 10 lines, cap at level 20 for speed (but endless gameplay)
    int newLevel = 1 + (linesCleared / 10);
    if (newLevel > level) {
      level = newLevel;
      // Speed progression: cap speed increase at level 20, but level keeps going
      int speedLevel = min(20, level);
      dropSpeed = max(50, 500 - (speedLevel - 1) * 23);  // Reaches 50ms at level 20
    }
  }
}

void TetrisGame::newPiece(bool setPiece) {
  if (setPiece) {
    canHold = true;
  }
  
  // Use next piece, generate new next
  if (nextPiece >= 0) {
    currentPiece = nextPiece;
    nextPiece = random(0, 8);  // 3 pieces
  } else {
    currentPiece = random(0, 8);
    nextPiece = random(0, 8);
  }
  
  currentRot = 0;
  posX = FIELD_WIDTH / 2 - 1;
  posY = 0;
  
  lockDelayActive = false;
}

void TetrisGame::holdPiece() {
  if (!canHold) return;
  
  if (heldPiece == -1) {
    heldPiece = currentPiece;
    newPiece(false);
  } else {
    int temp = currentPiece;
    currentPiece = heldPiece;
    heldPiece = temp;
    currentRot = 0;
    posX = FIELD_WIDTH / 2 - 1;
    posY = 0;
  }
  
  canHold = false;
  lockDelayActive = false;
}

int TetrisGame::calculateDropDistance() {
  int dropDist = 0;
  for (int testY = posY + 1; testY < FIELD_HEIGHT; testY++) {
    if (!test(testY, posX, currentPiece, currentRot)) {
      dropDist = testY - posY;
    } else {
      break;
    }
  }
  return dropDist;
}

void TetrisGame::drawGhostPiece() {
  int dropDist = calculateDropDistance();
  if (dropDist > 0) {
    int ghostY = posY + dropDist;
    for (int i = 0; i < 3; i++) {  // Changed from 4 to 3!
      int x = posX + pieces[currentPiece][currentRot][1][i];
      int y = ghostY + pieces[currentPiece][currentRot][0][i];
      if (y >= 0 && y < FIELD_HEIGHT && x >= 0 && x < FIELD_WIDTH) {
        int px = OFFSET_X + x * BLOCK_SIZE;
        int py = OFFSET_Y + y * BLOCK_SIZE;
        M5.Lcd.drawRect(px, py, BLOCK_SIZE-1, BLOCK_SIZE-1, 0x4208); // Gray
      }
    }
  }
}

void TetrisGame::drawMiniPiece(int pieceType, int x, int y, int scale) {
  if (pieceType < 0 || pieceType > 7) return;  // 3 pieces
  for (int i = 0; i < 3; i++) {
    int px = x + (pieces[pieceType][0][1][i] * scale);
    int py = y + (pieces[pieceType][0][0][i] * scale);
    M5.Lcd.fillRect(px, py, scale-1, scale-1, pieceColors[pieceType]);
  }
}

void TetrisGame::drawHoldPiece() {
  static int lastLines = -1;
  
  M5.Lcd.fillRect(110, 15, 30, 30, COLOR_BLACK);
  M5.Lcd.drawRect(109, 14, 32, 32, COLOR_WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_WHITE);
  M5.Lcd.setCursor(108, 48);
  M5.Lcd.print("HOLD");
  
  if (heldPiece >= 0) {
    drawMiniPiece(heldPiece, 117, 22, 5);
  }
  
  // Draw line counter below hold piece
  if (linesCleared != lastLines) {
    M5.Lcd.fillRect(108, 60, 50, 10, COLOR_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(COLOR_WHITE);
    M5.Lcd.setCursor(108, 60);
    M5.Lcd.print("L:");
    M5.Lcd.setTextColor(COLOR_CYAN);
    M5.Lcd.print(linesCleared);
    lastLines = linesCleared;
  }
}

void TetrisGame::drawNextPiece() {
  // Next piece preview box - directly under line counter, same X as HOLD
  M5.Lcd.fillRect(110, 75, 30, 30, COLOR_BLACK);
  M5.Lcd.drawRect(109, 74, 32, 32, COLOR_WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_WHITE);
  M5.Lcd.setCursor(108, 108);
  M5.Lcd.print("NEXT");
  
  if (nextPiece >= 0) {
    drawMiniPiece(nextPiece, 117, 82, 5);
  }
}

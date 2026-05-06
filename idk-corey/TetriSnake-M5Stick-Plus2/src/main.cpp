#include <M5StickCPlus2.h>

// Display constants
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define PLAY_AREA_TOP 22
#define PLAY_AREA_BOTTOM 234  // More room at bottom
#define BLOCK_SIZE 6

// 5-way tactile switch pins
#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

// Grid constants
#define GRID_COLS 22  // 135 / 6 = 22.5
#define GRID_ROWS 35  // 212 / 6 = 35.3 (more rows now)

// Colors
#define COLOR_SNAKE 0x07E0      // Green
#define COLOR_FOOD 0xFFFF       // White (stands out!)
#define COLOR_BG 0x0000         // Black
#define COLOR_I 0x07FF          // Cyan
#define COLOR_O 0xFFE0          // Yellow
#define COLOR_T 0xF81F          // Magenta
#define COLOR_L 0xFDA0          // Orange
#define COLOR_J 0x001F          // Blue
#define COLOR_S 0x07E0          // Green
#define COLOR_Z 0xF800          // Red

// JoyC removed

// Tetrimino shapes [type][block][x,y offset from anchor]
const int8_t TETRIMINO_SHAPES[7][4][2] = {
  {{0,0},{1,0},{2,0},{3,0}},   // I-piece (horizontal line, 4 blocks)
  {{0,0},{1,0},{0,1},{1,1}},   // O-piece (2x2 square, 4 blocks)
  {{1,0},{0,1},{1,1},{2,1}},   // T-piece (T shape, 4 blocks)
  {{0,0},{0,1},{0,2},{1,2}},   // L-piece (L shape, 4 blocks)
  {{1,0},{1,1},{1,2},{0,2}},   // J-piece (reverse L, 4 blocks)
  {{1,0},{2,0},{0,1},{1,1}},   // S-piece (S shape, 4 blocks)
  {{0,0},{1,0},{1,1},{2,1}}    // Z-piece (Z shape, 4 blocks)
};

const uint16_t TETRIMINO_COLORS[7] = {
  COLOR_I, COLOR_O, COLOR_T, COLOR_L, COLOR_J, COLOR_S, COLOR_Z
};

// Grid  - 0=empty, 1-7=obstacle (tetrimino type I,O,T,L,J,S,Z), 8=snake, 9=food
uint8_t grid[GRID_ROWS][GRID_COLS];
uint8_t prevGrid[GRID_ROWS][GRID_COLS];  // Track previous frame for flicker reduction

// Snake
#define MAX_SNAKE_LENGTH 200
struct SnakeSegment {
  int x, y;
};
SnakeSegment snake[MAX_SNAKE_LENGTH];
int snakeLength = 3;
int snakeDir = 0;  // 0=right, 1=down, 2=left, 3=up
int nextDir = 0;

// Food
int foodX, foodY;

// Game state
int score = 0;
int highScore = 0;
unsigned long lastMoveTime = 0;
int moveDelay = 200;  // Start slow, gets faster
bool gameOver = false;
bool godMode = false;

// ============================================================================
// UTILITY
// ============================================================================

void clearGrid() {
  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      grid[y][x] = 0;
      prevGrid[y][x] = 0xFF;  // Set to invalid value to force initial draw
    }
  }
}

void placeRandomTetriminos() {
  // Place one of each tetrimino type randomly on the grid
  for (int t = 0; t < 7; t++) {
    bool placed = false;
    int attempts = 0;
    
    while (!placed && attempts < 100) {
      // Random position (avoid edges)
      int anchorX = random(2, GRID_COLS - 5);
      int anchorY = random(2, GRID_ROWS - 5);
      
      // Check if all 4 blocks can be placed
      bool canPlace = true;
      for (int b = 0; b < 4; b++) {
        int bx = anchorX + TETRIMINO_SHAPES[t][b][0];
        int by = anchorY + TETRIMINO_SHAPES[t][b][1];
        
        if (bx < 0 || bx >= GRID_COLS || by < 0 || by >= GRID_ROWS || grid[by][bx] != 0) {
          canPlace = false;
          break;
        }
      }
      
      if (canPlace) {
        // Place the tetrimino - store type (1-7)
        for (int b = 0; b < 4; b++) {
          int bx = anchorX + TETRIMINO_SHAPES[t][b][0];
          int by = anchorY + TETRIMINO_SHAPES[t][b][1];
          grid[by][bx] = t + 1;  // 1-7 = tetrimino types
        }
        placed = true;
      }
      attempts++;
    }
  }
}

void spawnFood() {
  // Find empty spot for food
  int attempts = 0;
  while (attempts < 500) {  // Increased attempts
    foodX = random(0, GRID_COLS);
    foodY = random(0, GRID_ROWS);
    
    if (grid[foodY][foodX] == 0) {
      grid[foodY][foodX] = 9;  // 9 = food
      return;
    }
    attempts++;
  }
  
  // Fallback: scan entire grid for first empty spot
  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      if (grid[y][x] == 0) {
        foodX = x;
        foodY = y;
        grid[y][x] = 9;
        return;
      }
    }
  }
  
  // If we get here, grid is full - game over condition
  gameOver = true;
}

void initSnake() {
  // Place snake in center
  snakeLength = 3;
  snake[0].x = GRID_COLS / 2;
  snake[0].y = GRID_ROWS / 2;
  snake[1].x = snake[0].x - 1;
  snake[1].y = snake[0].y;
  snake[2].x = snake[0].x - 2;
  snake[2].y = snake[0].y;
  
  // Mark snake on grid
  for (int i = 0; i < snakeLength; i++) {
    grid[snake[i].y][snake[i].x] = 8;  // 8 = snake
  }
  
  snakeDir = 0;  // Start moving right
  nextDir = 0;
}

// ============================================================================
// DRAWING
// ============================================================================

void drawBlock(int x, int y, uint16_t color) {
  M5.Lcd.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE, color);
  M5.Lcd.drawRect(x, y, BLOCK_SIZE, BLOCK_SIZE, 0x2104);  // Dark border
}

void drawGrid() {
  // Only redraw cells that changed
  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      // Skip if cell hasn't changed
      if (grid[y][x] == prevGrid[y][x]) continue;
      
      int screenX = x * BLOCK_SIZE;
      int screenY = PLAY_AREA_TOP + y * BLOCK_SIZE;
      
      if (grid[y][x] == 0) {
        // Empty - clear it
        M5.Lcd.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, COLOR_BG);
      } else if (grid[y][x] >= 1 && grid[y][x] <= 7) {
        // Tetrimino obstacle - draw with specific color
        drawBlock(screenX, screenY, TETRIMINO_COLORS[grid[y][x] - 1]);
      } else if (grid[y][x] == 8) {
        // Snake
        drawBlock(screenX, screenY, COLOR_SNAKE);
      } else if (grid[y][x] == 9) {
        // Food
        drawBlock(screenX, screenY, COLOR_FOOD);
      }
      
      // Update previous grid
      prevGrid[y][x] = grid[y][x];
    }
  }
  
  // Draw blue border around play area
  M5.Lcd.drawRect(0, PLAY_AREA_TOP, SCREEN_WIDTH, PLAY_AREA_BOTTOM - PLAY_AREA_TOP, 0x001F);  // Blue
}

void drawHUD() {
  M5.Lcd.fillRect(0, 0, SCREEN_WIDTH, PLAY_AREA_TOP, COLOR_BG);
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.printf("S:%d", score);
  
  M5.Lcd.setCursor(60, 2);
  M5.Lcd.printf("L:%d", snakeLength);
  
  if (highScore > 0) {
    M5.Lcd.setCursor(2, 12);
    M5.Lcd.printf("Hi:%d", highScore);
  }
  
  if (godMode) {
    M5.Lcd.fillRect(0, PLAY_AREA_TOP - 3, SCREEN_WIDTH, 3, 0xFFE0);
  }
}

// ============================================================================
// GAME LOGIC
// ============================================================================

void handleInput() {
  M5.update();
  
  // 5-way switch direction (prevent 180 degree turns)
  if (digitalRead(PIN_LEFT) == LOW && snakeDir != 0) {  // Left
    nextDir = 2;
  } else if (digitalRead(PIN_RIGHT) == LOW && snakeDir != 2) {  // Right
    nextDir = 0;
  } else if (digitalRead(PIN_UP) == LOW && snakeDir != 1) {  // Up
    nextDir = 3;
  } else if (digitalRead(PIN_DOWN) == LOW && snakeDir != 3) {  // Down
    nextDir = 1;
  }
  
  // Button B toggles god mode
  if (M5.BtnB.wasPressed()) {
    godMode = !godMode;
  }
  
  // Button A or Center toggles pause
  if (M5.BtnA.wasPressed() || digitalRead(PIN_CENTER) == LOW) {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setCursor(35, 110);
    M5.Lcd.print("PAUSE");
    delay(300);
    while (!M5.BtnA.wasPressed() && digitalRead(PIN_CENTER) != LOW) {
      M5.update();
      delay(50);
    }
    drawGrid(); // Force redraw to clear PAUSE text
  }
}

void updateSnake() {
  // Determine new head position
  snakeDir = nextDir;
  int newHeadX = snake[0].x;
  int newHeadY = snake[0].y;
  
  switch (snakeDir) {
    case 0: newHeadX++; break; // Right
    case 1: newHeadY++; break; // Down
    case 2: newHeadX--; break; // Left
    case 3: newHeadY--; break; // Up
  }
  
  // Check bounds
  if (newHeadX < 0 || newHeadX >= GRID_COLS || newHeadY < 0 || newHeadY >= GRID_ROWS) {
    if (!godMode) gameOver = true;
    else {
      // Warp to other side in god mode
      if (newHeadX < 0) newHeadX = GRID_COLS - 1;
      else if (newHeadX >= GRID_COLS) newHeadX = 0;
      if (newHeadY < 0) newHeadY = GRID_ROWS - 1;
      else if (newHeadY >= GRID_ROWS) newHeadY = 0;
    }
  }
  
  if (gameOver) return;
  
  // Check collision with grid
  uint8_t cell = grid[newHeadY][newHeadX];
  bool grow = false;
  
  if (cell >= 1 && cell <= 7) {
    // Collision with tetrimino
    if (!godMode) {
      gameOver = true;
      return;
    }
  } else if (cell == 8) {
    // Collision with self
    if (!godMode) {
      gameOver = true;
      return;
    }
  } else if (cell == 9) {
    // Ate food
    grow = true;
    score += 10 + (snakeLength / 5);
    if (moveDelay > 50) moveDelay -= 2;
    spawnFood();
  }
  
  // Move segments
  if (!grow) {
    // Remove tail from grid
    grid[snake[snakeLength-1].y][snake[snakeLength-1].x] = 0;
  } else {
    // Increase length
    if (snakeLength < MAX_SNAKE_LENGTH) snakeLength++;
  }
  
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i-1];
  }
  
  // Update head
  snake[0].x = newHeadX;
  snake[0].y = newHeadY;
  
  // Mark head on grid
  grid[newHeadY][newHeadX] = 8;
}

void resetGame() {
  clearGrid();
  placeRandomTetriminos();
  initSnake();
  spawnFood();
  score = 0;
  moveDelay = 200;
  gameOver = false;
  lastMoveTime = millis();
  
  M5.Lcd.fillScreen(COLOR_BG);
  drawHUD();
  drawGrid();
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
  M5.begin();
  M5.Lcd.setRotation(0);
  
  // Initialize 5-way switch pins
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_CENTER, INPUT_PULLUP);
  
  resetGame();
}

void loop() {
  handleInput();
  
  if (!gameOver) {
    if (millis() - lastMoveTime > moveDelay) {
      updateSnake();
      drawGrid();
      drawHUD();
      lastMoveTime = millis();
    }
  } else {
    // Game Over screen
    if (score > highScore) highScore = score;
    
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(0xF800);  // Red
    M5.Lcd.setCursor(15, 100);
    M5.Lcd.print("GAME OVER");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setCursor(30, 130);
    M5.Lcd.printf("Score: %d", score);
    M5.Lcd.setCursor(20, 150);
    M5.Lcd.print("Press M5 to Restart");
    
    while (!M5.BtnA.wasPressed() && digitalRead(PIN_CENTER) != LOW) {
      M5.update();
      delay(50);
    }
    resetGame();
  }
  
  delay(10);
}

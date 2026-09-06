#include <M5StickCPlus2.h>
#include <math.h>

// Display constants
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define PLAY_AREA_LEFT 0
#define PLAY_AREA_RIGHT 135
#define PLAY_AREA_TOP 40
#define PLAY_AREA_BOTTOM 235
#define BLOCK_SIZE 3

// 5-way tactile switch pins
#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

// Grid constants
#define GRID_COLS 45  // 135 / 3
#define GRID_ROWS 65  // 195 / 3

// Colors
#define COLOR_SKY 0x001F        // Dark Blue
#define COLOR_TERRAIN 0x6B4D    // Brown
#define COLOR_TANK1 0x07E0      // Green (Player 1)
#define COLOR_TANK2 0xF800      // Red (Player 2)
#define COLOR_PROJECTILE 0xFFFF // White
#define COLOR_EXPLOSION 0xFD20  // Orange
#define COLOR_BG 0x0000         // Black
#define COLOR_I 0x07FF          // Cyan
#define COLOR_O 0xFFE0          // Yellow
#define COLOR_PLAYER COLOR_TANK1 // Player color

// Grid - 0=empty, 1=terrain, 2=tank1, 3=tank2
uint8_t grid[GRID_ROWS][GRID_COLS];
uint8_t prevGrid[GRID_ROWS][GRID_COLS];  // For flicker reduction

// Tank structure
struct Tank {
  int x, y;          // Base position (center of T)
  float angle;       // Firing angle (0-170 degrees, 2.5 degree increments)
  float power;       // Power (10-100, 2.5% increments)
  int hp;            // Health points
  bool active;
  uint16_t color;
};

Tank tank1, tank2;
int currentPlayer = 1;  // 1 or 2

// Projectile
struct Projectile {
  float x, y;
  float vx, vy;
  bool active;
  unsigned long lastUpdate;
};

Projectile projectile;

// Game state
int turnPhase = 0;  // 0=adjust angle, 1=adjust power, 2=firing
bool gameOver = false;
int winner = 0;

// Win/Loss tracking
int playerWins = 0;
int playerLosses = 0;

// Visual effects
int cloudX[3] = {10, 30, 50};
int cloudY[3] = {5, 12, 8};
int prevCloudX[3] = {10, 30, 50};
int birdX[2] = {20, 60};
int birdY[2] = {3, 10};
int birdFrame = 0;
unsigned long lastBirdUpdate = 0;

// Floating tetriminos
struct FloatingPiece {
  int x;
  int y;
  int prevX;
  int prevY;
  int type;  // 0-6 for different tetriminos
  uint16_t color;
  bool alive;
};
FloatingPiece floatingPieces[4];

// Falling debris particles
struct Debris {
  float x, y;
  float vx, vy;
  uint16_t color;
  bool active;
};
Debris debris[20];  // Max 20 debris particles at once

// Previous projectile position for clearing
int prevProjectileX = -1;
int prevProjectileY = -1;

// Physics
#define GRAVITY 0.3f
#define PROJECTILE_SPEED 0.5f

// Forward declarations
void handleExplosion(int cx, int cy);
void applyGravity();
void dropTankIfNeeded(Tank* tank, int tankType);
void endTurn();
void fireProjectile();
void spawnDebris(int centerX, int centerY, uint16_t color);
void updateDebris();
bool checkSkyCollision(float px, float py);

// ============================================================================
// UTILITY
// ============================================================================

void clearGrid() {
  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      grid[y][x] = 0;
      prevGrid[y][x] = 0xFF;  // Force initial redraw
    }
  }
}

void generateTerrain() {
  // Random terrain style (0-3 for variety)
  int style = random(0, 4);
  int baseHeight = GRID_ROWS - random(10, 18);  // Random base height
  
  // Create different terrain styles
  for (int x = 0; x < GRID_COLS; x++) {
    float wave1, wave2, wave3;
    int height;
    
    switch (style) {
      case 0:  // Rolling hills
        wave1 = sin(x * 0.2f) * 5.0f;
        wave2 = sin(x * 0.1f) * 8.0f;
        wave3 = sin(x * 0.05f) * 3.0f;
        height = baseHeight + (int)(wave1 + wave2 + wave3);
        break;
        
      case 1:  // Tall mountains
        wave1 = sin(x * 0.15f) * 12.0f;
        wave2 = sin(x * 0.08f) * 8.0f;
        wave3 = cos(x * 0.12f) * 6.0f;
        height = baseHeight + (int)(wave1 + wave2 + wave3);
        break;
        
      case 2:  // Sharp peaks
        wave1 = sin(x * 0.25f) * 10.0f;
        wave2 = cos(x * 0.18f) * 7.0f;
        wave3 = sin(x * 0.35f) * 5.0f;
        height = baseHeight + (int)(wave1 + wave2 + wave3);
        break;
        
      case 3:  // Mixed terrain
        wave1 = sin(x * 0.18f) * 9.0f;
        wave2 = cos(x * 0.11f) * 6.0f;
        wave3 = sin(x * 0.06f) * 11.0f;
        height = baseHeight + (int)(wave1 + wave2 + wave3);
        break;
    }
    
    // Constrain height to valid range
    height = constrain(height, GRID_ROWS - 25, GRID_ROWS - 5);
    
    // Fill from bottom up to height
    for (int y = height; y < GRID_ROWS; y++) {
      grid[y][x] = 1;  // Terrain
    }
  }
}

void placeTank(Tank* tank, int side) {
  // Random position on left or right third of map
  int searchX;
  if (side == 1) {
    // Left side - random position in left third
    searchX = random(5, GRID_COLS / 3);
  } else {
    // Right side - random position in right third
    searchX = random((GRID_COLS * 2) / 3, GRID_COLS - 5);
  }
  
  // Find top of terrain
  for (int y = 0; y < GRID_ROWS; y++) {
    if (grid[y][searchX] == 1) {
      tank->x = searchX;
      tank->y = y - 3;  // Place 3 blocks above terrain
      tank->hp = 100;
      tank->angle = (side == 1) ? 45 : 135;  // Face toward center
      tank->power = 50;
      tank->active = true;
      tank->color = (side == 1) ? COLOR_TANK1 : COLOR_TANK2;
      
      // Place upside-down T shape
      // T shape: top center, then 3 blocks across bottom
      grid[tank->y][tank->x] = (side == 1) ? 2 : 3;      // Top center
      grid[tank->y + 1][tank->x - 1] = (side == 1) ? 2 : 3;  // Bottom left
      grid[tank->y + 1][tank->x] = (side == 1) ? 2 : 3;      // Bottom center
      grid[tank->y + 1][tank->x + 1] = (side == 1) ? 2 : 3;  // Bottom right
      
      return;
    }
  }
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

void drawBlock(int x, int y, uint16_t color) {
  M5.Lcd.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE, color);
}

void drawSky() {
  // Clear previous clouds
  for (int i = 0; i < 3; i++) {
    int pcx = prevCloudX[i] * BLOCK_SIZE;
    int cy = PLAY_AREA_TOP + cloudY[i] * BLOCK_SIZE;
    M5.Lcd.fillRect(pcx, cy, 9, 3, COLOR_SKY);
  }
  
  // Clear previous tetriminos
  for (int i = 0; i < 4; i++) {
    if (floatingPieces[i].prevX >= -10) {
      int ppx = floatingPieces[i].prevX * BLOCK_SIZE;
      int ppy = PLAY_AREA_TOP + floatingPieces[i].prevY * BLOCK_SIZE;
      // Clear a larger area to cover all tetrimino shapes
      M5.Lcd.fillRect(ppx - BLOCK_SIZE, ppy, BLOCK_SIZE * 5, BLOCK_SIZE * 3, COLOR_SKY);
    }
  }
  
  // Draw clouds (simple white blocks)
  for (int i = 0; i < 3; i++) {
    int cx = cloudX[i] * BLOCK_SIZE;
    int cy = PLAY_AREA_TOP + cloudY[i] * BLOCK_SIZE;
    M5.Lcd.fillRect(cx, cy, 9, 3, 0xFFFF);  // 3x1 block cloud
    prevCloudX[i] = cloudX[i];
  }
  
  // Draw sun (yellow circle in top right)
  M5.Lcd.fillCircle(120, PLAY_AREA_TOP + 15, 8, 0xFFE0);  // Yellow
  
  // Draw floating tetriminos
  for (int i = 0; i < 4; i++) {
    if (!floatingPieces[i].alive) continue;  // Skip destroyed pieces
    
    int px = floatingPieces[i].x * BLOCK_SIZE;
    int py = PLAY_AREA_TOP + floatingPieces[i].y * BLOCK_SIZE;
    
    // Draw piece based on type
    uint16_t c = floatingPieces[i].color;
    switch (floatingPieces[i].type) {
      case 0: // I piece
        M5.Lcd.fillRect(px, py, BLOCK_SIZE * 4, BLOCK_SIZE, c);
        break;
      case 1: // O piece
        M5.Lcd.fillRect(px, py, BLOCK_SIZE * 2, BLOCK_SIZE * 2, c);
        break;
      case 2: // T piece
        M5.Lcd.fillRect(px, py + BLOCK_SIZE, BLOCK_SIZE * 3, BLOCK_SIZE, c);
        M5.Lcd.fillRect(px + BLOCK_SIZE, py, BLOCK_SIZE, BLOCK_SIZE, c);
        break;
      default: // Just a block for others
        M5.Lcd.fillRect(px, py, BLOCK_SIZE * 2, BLOCK_SIZE, c);
        break;
    }
    
    floatingPieces[i].prevX = floatingPieces[i].x;
    floatingPieces[i].prevY = floatingPieces[i].y;
  }
}

void drawGrid() {
  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      if (grid[y][x] == prevGrid[y][x]) continue;
      
      int screenX = x * BLOCK_SIZE;
      int screenY = PLAY_AREA_TOP + y * BLOCK_SIZE;
      
      uint16_t color;
      switch (grid[y][x]) {
        case 0: color = COLOR_SKY; break;
        case 1: color = COLOR_TERRAIN; break;
        case 2: color = COLOR_TANK1; break;
        case 3: color = COLOR_TANK2; break;
        default: color = COLOR_SKY; break;
      }
      
      M5.Lcd.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, color);
      prevGrid[y][x] = grid[y][x];
    }
  }
}

void drawHUD() {
  M5.Lcd.fillRect(0, 0, SCREEN_WIDTH, PLAY_AREA_TOP, COLOR_BG);
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_TANK1);
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.printf("P1: %d HP", tank1.hp);
  
  M5.Lcd.setTextColor(COLOR_TANK2);
  M5.Lcd.setCursor(SCREEN_WIDTH - 60, 2);
  M5.Lcd.printf("P2: %d HP", tank2.hp);
  
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(2, 12);
  M5.Lcd.printf("W: %d L: %d", playerWins, playerLosses);
  
  // Draw current settings for active player
  Tank* current = (currentPlayer == 1) ? &tank1 : &tank2;
  M5.Lcd.setTextColor(current->color);
  M5.Lcd.setCursor(2, 22);
  M5.Lcd.printf("Player %d Turn", currentPlayer);
  M5.Lcd.setCursor(2, 32);
  M5.Lcd.printf("Angle: %.1f Pwr: %.1f", current->angle, current->power);
  
  // Phase indicator
  M5.Lcd.setCursor(SCREEN_WIDTH - 50, 32);
  if (turnPhase == 0) M5.Lcd.print("[ANGLE]");
  else if (turnPhase == 1) M5.Lcd.print("[POWER]");
}

// ============================================================================
// GAME LOGIC
// ============================================================================

void updateVisuals() {
  // Move clouds
  if (millis() % 200 < 20) {
    for (int i = 0; i < 3; i++) {
      cloudX[i]++;
      if (cloudX[i] * BLOCK_SIZE > SCREEN_WIDTH) cloudX[i] = -10;
    }
  }
  
  // Move floating tetriminos
  if (millis() % 150 < 15) {
    for (int i = 0; i < 4; i++) {
      floatingPieces[i].x++;
      if (floatingPieces[i].x * BLOCK_SIZE > SCREEN_WIDTH) {
        floatingPieces[i].x = -10;
        floatingPieces[i].y = random(2, 20);
        floatingPieces[i].alive = true;
      }
    }
  }
  
  updateDebris();
}

void spawnDebris(int centerX, int centerY, uint16_t color) {
  for (int i = 0; i < 20; i++) {
    if (!debris[i].active) {
      debris[i].x = centerX * BLOCK_SIZE;
      debris[i].y = PLAY_AREA_TOP + centerY * BLOCK_SIZE;
      debris[i].vx = (random(-20, 21) / 10.0f);
      debris[i].vy = (random(-30, 0) / 10.0f);
      debris[i].color = color;
      debris[i].active = true;
    }
  }
}

void updateDebris() {
  for (int i = 0; i < 20; i++) {
    if (debris[i].active) {
      // Clear old position
      M5.Lcd.drawPixel((int)debris[i].x, (int)debris[i].y, COLOR_SKY);
      
      debris[i].x += debris[i].vx;
      debris[i].y += debris[i].vy;
      debris[i].vy += 0.2f;  // Gravity
      
      if (debris[i].y > PLAY_AREA_BOTTOM || debris[i].x < 0 || debris[i].x > SCREEN_WIDTH) {
        debris[i].active = false;
      } else {
        M5.Lcd.drawPixel((int)debris[i].x, (int)debris[i].y, debris[i].color);
      }
    }
  }
}

void handleInput() {
  M5.update();
  Tank* current = (currentPlayer == 1) ? &tank1 : &tank2;
  
  // Left/Right: Adjust Angle or Power
  if (digitalRead(PIN_LEFT) == LOW) {
    if (turnPhase == 0) current->angle = fmax(0.0f, current->angle - 2.5f);
    else current->power = fmax(10.0f, current->power - 2.5f);
  }
  if (digitalRead(PIN_RIGHT) == LOW) {
    if (turnPhase == 0) current->angle = fmin(180.0f, current->angle + 2.5f);
    else current->power = fmin(100.0f, current->power + 2.5f);
  }
  
  // Center or Button A: Switch phase or Fire
  if (M5.BtnA.wasPressed() || digitalRead(PIN_CENTER) == LOW) {
    if (turnPhase == 0) {
      turnPhase = 1;
    } else if (turnPhase == 1) {
      fireProjectile();
    }
    delay(200);
  }
  
  // Button B: Reset phase
  if (M5.BtnB.wasPressed()) {
    turnPhase = 0;
  }
}

void fireProjectile() {
  Tank* current = (currentPlayer == 1) ? &tank1 : &tank2;
  float rad = current->angle * PI / 180.0f;
  
  projectile.x = current->x;
  projectile.y = current->y;
  projectile.vx = cos(rad) * (current->power / 10.0f) * PROJECTILE_SPEED;
  projectile.vy = -sin(rad) * (current->power / 10.0f) * PROJECTILE_SPEED;
  projectile.active = true;
  projectile.lastUpdate = millis();
  
  turnPhase = 2;
  prevProjectileX = -1;
}

bool checkSkyCollision(float px, float py) {
  int gx = (int)(px / BLOCK_SIZE);
  int gy = (int)((py - PLAY_AREA_TOP) / BLOCK_SIZE);
  
  for (int i = 0; i < 4; i++) {
    if (!floatingPieces[i].alive) continue;
    if (abs(gx - floatingPieces[i].x) < 2 && abs(gy - floatingPieces[i].y) < 2) {
      floatingPieces[i].alive = false;
      return true;
    }
  }
  return false;
}

void updateProjectile() {
  if (!projectile.active) return;
  
  // Clear previous dot
  if (prevProjectileX >= 0) {
    M5.Lcd.drawPixel(prevProjectileX, prevProjectileY, COLOR_SKY);
  }
  
  projectile.x += projectile.vx;
  projectile.y += projectile.vy;
  projectile.vy += GRAVITY * 0.1f;
  
  int gx = (int)(projectile.x / BLOCK_SIZE);
  int gy = (int)((projectile.y - PLAY_AREA_TOP) / BLOCK_SIZE);
  
  // Draw current dot
  if (projectile.y >= PLAY_AREA_TOP && projectile.y < PLAY_AREA_BOTTOM && 
      projectile.x >= 0 && projectile.x < SCREEN_WIDTH) {
    M5.Lcd.drawPixel((int)projectile.x, (int)projectile.y, COLOR_PROJECTILE);
    prevProjectileX = (int)projectile.x;
    prevProjectileY = (int)projectile.y;
  }
  
  // Check collisions
  if (gy >= 0 && gy < GRID_ROWS && gx >= 0 && gx < GRID_COLS) {
    uint8_t hit = grid[gy][gx];
    if (hit != 0 || checkSkyCollision(projectile.x, projectile.y)) {
      handleExplosion(gx, gy);
      projectile.active = false;
      endTurn();
    }
  } else if (projectile.y > PLAY_AREA_BOTTOM || projectile.x < 0 || projectile.x > SCREEN_WIDTH) {
    projectile.active = false;
    endTurn();
  }
}

void handleExplosion(int cx, int cy) {
  int radius = 4;
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if (x*x + y*y <= radius*radius) {
        int gx = cx + x;
        int gy = cy + y;
        if (gx >= 0 && gx < GRID_COLS && gy >= 0 && gy < GRID_ROWS) {
          if (grid[gy][gx] == 2) tank1.hp -= 10;
          if (grid[gy][gx] == 3) tank2.hp -= 10;
          grid[gy][gx] = 0;
        }
      }
    }
  }
  spawnDebris(cx, cy, COLOR_EXPLOSION);
  applyGravity();
}

void applyGravity() {
  for (int x = 0; x < GRID_COLS; x++) {
    for (int y = GRID_ROWS - 2; y >= 0; y--) {
      if (grid[y][x] == 1 && grid[y+1][x] == 0) {
        int drop = y;
        while (drop < GRID_ROWS - 1 && grid[drop+1][x] == 0) {
          grid[drop+1][x] = 1;
          grid[drop][x] = 0;
          drop++;
        }
      }
    }
  }
  dropTankIfNeeded(&tank1, 2);
  dropTankIfNeeded(&tank2, 3);
}

void dropTankIfNeeded(Tank* tank, int tankType) {
  if (!tank->active) return;
  while (tank->y < GRID_ROWS - 2 && grid[tank->y + 2][tank->x] == 0) {
    // Clear old
    grid[tank->y][tank->x] = 0;
    grid[tank->y + 1][tank->x - 1] = 0;
    grid[tank->y + 1][tank->x] = 0;
    grid[tank->y + 1][tank->x + 1] = 0;
    
    tank->y++;
    
    // Set new
    grid[tank->y][tank->x] = tankType;
    grid[tank->y + 1][tank->x - 1] = tankType;
    grid[tank->y + 1][tank->x] = tankType;
    grid[tank->y + 1][tank->x + 1] = tankType;
  }
}

void endTurn() {
  currentPlayer = (currentPlayer == 1) ? 2 : 1;
  turnPhase = 0;
  if (tank1.hp <= 0 || tank2.hp <= 0) {
    gameOver = true;
    winner = (tank1.hp > 0) ? 1 : 2;
    if (winner == 1) playerWins++; else playerLosses++;
  }
}

void initGame() {
  clearGrid();
  generateTerrain();
  placeTank(&tank1, 1);
  placeTank(&tank2, 2);
  
  for (int i = 0; i < 4; i++) {
    floatingPieces[i].x = random(0, GRID_COLS);
    floatingPieces[i].y = random(2, 20);
    floatingPieces[i].type = random(0, 3);
    floatingPieces[i].color = (i == 0) ? COLOR_I : (i == 1) ? COLOR_O : COLOR_PLAYER;
    floatingPieces[i].alive = true;
    floatingPieces[i].prevX = -100;
  }
  
  currentPlayer = 1;
  turnPhase = 0;
  gameOver = false;
  projectile.active = false;
  M5.Lcd.fillScreen(COLOR_SKY);
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(0);
  
  // Initialize 5-way switch pins
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_CENTER, INPUT_PULLUP);
  
  initGame();
}

void loop() {
  if (!gameOver) {
    handleInput();
    updateProjectile();
    updateVisuals();
    
    drawSky();
    drawGrid();
    drawHUD();
  } else {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 100);
    M5.Lcd.printf("P%d WINS!", winner);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 140);
    M5.Lcd.print("Press M5 to Restart");
    
    M5.update();
    if (M5.BtnA.wasPressed() || digitalRead(PIN_CENTER) == LOW) {
      initGame();
    }
  }
  delay(10);
}

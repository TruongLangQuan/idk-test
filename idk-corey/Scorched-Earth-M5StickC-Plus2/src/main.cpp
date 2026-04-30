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
// DRAWING
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
    
    // Store previous position
    floatingPieces[i].prevX = floatingPieces[i].x;
    floatingPieces[i].prevY = floatingPieces[i].y;
    
    // Draw different tetrimino shapes
    switch (floatingPieces[i].type) {
      case 0: // I-piece (horizontal)
        M5.Lcd.fillRect(px, py, BLOCK_SIZE * 4, BLOCK_SIZE, floatingPieces[i].color);
        break;
      case 1: // O-piece (square)
        M5.Lcd.fillRect(px, py, BLOCK_SIZE * 2, BLOCK_SIZE * 2, floatingPieces[i].color);
        break;
      case 2: // T-piece
        M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, floatingPieces[i].color);
        M5.Lcd.fillRect(px - BLOCK_SIZE, py + BLOCK_SIZE, BLOCK_SIZE * 3, BLOCK_SIZE, floatingPieces[i].color);
        break;
      case 3: // L-piece
        M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE * 2, floatingPieces[i].color);
        M5.Lcd.fillRect(px + BLOCK_SIZE, py + BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, floatingPieces[i].color);
        break;
      case 4: // Z-piece
        M5.Lcd.fillRect(px, py, BLOCK_SIZE * 2, BLOCK_SIZE, floatingPieces[i].color);
        M5.Lcd.fillRect(px + BLOCK_SIZE, py + BLOCK_SIZE, BLOCK_SIZE * 2, BLOCK_SIZE, floatingPieces[i].color);
        break;
      case 5: // S-piece
        M5.Lcd.fillRect(px + BLOCK_SIZE, py, BLOCK_SIZE * 2, BLOCK_SIZE, floatingPieces[i].color);
        M5.Lcd.fillRect(px, py + BLOCK_SIZE, BLOCK_SIZE * 2, BLOCK_SIZE, floatingPieces[i].color);
        break;
      case 6: // J-piece
        M5.Lcd.fillRect(px + BLOCK_SIZE, py, BLOCK_SIZE, BLOCK_SIZE * 2, floatingPieces[i].color);
        M5.Lcd.fillRect(px, py + BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, floatingPieces[i].color);
        break;
    }
  }
}

void updateSky() {
  unsigned long now = millis();
  
  // Update every 300ms
  if (now - lastBirdUpdate > 300) {
    lastBirdUpdate = now;
    birdFrame++;
    
    // Move floating tetriminos (faster than clouds)
    for (int i = 0; i < 4; i++) {
      // Respawn if destroyed
      if (!floatingPieces[i].alive && floatingPieces[i].x < -10) {
        floatingPieces[i].x = -5;
        floatingPieces[i].y = random(15, 22);
        floatingPieces[i].type = random(0, 7);
        uint16_t colors[] = {0x07FF, 0xFFE0, 0xF81F, 0x07E0, 0xF800, 0x001F, 0xFD20};
        floatingPieces[i].color = colors[floatingPieces[i].type];
        floatingPieces[i].alive = true;
      }
      
      floatingPieces[i].x++;
      if (floatingPieces[i].x > GRID_COLS + 5) {
        floatingPieces[i].x = -5;
        floatingPieces[i].y = random(15, 22);  // Below clouds
        floatingPieces[i].type = random(0, 7);
        
        // Assign colors
        uint16_t colors[] = {0x07FF, 0xFFE0, 0xF81F, 0x07E0, 0xF800, 0x001F, 0xFD20};
        floatingPieces[i].color = colors[floatingPieces[i].type];
        floatingPieces[i].alive = true;
      }
    }
    
    // Slowly move clouds
    if (birdFrame % 5 == 0) {
      for (int i = 0; i < 3; i++) {
        cloudX[i]++;
        if (cloudX[i] > GRID_COLS) {
          cloudX[i] = -5;
        }
      }
    }
  }
}

void drawGrid() {
  // Only redraw changed cells
  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      if (grid[y][x] == prevGrid[y][x]) continue;  // Skip unchanged
      
      int screenX = x * BLOCK_SIZE;
      int screenY = PLAY_AREA_TOP + y * BLOCK_SIZE;
      
      if (grid[y][x] == 0) {
        drawBlock(screenX, screenY, COLOR_SKY);
      } else if (grid[y][x] == 1) {
        drawBlock(screenX, screenY, COLOR_TERRAIN);
      } else if (grid[y][x] == 2) {
        drawBlock(screenX, screenY, COLOR_TANK1);
      } else if (grid[y][x] == 3) {
        drawBlock(screenX, screenY, COLOR_TANK2);
      }
      
      prevGrid[y][x] = grid[y][x];
    }
  }
}

void drawTankAngleIndicator(Tank* tank, int playerNum) {
  // Only draw for player 1
  if (playerNum != 1) return;
  
  // Clear previous arrow area (small rectangle around tank)
  int centerX = tank->x * BLOCK_SIZE + BLOCK_SIZE / 2;
  int centerY = PLAY_AREA_TOP + tank->y * BLOCK_SIZE + BLOCK_SIZE / 2;
  M5.Lcd.fillRect(centerX - 15, centerY - 15, 30, 30, COLOR_SKY);
  
  // Redraw tank blocks that might have been cleared
  for (int dy = -1; dy <= 2; dy++) {
    for (int dx = -2; dx <= 2; dx++) {
      int gx = tank->x + dx;
      int gy = tank->y + dy;
      if (gx >= 0 && gx < GRID_COLS && gy >= 0 && gy < GRID_ROWS) {
        if (grid[gy][gx] == 2) {
          drawBlock(gx * BLOCK_SIZE, PLAY_AREA_TOP + gy * BLOCK_SIZE, COLOR_TANK1);
        }
      }
    }
  }
  
  // Draw arrow showing firing angle (just a line, no dot)
  float angleRad = tank->angle * PI / 180.0f;
  int arrowLen = 6;  // pixels (half of previous 12)
  int endX = centerX + cos(angleRad) * arrowLen;
  int endY = centerY - sin(angleRad) * arrowLen;
  
  // Draw line only - no circle at end
  M5.Lcd.drawLine(centerX, centerY, endX, endY, 0xFFFF);  // White arrow
}

void drawProjectile() {
  // Clear previous projectile position
  if (prevProjectileX >= 0 && prevProjectileY >= 0) {
    int ppx = prevProjectileX * BLOCK_SIZE;
    int ppy = PLAY_AREA_TOP + prevProjectileY * BLOCK_SIZE;
    M5.Lcd.fillRect(ppx - 1, ppy - 1, BLOCK_SIZE + 2, BLOCK_SIZE + 2, COLOR_SKY);
  }
  
  if (projectile.active) {
    int px = (int)projectile.x * BLOCK_SIZE;
    int py = PLAY_AREA_TOP + (int)projectile.y * BLOCK_SIZE;
    
    // Draw bright colored block (magenta/hot pink)
    M5.Lcd.fillRect(px - 1, py - 1, BLOCK_SIZE + 2, BLOCK_SIZE + 2, 0xF81F);  // Magenta border
    M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, 0xFFFF);  // White center
    
    // Store position for clearing next frame
    prevProjectileX = (int)projectile.x;
    prevProjectileY = (int)projectile.y;
  } else {
    prevProjectileX = -1;
    prevProjectileY = -1;
  }
}

void drawDebris() {
  for (int i = 0; i < 20; i++) {
    if (!debris[i].active) continue;
    
    int dx = (int)(debris[i].x * BLOCK_SIZE);
    int dy = PLAY_AREA_TOP + (int)(debris[i].y * BLOCK_SIZE);
    
    // Draw small colored pixel
    M5.Lcd.fillRect(dx, dy, 2, 2, debris[i].color);
  }
}

void drawHUD() {
  M5.Lcd.fillRect(0, 0, SCREEN_WIDTH, PLAY_AREA_TOP, COLOR_BG);
  
  M5.Lcd.setTextSize(1);
  
  // Win/Loss counter at top
  M5.Lcd.setTextColor(0x07E0);  // Green for wins
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.printf("W:%d", playerWins);
  
  M5.Lcd.setTextColor(0xF800);  // Red for losses
  M5.Lcd.setCursor(40, 2);
  M5.Lcd.printf("L:%d", playerLosses);
  
  // Current player indicator
  if (currentPlayer == 1) {
    M5.Lcd.setTextColor(COLOR_TANK1);
    M5.Lcd.setCursor(80, 2);
    M5.Lcd.print("[YOU]");
  } else {
    M5.Lcd.setTextColor(COLOR_TANK2);
    M5.Lcd.setCursor(80, 2);
    M5.Lcd.print("[AI]");
  }
  
  // Tank stats
  Tank* curTank = (currentPlayer == 1) ? &tank1 : &tank2;
  
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(2, 12);
  M5.Lcd.printf("HP:%d", curTank->hp);
  
  M5.Lcd.setCursor(50, 12);
  M5.Lcd.printf("A:%.1f", curTank->angle);
  
  M5.Lcd.setCursor(2, 22);
  M5.Lcd.printf("P:%.1f", curTank->power);
  
  // Phase indicator
  M5.Lcd.setCursor(50, 22);
  if (currentPlayer == 2) {
    M5.Lcd.print("AI...");
  } else if (turnPhase == 0) {
    M5.Lcd.print("ANG P/B");
  } else if (turnPhase == 1) {
    M5.Lcd.print("PWR A/B");
  } else if (turnPhase == 2) {
    M5.Lcd.print("FIRING");
  }
  
  // Health bars
  M5.Lcd.setCursor(2, 32);
  M5.Lcd.setTextColor(COLOR_TANK1);
  M5.Lcd.printf("P1:%d", tank1.hp);
  
  M5.Lcd.setCursor(70, 32);
  M5.Lcd.setTextColor(COLOR_TANK2);
  M5.Lcd.printf("AI:%d", tank2.hp);
}

// ============================================================================
// GAME LOGIC
// ============================================================================

void handleInput() {
  M5.update();
  
  // AI player 2 - auto fire with calculated values
  if (currentPlayer == 2) {
    if (turnPhase == 0) {
      // AI calculates angle toward player (AI is on right, shoots left)
      int dx = tank2.x - tank1.x;  // Distance (positive = target is to left)
      int dy = tank2.y - tank1.y;  // Height difference
      
      // AI shoots left toward player
      // Use angles 100-170 (110-170 is more realistic arc)
      int baseAngle = 130;  // Good starting angle
      
      // Adjust based on distance
      if (dx > 30) {
        // Far away - use higher angle for arc
        baseAngle = random(120, 151);
      } else if (dx > 15) {
        // Medium distance
        baseAngle = random(110, 141);
      } else {
        // Close - lower angle, more direct
        baseAngle = random(100, 131);
      }
      
      // Add some randomness (±8 degrees)
      tank2.angle = baseAngle + random(-8, 9);
      tank2.angle = constrain(tank2.angle, 100, 170);
      
      turnPhase = 1;
      delay(500);
    } else if (turnPhase == 1) {
      // AI calculates power based on distance
      int distance = abs(tank2.x - tank1.x);
      
      // Scale power: closer = less, farther = more
      // Use 25-45 range (reduced high end from 55)
      int basePower = map(distance, 5, GRID_COLS, 25, 45);
      basePower = constrain(basePower, 25, 45);
      
      // Add randomness (±8) to make imperfect but closer
      tank2.power = basePower + random(-8, 9);
      tank2.power = constrain(tank2.power, 20, 50);
      
      turnPhase = 2;
      fireProjectile();
      delay(500);
    }
    return;
  }
  
  // Player 1 controls
  if (turnPhase == 2) return;  // No input during firing
  
  Tank* curTank = &tank1;
  
  static unsigned long lastAdjust = 0;
  static bool lastPwrBtn = false;
  static bool lastBtnA = false;
  static bool lastBtnB = false;
  static unsigned long pwrHoldStart = 0;
  static unsigned long btnAHoldStart = 0;
  
  unsigned long now = millis();
  
  if (now - lastAdjust < 100) {
    // Update button states
    lastPwrBtn = M5.BtnPWR.isPressed();
    lastBtnA = M5.BtnA.isPressed();
    lastBtnB = M5.BtnB.isPressed();
    return;  // Debounce
  }
  
  if (turnPhase == 0) {
    // Adjust angle with PWR (left) and BtnB (right) - 2.5 degree increments
    // PWR button increases angle (aim more left/up)
    if (M5.BtnPWR.isPressed()) {
      if (!lastPwrBtn) {
        pwrHoldStart = now;
      }
      curTank->angle += 2.5;  // Swapped: was -=
      if (curTank->angle > 170) curTank->angle = 170;
      lastAdjust = now;
    } else {
      pwrHoldStart = 0;
    }
    
    // BtnB decreases angle (aim more right/down)
    if (M5.BtnB.isPressed()) {
      curTank->angle -= 2.5;  // Swapped: was +=
      if (curTank->angle < 10) curTank->angle = 10;
      lastAdjust = now;
    }
    
    // Press middle button to confirm and move to power adjustment
    if (M5.BtnA.isPressed() && !lastBtnA) {
      turnPhase = 1;
      lastAdjust = now;
    }
    
  } else if (turnPhase == 1) {
    // Adjust power - BtnA cycles through power levels (2.5% increments)
    // Hold BtnA to reverse direction
    if (M5.BtnA.isPressed()) {
      if (!lastBtnA) {
        btnAHoldStart = now;
        // Short press increases
        curTank->power += 2.5;
        if (curTank->power > 100) curTank->power = 10;
        lastAdjust = now;
      } else if (now - btnAHoldStart > 500) {
        // Holding decreases after 0.5 seconds
        curTank->power -= 2.5;
        if (curTank->power < 10) curTank->power = 100;
        lastAdjust = now;
      }
    } else {
      btnAHoldStart = 0;
    }
    
    // Press BtnB to fire
    if (M5.BtnB.isPressed() && !lastBtnB) {
      turnPhase = 2;
      fireProjectile();
      lastAdjust = now;
    }
  }
  
  // Update button states
  lastPwrBtn = M5.BtnPWR.isPressed();
  lastBtnA = M5.BtnA.isPressed();
  lastBtnB = M5.BtnB.isPressed();
}

void fireProjectile() {
  Tank* curTank = (currentPlayer == 1) ? &tank1 : &tank2;
  
  // Start from top of tank (center of T)
  projectile.x = curTank->x;
  projectile.y = curTank->y;
  
  // Calculate velocity based on angle and power
  float angleRad = curTank->angle * PI / 180.0f;
  float powerScale = curTank->power / 100.0f;
  
  // Increased power multiplier for longer range (was 2.0f)
  projectile.vx = cos(angleRad) * powerScale * 8.0f;  // Much more powerful!
  projectile.vy = -sin(angleRad) * powerScale * 8.0f;  // Negative is up
  
  projectile.active = true;
  projectile.lastUpdate = millis();
}

void updateProjectile() {
  if (!projectile.active) return;
  
  unsigned long now = millis();
  if (now - projectile.lastUpdate < 20) return;  // 50 FPS
  projectile.lastUpdate = now;
  
  // Apply gravity
  projectile.vy += GRAVITY;
  
  // Update position
  projectile.x += projectile.vx;
  projectile.y += projectile.vy;
  
  // Check bounds
  if (projectile.x < 0 || projectile.x >= GRID_COLS || 
      projectile.y < 0 || projectile.y >= GRID_ROWS) {
    // Missed
    projectile.active = false;
    endTurn();
    return;
  }
  
  // Check collision
  int gx = (int)projectile.x;
  int gy = (int)projectile.y;
  
  if (grid[gy][gx] != 0) {
    // Hit something!
    handleExplosion(gx, gy);
    projectile.active = false;
    endTurn();
  }
}

bool checkSkyCollision(float px, float py) {
  // Convert to grid coords
  int gx = (int)px;
  int gy = (int)py;
  
  // Only check if in sky area (above terrain)
  if (gy > 25) return false;
  
  // Check clouds
  for (int i = 0; i < 3; i++) {
    int cx = cloudX[i];
    int cy = cloudY[i];
    
    // Cloud is 3 blocks wide, 1 block tall
    if (gx >= cx && gx < cx + 3 && gy == cy) {
      // Hit cloud! Spawn white debris
      spawnDebris(gx, gy, 0xFFFF);
      return true;
    }
  }
  
  // Check floating tetriminos
  for (int i = 0; i < 4; i++) {
    if (!floatingPieces[i].alive) continue;
    
    int fx = floatingPieces[i].x;
    int fy = floatingPieces[i].y;
    int type = floatingPieces[i].type;
    
    // Check hit based on type shape
    bool hit = false;
    switch (type) {
      case 0: // I-piece: 4 blocks horizontal
        if (gy == fy && gx >= fx && gx < fx + 4) hit = true;
        break;
      case 1: // O-piece: 2x2
        if (gx >= fx && gx < fx + 2 && gy >= fy && gy < fy + 2) hit = true;
        break;
      case 2: // T-piece
        if ((gy == fy && gx >= fx - 1 && gx <= fx + 1) || 
            (gy == fy + 1 && gx == fx)) hit = true;
        break;
      case 3: // L-piece
        if ((gx == fx && (gy == fy || gy == fy + 1)) || 
            (gy == fy + 1 && gx == fx + 1)) hit = true;
        break;
      case 4: // Z-piece
        if ((gy == fy && (gx == fx || gx == fx + 1)) || 
            (gy == fy + 1 && (gx == fx + 1 || gx == fx + 2))) hit = true;
        break;
      default:
        // Simplified check for other pieces
        if (gx >= fx - 1 && gx < fx + 3 && gy >= fy && gy < fy + 2) hit = true;
        break;
    }
    
    if (hit) {
      spawnDebris(fx, fy, floatingPieces[i].color);
      floatingPieces[i].alive = false;
      floatingPieces[i].x = -100;  // Move off screen
      return true;
    }
  }
  
  return false;
}

void spawnDebris(int centerX, int centerY, uint16_t color) {
  // Spawn 8-12 debris particles
  int numDebris = random(8, 13);
  int spawned = 0;
  
  for (int i = 0; i < 20 && spawned < numDebris; i++) {
    if (!debris[i].active) {
      debris[i].x = centerX + random(-2, 3) * 0.5f;
      debris[i].y = centerY;
      debris[i].vx = random(-3, 4) * 0.3f;
      debris[i].vy = random(-2, 1) * 0.5f;  // Mostly downward
      debris[i].color = color;
      debris[i].active = true;
      spawned++;
    }
  }
}

void updateDebris() {
  bool anyActive = false;
  
  for (int i = 0; i < 20; i++) {
    if (!debris[i].active) continue;
    
    anyActive = true;
    
    // Apply gravity
    debris[i].vy += GRAVITY * 0.5f;  // Half gravity for visual effect
    
    // Update position
    debris[i].x += debris[i].vx;
    debris[i].y += debris[i].vy;
    
    // Check if hit ground/terrain
    int gx = (int)debris[i].x;
    int gy = (int)debris[i].y;
    
    if (gy < 0 || gx < 0 || gx >= GRID_COLS) {
      debris[i].active = false;
      continue;
    }
    
    if (gy >= GRID_ROWS) {
      debris[i].active = false;
      continue;
    }
    
    // Check collision with terrain - just stop
    if (grid[gy][gx] != 0) {
      debris[i].active = false;
    }
  }
}

void handleExplosion(int cx, int cy) {
  // Explosion radius of 2 blocks
  int radius = 2;
  
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      int x = cx + dx;
      int y = cy + dy;
      
      if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS) continue;
      
      // Circle check
      if (dx*dx + dy*dy <= radius*radius) {
        if (grid[y][x] == 2) {
          // Hit tank 1
          tank1.hp -= 25;
          if (tank1.hp <= 0) {
            tank1.active = false;
            gameOver = true;
            winner = 2;
            playerLosses++;  // Player lost
          }
        } else if (grid[y][x] == 3) {
          // Hit tank 2
          tank2.hp -= 25;
          if (tank2.hp <= 0) {
            tank2.active = false;
            gameOver = true;
            winner = 1;
            playerWins++;  // Player won
          }
        } else if (grid[y][x] == 1) {
          // Destroy terrain
          grid[y][x] = 0;
        }
      }
    }
  }
  
  // Apply gravity to terrain
  applyGravity();
}

void applyGravity() {
  // Let unsupported blocks fall (simple version - scan bottom to top)
  for (int pass = 0; pass < 10; pass++) {  // Multiple passes for cascading
    bool changed = false;
    
    for (int y = GRID_ROWS - 2; y >= 0; y--) {
      for (int x = 0; x < GRID_COLS; x++) {
        if (grid[y][x] == 1 && grid[y + 1][x] == 0) {
          // Terrain block with empty space below - drop it
          grid[y + 1][x] = 1;
          grid[y][x] = 0;
          changed = true;
        }
      }
    }
    
    if (!changed) break;
  }
  
  // Drop tanks if no support
  dropTankIfNeeded(&tank1, 2);
  dropTankIfNeeded(&tank2, 3);
}

void dropTankIfNeeded(Tank* tank, int tankType) {
  if (!tank->active) return;
  
  // Check if any tank block has support below
  bool hasSupport = false;
  
  for (int y = 0; y < GRID_ROWS - 1; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      if (grid[y][x] == tankType) {
        // Check below this block
        if (y + 1 < GRID_ROWS && (grid[y + 1][x] == 1 || grid[y + 1][x] == tankType)) {
          hasSupport = true;
          break;
        }
      }
    }
    if (hasSupport) break;
  }
  
  if (!hasSupport) {
    // Drop tank one row
    for (int y = GRID_ROWS - 1; y >= 0; y--) {
      for (int x = 0; x < GRID_COLS; x++) {
        if (grid[y][x] == tankType && y + 1 < GRID_ROWS) {
          grid[y + 1][x] = tankType;
          grid[y][x] = 0;
        }
      }
    }
    tank->y++;
  }
}

void endTurn() {
  // Switch player
  currentPlayer = (currentPlayer == 1) ? 2 : 1;
  turnPhase = 0;
  delay(500);
}

void resetGame() {
  clearGrid();
  generateTerrain();
  
  // Always player on left, AI on right
  placeTank(&tank1, 1);
  placeTank(&tank2, 2);
  
  currentPlayer = 1;
  turnPhase = 0;
  gameOver = false;
  winner = 0;
  projectile.active = false;
  
  // Initialize floating tetriminos
  uint16_t colors[] = {0x07FF, 0xFFE0, 0xF81F, 0x07E0, 0xF800, 0x001F, 0xFD20};
  for (int i = 0; i < 4; i++) {
    floatingPieces[i].x = random(-5, GRID_COLS);
    floatingPieces[i].y = random(15, 22);
    floatingPieces[i].prevX = floatingPieces[i].x;
    floatingPieces[i].prevY = floatingPieces[i].y;
    floatingPieces[i].type = random(0, 7);
    floatingPieces[i].color = colors[floatingPieces[i].type];
    floatingPieces[i].alive = true;
  }
  
  // Clear all debris
  for (int i = 0; i < 20; i++) {
    debris[i].active = false;
  }
  
  // Clear sky area
  M5.Lcd.fillRect(0, PLAY_AREA_TOP, SCREEN_WIDTH, 25 * BLOCK_SIZE, COLOR_SKY);
}

void showGameOver() {
  M5.Lcd.fillScreen(COLOR_BG);
  M5.Lcd.setTextSize(2);
  
  if (winner == 1) {
    M5.Lcd.setTextColor(COLOR_TANK1);
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.print("PLAYER 1");
  } else {
    M5.Lcd.setTextColor(COLOR_TANK2);
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.print("PLAYER 2");
  }
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(40, 130);
  M5.Lcd.print("WINS!");
  
  M5.Lcd.setCursor(20, 160);
  M5.Lcd.print("Press BtnA");
  M5.Lcd.setCursor(20, 175);
  M5.Lcd.print("to restart");
  
  while (!M5.BtnA.isPressed()) {
    M5.update();
    delay(50);
  }
  
  delay(200);
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  M5.Lcd.setRotation(0);  // Portrait mode
  M5.Lcd.fillScreen(COLOR_SKY);
  
  Serial.begin(115200);
  Serial.println("ScorchedM5 Starting...");
  
  // Title screen
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(5, 80);
  M5.Lcd.print("ScorchedM5");
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 110);
  M5.Lcd.print("Tetris Artillery");
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.print("Player vs AI");
  M5.Lcd.setCursor(10, 150);
  M5.Lcd.print("Press BtnA to start");
  
  while (!M5.BtnA.isPressed()) {
    M5.update();
    delay(50);
  }
  
  delay(200);
  resetGame();
}

void loop() {
  M5.update();
  
  if (gameOver) {
    showGameOver();
    resetGame();
    return;
  }
  
  // Update
  handleInput();
  updateProjectile();
  updateDebris();
  updateSky();
  
  // Draw (no full clear - only changed cells)
  drawSky();
  drawGrid();
  drawTankAngleIndicator(&tank1, 1);  // Only player tank
  drawProjectile();
  drawDebris();
  drawHUD();
  
  delay(20);
}

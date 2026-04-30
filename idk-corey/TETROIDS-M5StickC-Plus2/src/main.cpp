/*
 * TETROIDS - Tetris meets Asteroids
 * M5StickC Plus2 with Mini JoyC Hat
 * 
 * Player: T-piece ship that rotates and shoots
 * Enemies: Floating Tetris pieces (asteroids)
 * 
 * Based on M5StickDodge4Wave architecture
 */

#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"
#include <math.h>

// Display configuration
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define PLAY_AREA_TOP 12
#define PLAY_AREA_BOTTOM 220
#define BLOCK_SIZE 8

// Game constants
#define MAX_BULLETS 4
#define MAX_ASTEROIDS 15
#define MAX_BLOCKS 30

// JoyC
UNIT_JOYC Joystick;
#define JoyC_ADDR 0x54  // Mini JoyC Hat I2C address

// Colors (Tetris themed)
#define COLOR_PLAYER 0xF81F    // Magenta (T-piece)
#define COLOR_BULLET 0xFFFF    // White
#define COLOR_I 0x07FF         // Cyan
#define COLOR_O 0xFFE0         // Yellow
#define COLOR_L 0xFD20         // Orange
#define COLOR_J 0x001F         // Blue
#define COLOR_S 0x07E0         // Green
#define COLOR_Z 0xF800         // Red

// Physics constants
#define THRUST_POWER 0.4f
#define FRICTION 0.98f
#define MAX_SPEED 4.0f
#define BULLET_SPEED 6.0f
#define ASTEROID_SPEED 1.2f

// Tetris piece shapes [type][rotation][block][x/y]
// 0=I, 1=O, 2=L, 3=J, 4=S, 5=Z (NOT T - that's the player)
const int8_t ASTEROID_SHAPES[6][4][4][2] = {
  // I-piece (4 rotations)
  {{{-1,0},{0,0},{1,0},{2,0}}, {{0,-1},{0,0},{0,1},{0,2}}, {{-1,0},{0,0},{1,0},{2,0}}, {{0,-1},{0,0},{0,1},{0,2}}},
  // O-piece (all same)
  {{{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}}},
  // L-piece
  {{{-1,0},{0,0},{1,0},{1,-1}}, {{0,-1},{0,0},{0,1},{1,1}}, {{-1,1},{-1,0},{0,0},{1,0}}, {{-1,-1},{0,-1},{0,0},{0,1}}},
  // J-piece
  {{{-1,-1},{-1,0},{0,0},{1,0}}, {{0,-1},{1,-1},{0,0},{0,1}}, {{-1,0},{0,0},{1,0},{1,1}}, {{0,-1},{0,0},{-1,1},{0,1}}},
  // S-piece
  {{{-1,0},{0,0},{0,-1},{1,-1}}, {{0,-1},{0,0},{1,0},{1,1}}, {{-1,0},{0,0},{0,-1},{1,-1}}, {{0,-1},{0,0},{1,0},{1,1}}},
  // Z-piece
  {{{-1,-1},{0,-1},{0,0},{1,0}}, {{1,-1},{1,0},{0,0},{0,1}}, {{-1,-1},{0,-1},{0,0},{1,0}}, {{1,-1},{1,0},{0,0},{0,1}}}
};

const uint16_t ASTEROID_COLORS[6] = {COLOR_I, COLOR_O, COLOR_L, COLOR_J, COLOR_S, COLOR_Z};

// Player ship (T-piece) in 8 rotations [rotation][block][x/y]
const int8_t PLAYER_SHAPE[8][4][2] = {
  {{0,-1},{-1,0},{0,0},{1,0}},  // 0: UP
  {{0,-1},{0,0},{1,0},{0,1}},   // 1: UP-RIGHT
  {{-1,0},{0,0},{1,0},{0,1}},   // 2: RIGHT
  {{0,-1},{-1,0},{0,0},{0,1}},  // 3: DOWN-RIGHT
  {{-1,0},{0,0},{1,0},{0,1}},   // 4: DOWN
  {{0,-1},{0,0},{-1,0},{0,1}},  // 5: DOWN-LEFT
  {{-1,0},{0,0},{1,0},{0,-1}},  // 6: LEFT
  {{0,-1},{1,0},{0,0},{0,1}}    // 7: UP-LEFT
};

// Forward direction vectors for each rotation
const float DIR_X[8] = {0, 0.707f, 1, 0.707f, 0, -0.707f, -1, -0.707f};
const float DIR_Y[8] = {-1, -0.707f, 0, 0.707f, 1, 0.707f, 0, -0.707f};

// Game objects
struct Bullet {
  float x, y;
  float vx, vy;
  bool active;
};

struct Asteroid {
  float x, y;
  float vx, vy;
  int type;        // 0-5 (I,O,L,J,S,Z)
  int rotation;    // 0-3
  float rotSpeed;  // Rotation speed
  bool active;
  bool isLarge;    // true = full piece, false = single block
};

struct Player {
  float x, y;
  float vx, vy;
  float angle;     // 0-360 degrees (smooth rotation)
  int lives;
  bool alive;
  unsigned long invincibleUntil;  // Timestamp for invincibility
  unsigned long lastWarpTime;     // Cooldown for warp
} player;

Bullet bullets[MAX_BULLETS];
Asteroid asteroids[MAX_ASTEROIDS];

// Game state
int score = 0;
int wave = 1;
unsigned long lastShootTime = 0;
unsigned long gameStartTime = 0;
bool gameOver = false;
bool gameStarted = false;
bool godMode = false;  // Toggle for invincibility mode
bool showHUD = true;   // Toggle HUD visibility
unsigned long btnBHoldStart = 0;  // For detecting hold

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

float wrapX(float x) {
  if (x < 0) return SCREEN_WIDTH + x;
  if (x >= SCREEN_WIDTH) return x - SCREEN_WIDTH;
  return x;
}

float wrapY(float y) {
  if (y < PLAY_AREA_TOP) return PLAY_AREA_BOTTOM - (PLAY_AREA_TOP - y);
  if (y >= PLAY_AREA_BOTTOM) return PLAY_AREA_TOP + (y - PLAY_AREA_BOTTOM);
  return y;
}

float randomFloat(float min, float max) {
  return min + (float)random(10000) / 10000.0f * (max - min);
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

void drawBlock(int x, int y, uint16_t color) {
  M5.Lcd.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE, color);
  M5.Lcd.drawRect(x, y, BLOCK_SIZE, BLOCK_SIZE, 0x2104); // Dark outline
}

void drawPlayer() {
  if (!player.alive) return;
  
  // Blink during invincibility
  bool invincible = (millis() < player.invincibleUntil);
  if (invincible && (millis() / 200) % 2 == 0) return;  // Blink effect
  
  // Convert angle to radians
  float rad = player.angle * PI / 180.0f;
  float cosA = cos(rad);
  float sinA = sin(rad);
  
  // Draw T-piece rotated smoothly
  // Base T shape: stem at top (0,-1), cross at (-1,0), (0,0), (1,0)
  int8_t baseShape[4][2] = {{0,-1},{-1,0},{0,0},{1,0}};
  
  for (int i = 0; i < 4; i++) {
    // Rotate each block around center
    float bx = baseShape[i][0] * BLOCK_SIZE;
    float by = baseShape[i][1] * BLOCK_SIZE;
    
    float rotX = bx * cosA - by * sinA;
    float rotY = bx * sinA + by * cosA;
    
    drawBlock(player.x + rotX, player.y + rotY, COLOR_PLAYER);
  }
}

void drawBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      M5.Lcd.fillCircle((int)bullets[i].x, (int)bullets[i].y, 2, COLOR_BULLET);
    }
  }
}

void drawAsteroid(Asteroid* ast) {
  if (!ast->active) return;
  
  uint16_t color = ASTEROID_COLORS[ast->type];
  
  if (ast->isLarge) {
    // Draw full tetris piece
    for (int i = 0; i < 4; i++) {
      int bx = ast->x + ASTEROID_SHAPES[ast->type][ast->rotation][i][0] * BLOCK_SIZE;
      int by = ast->y + ASTEROID_SHAPES[ast->type][ast->rotation][i][1] * BLOCK_SIZE;
      drawBlock(bx, by, color);
    }
  } else {
    // Draw single block
    drawBlock((int)ast->x, (int)ast->y, color);
  }
}

void drawAsteroids() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    drawAsteroid(&asteroids[i]);
  }
}

void drawHUD() {
  // Draw lives as red boxes at very top of screen
  M5.Lcd.fillRect(0, 0, SCREEN_WIDTH, 10, 0x0000);  // Clear top bar
  
  if (godMode) {
    // God mode indicator - yellow bar across top
    M5.Lcd.fillRect(0, 0, SCREEN_WIDTH, 10, 0xFFE0);
  } else {
    // Draw lives as red rectangles at top
    for (int i = 0; i < player.lives; i++) {
      M5.Lcd.fillRect(5 + i * 15, 2, 12, 6, 0xF800);  // Red lives
    }
  }
}

// ============================================================================
// GAME LOGIC
// ============================================================================

// Forward declarations
void spawnWave();
void initPlayer();
void initPlayer() {
  player.x = SCREEN_WIDTH / 2;
  player.y = (PLAY_AREA_TOP + PLAY_AREA_BOTTOM) / 2;
  player.vx = 0;
  player.vy = 0;
  player.angle = 0;  // Facing up
  player.lives = 3;
  player.alive = true;
  player.invincibleUntil = millis() + 2000;  // 2 seconds invincibility
  player.lastWarpTime = 0;
  Serial.printf("initPlayer() called - lives set to: %d\n", player.lives);
}

void respawnPlayer() {
  // Respawn WITHOUT resetting lives count
  player.x = SCREEN_WIDTH / 2;
  player.y = (PLAY_AREA_TOP + PLAY_AREA_BOTTOM) / 2;
  player.vx = 0;
  player.vy = 0;
  player.angle = 0;  // Facing up
  player.alive = true;
  player.invincibleUntil = millis() + 2000;  // 2 seconds invincibility
  Serial.printf("respawnPlayer() called - lives: %d\n", player.lives);
}

void warpPlayer() {
  unsigned long now = millis();
  if (now - player.lastWarpTime < 1000) return;  // 1 second cooldown
  
  Serial.println("WARP activated!");
  
  // Visual effect - white circle expanding
  for (int r = 5; r < 25; r += 5) {
    M5.Lcd.drawCircle(player.x, player.y, r, 0xFFFF);
    delay(20);
  }
  
  // Random teleport location
  player.x = randomFloat(30, SCREEN_WIDTH - 30);
  player.y = randomFloat(PLAY_AREA_TOP + 30, PLAY_AREA_BOTTOM - 30);
  player.vx = 0;  // Cancel momentum
  player.vy = 0;
  
  // Small chance of danger (10% chance to lose a life)
  if (random(100) < 10 && !godMode) {
    Serial.println("WARP MALFUNCTION!");
    // Flash red
    M5.Lcd.fillCircle(player.x, player.y, 15, 0xF800);
    delay(200);
    
    player.lives--;
    if (player.lives <= 0) {
      // Game over from warp malfunction
      M5.Lcd.fillScreen(0x0000);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(0xF800);
      M5.Lcd.setCursor(20, 100);
      M5.Lcd.print("WARP FAIL!");
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(20, 130);
      M5.Lcd.printf("Score: %d", score);
      delay(2000);
      
      // Reset game
      score = 0;
      wave = 1;
      for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;
      for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
      initPlayer();
      spawnWave();
    } else {
      respawnPlayer();  // Respawn at new location with invincibility
    }
  } else {
    // Successful warp - brief invincibility
    player.invincibleUntil = millis() + 1000;  // 1 second invincibility
    
    // Visual arrival effect - green circle
    M5.Lcd.drawCircle(player.x, player.y, 15, 0x07E0);
    delay(100);
  }
  
  player.lastWarpTime = now;
}

void shootBullet() {
  unsigned long now = millis();
  if (now - lastShootTime < 200) return; // Rate limit
  
  // Calculate forward direction based on player angle
  float rad = player.angle * PI / 180.0f;
  float dirX = sin(rad);
  float dirY = -cos(rad);  // Negative because Y increases downward
  
  // Find inactive bullet
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      // Spawn bullet from the nose of the T-piece (stem position)
      bullets[i].x = player.x + dirX * (BLOCK_SIZE * 1.5f);
      bullets[i].y = player.y + dirY * (BLOCK_SIZE * 1.5f);
      bullets[i].vx = dirX * BULLET_SPEED;
      bullets[i].vy = dirY * BULLET_SPEED;
      bullets[i].active = true;
      lastShootTime = now;
      return;
    }
  }
}

void spawnAsteroid(float x, float y, int type, bool isLarge) {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) {
      asteroids[i].x = x;
      asteroids[i].y = y;
      asteroids[i].type = type;
      asteroids[i].rotation = random(4);
      asteroids[i].rotSpeed = randomFloat(-0.05f, 0.05f);
      asteroids[i].isLarge = isLarge;
      asteroids[i].active = true;
      
      // Random velocity
      float angle = randomFloat(0, 2 * PI);
      float speed = randomFloat(0.5f, ASTEROID_SPEED);
      asteroids[i].vx = cos(angle) * speed;
      asteroids[i].vy = sin(angle) * speed;
      return;
    }
  }
}

void spawnWave() {
  int count = 3 + wave / 2;
  if (count > 8) count = 8;
  
  for (int i = 0; i < count; i++) {
    int type = random(6);
    float x, y;
    
    // Spawn at edges
    if (random(2) == 0) {
      x = random(2) == 0 ? 10 : SCREEN_WIDTH - 10;
      y = randomFloat(PLAY_AREA_TOP + 20, PLAY_AREA_BOTTOM - 20);
    } else {
      x = randomFloat(20, SCREEN_WIDTH - 20);
      y = random(2) == 0 ? PLAY_AREA_TOP + 10 : PLAY_AREA_BOTTOM - 10;
    }
    
    spawnAsteroid(x, y, type, true);
  }
}

void breakAsteroid(Asteroid* ast) {
  if (!ast->isLarge) return;
  
  // Spawn 4 small blocks at piece positions
  for (int i = 0; i < 4; i++) {
    float bx = ast->x + ASTEROID_SHAPES[ast->type][ast->rotation][i][0] * BLOCK_SIZE;
    float by = ast->y + ASTEROID_SHAPES[ast->type][ast->rotation][i][1] * BLOCK_SIZE;
    
    // Scatter outward
    float angle = atan2(by - ast->y, bx - ast->x);
    float speed = 1.5f;
    
    spawnAsteroid(bx, by, ast->type, false);
    // Set velocity for the block we just spawned
    for (int j = MAX_ASTEROIDS - 1; j >= 0; j--) {
      if (asteroids[j].active && !asteroids[j].isLarge) {
        asteroids[j].vx = cos(angle) * speed;
        asteroids[j].vy = sin(angle) * speed;
        break;
      }
    }
  }
  
  score += 50;
  ast->active = false;
}

void updatePlayer() {
  if (!player.alive) return;
  
  // Read joystick
  int joyX = Joystick.getADCValue(0);
  int joyY = Joystick.getADCValue(1);
  bool joyBtn = (Joystick.getButtonStatus() == 0);
  
  // Smooth rotation (Left/Right on joystick)
  if (joyX < 1350) {
    player.angle -= 3.0f;  // Rotate counter-clockwise
  } else if (joyX > 2950) {
    player.angle += 3.0f;  // Rotate clockwise
  }
  
  // Keep angle in 0-360 range
  if (player.angle < 0) player.angle += 360;
  if (player.angle >= 360) player.angle -= 360;
  
  // Calculate forward direction based on angle
  float rad = player.angle * PI / 180.0f;
  float dirX = sin(rad);
  float dirY = -cos(rad);  // Negative because Y increases downward
  
  // Thrust (Up on joystick)
  if (joyY > 2950) {
    player.vx += dirX * THRUST_POWER;
    player.vy += dirY * THRUST_POWER;
    
    // Limit speed
    float speed = sqrt(player.vx * player.vx + player.vy * player.vy);
    if (speed > MAX_SPEED) {
      player.vx = (player.vx / speed) * MAX_SPEED;
      player.vy = (player.vy / speed) * MAX_SPEED;
    }
  }
  
  // Shoot
  if (joyBtn) {
    shootBullet();
  }
  
  // Simple BtnB toggle for God Mode
  static bool lastBtnB = false;
  if (M5.BtnB.isPressed() && !lastBtnB) {
    godMode = !godMode;
    Serial.printf("God Mode: %s\n", godMode ? "ON" : "OFF");
    delay(200);  // Debounce
  }
  lastBtnB = M5.BtnB.isPressed();
  
  // BtnA activates WARP
  static bool lastBtnA = false;
  if (M5.BtnA.isPressed() && !lastBtnA) {
    warpPlayer();
  }
  lastBtnA = M5.BtnA.isPressed();
  
  // Apply friction
  player.vx *= FRICTION;
  player.vy *= FRICTION;
  
  // Move
  player.x += player.vx;
  player.y += player.vy;
  
  // Wrap
  player.x = wrapX(player.x);
  player.y = wrapY(player.y);
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;
    
    bullets[i].x += bullets[i].vx;
    bullets[i].y += bullets[i].vy;
    
    // Deactivate if off screen
    if (bullets[i].x < 0 || bullets[i].x >= SCREEN_WIDTH ||
        bullets[i].y < PLAY_AREA_TOP || bullets[i].y >= PLAY_AREA_BOTTOM) {
      bullets[i].active = false;
    }
  }
}

void updateAsteroids() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) continue;
    
    asteroids[i].x += asteroids[i].vx;
    asteroids[i].y += asteroids[i].vy;
    
    // Wrap
    asteroids[i].x = wrapX(asteroids[i].x);
    asteroids[i].y = wrapY(asteroids[i].y);
    
    // Rotate (large only)
    if (asteroids[i].isLarge) {
      asteroids[i].rotation += asteroids[i].rotSpeed;
      if (asteroids[i].rotation < 0) asteroids[i].rotation += 4;
      if (asteroids[i].rotation >= 4) asteroids[i].rotation -= 4;
    }
  }
}

void checkCollisions() {
  // Bullet vs Asteroid
  for (int b = 0; b < MAX_BULLETS; b++) {
    if (!bullets[b].active) continue;
    
    for (int a = 0; a < MAX_ASTEROIDS; a++) {
      if (!asteroids[a].active) continue;
      
      float dx = bullets[b].x - asteroids[a].x;
      float dy = bullets[b].y - asteroids[a].y;
      float dist = sqrt(dx*dx + dy*dy);
      
      if (dist < BLOCK_SIZE * 2) {
        bullets[b].active = false;
        
        if (asteroids[a].isLarge) {
          breakAsteroid(&asteroids[a]);
        } else {
          asteroids[a].active = false;
          score += 10;
        }
        break;
      }
    }
  }
  
  // Player vs Asteroid (only if not invincible and not in god mode)
  if (player.alive && !godMode && millis() >= player.invincibleUntil) {
    for (int a = 0; a < MAX_ASTEROIDS; a++) {
      if (!asteroids[a].active) continue;
      
      float dx = player.x - asteroids[a].x;
      float dy = player.y - asteroids[a].y;
      float dist = sqrt(dx*dx + dy*dy);
      
      // Better collision - check closer distance
      if (dist < BLOCK_SIZE * 2.0f) {  // Made more sensitive
        // Death explosion effect
        for (int i = 0; i < 8; i++) {
          float angle = (i * 45.0f) * PI / 180.0f;
          M5.Lcd.drawLine(player.x, player.y, 
                          player.x + cos(angle) * 20, 
                          player.y + sin(angle) * 20, 
                          0xFFFF);
        }
        delay(150);  // Show explosion longer
        
        player.lives--;
        Serial.printf("HIT! Lives remaining: %d\n", player.lives);
        
        if (player.lives <= 0) {
          // Auto-reset instead of game over
          Serial.println("=== ALL LIVES LOST - AUTO RESET ===");
          
          // Show quick "GAME OVER" message
          M5.Lcd.fillScreen(0x0000);
          M5.Lcd.setTextSize(2);
          M5.Lcd.setTextColor(0xF800);
          M5.Lcd.setCursor(20, 100);
          M5.Lcd.print("GAME OVER");
          M5.Lcd.setTextSize(1);
          M5.Lcd.setCursor(20, 130);
          M5.Lcd.printf("Score: %d", score);
          M5.Lcd.setCursor(20, 145);
          M5.Lcd.printf("Wave: %d", wave);
          delay(2000);  // Show for 2 seconds
          
          // Auto-reset everything
          score = 0;
          wave = 1;
          for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;
          for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
          initPlayer();  // Full reset with 3 lives
          spawnWave();
          Serial.println("Game reset complete - starting fresh");
        } else {
          respawnPlayer();  // Respawn WITHOUT resetting lives count
        }
        return;
      }
    }
  }
}

bool waveComplete() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (asteroids[i].active) return false;
  }
  return true;
}

// ============================================================================
// MAIN GAME LOOP
// ============================================================================

void setup() {
  M5.begin();
  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(0x0000);
  
  // Init JoyC
  while (!Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000L)) {
    M5.Lcd.println("JoyC not found!");
    delay(500);
  }
  
  randomSeed(esp_random());
  
  // Init game
  initPlayer();
  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;
  
  spawnWave();
  gameStartTime = millis();
  gameStarted = true;
}

void loop() {
  M5.update();
  
  if (gameOver) {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(0xF800);
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.print("GAME");
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.print("OVER");
    
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setCursor(10, 150);
    M5.Lcd.printf("Score:%d", score);
    M5.Lcd.setCursor(10, 170);
    M5.Lcd.printf("Wave: %d", wave);
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 200);
    M5.Lcd.print("Press A or B");
    M5.Lcd.setCursor(10, 210);
    M5.Lcd.print("to restart");
    
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
      // Reset
      score = 0;
      wave = 1;
      gameOver = false;
      initPlayer();
      for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;
      for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
      spawnWave();
    }
    delay(50);
    return;
  }
  
  // Check wave complete
  if (waveComplete()) {
    wave++;
    score += 100 * wave;
    
    // Extra life every 2 waves (wave 2, 4, 6, 8...)
    if (wave % 2 == 0 && player.lives < 5) {  // Cap at 5 lives max
      player.lives++;
      Serial.printf("EXTRA LIFE! Now have %d lives\n", player.lives);
      
      // Visual feedback - flash green
      M5.Lcd.fillRect(0, 0, SCREEN_WIDTH, 10, 0x07E0);
      delay(300);
    }
    
    spawnWave();
    delay(1000);
  }
  
  // Update
  updatePlayer();
  updateBullets();
  updateAsteroids();
  checkCollisions();
  
  // Draw
  // Clear entire screen below HUD and including bottom margin
  M5.Lcd.fillRect(0, 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, 0x0000);
  drawAsteroids();
  drawBullets();
  drawPlayer();
  drawHUD();
  
  delay(30); // ~33fps
}

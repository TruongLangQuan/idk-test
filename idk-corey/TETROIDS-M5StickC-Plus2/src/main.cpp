#include <M5StickCPlus2.h>
#include <math.h>

// Display configuration
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define PLAY_AREA_TOP 12
#define PLAY_AREA_BOTTOM 220
#define BLOCK_SIZE 8

// 5-way tactile switch pins
#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

// Game constants
#define MAX_BULLETS 4
#define MAX_ASTEROIDS 15
#define MAX_BLOCKS 30

// JoyC removed

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
  player.alive = true;
  player.invincibleUntil = millis() + 2000;
  Serial.printf("respawnPlayer() called - lives remaining: %d\n", player.lives);
}

void fire() {
  if (millis() - lastShootTime < 400) return;
  
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = player.x;
      bullets[i].y = player.y;
      
      float rad = player.angle * PI / 180.0f;
      bullets[i].vx = sin(rad) * BULLET_SPEED;
      bullets[i].vy = -cos(rad) * BULLET_SPEED;
      bullets[i].active = true;
      lastShootTime = millis();
      break;
    }
  }
}

void warp() {
  if (millis() - player.lastWarpTime < 1000) return;
  
  player.x = random(10, SCREEN_WIDTH - 10);
  player.y = random(PLAY_AREA_TOP + 10, PLAY_AREA_BOTTOM - 10);
  player.vx = 0;
  player.vy = 0;
  player.lastWarpTime = millis();
}

void spawnAsteroid(float x, float y, int type, bool isLarge) {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) {
      asteroids[i].x = x;
      asteroids[i].y = y;
      
      float angle = random(360) * PI / 180.0f;
      float speed = isLarge ? ASTEROID_SPEED : ASTEROID_SPEED * 1.5f;
      asteroids[i].vx = cos(angle) * speed;
      asteroids[i].vy = sin(angle) * speed;
      
      asteroids[i].type = type;
      asteroids[i].rotation = random(4);
      asteroids[i].rotSpeed = randomFloat(-5.0f, 5.0f);
      asteroids[i].active = true;
      asteroids[i].isLarge = isLarge;
      break;
    }
  }
}

void spawnWave() {
  int count = 2 + wave;
  if (count > MAX_ASTEROIDS - 5) count = MAX_ASTEROIDS - 5;
  
  for (int i = 0; i < count; i++) {
    float x, y;
    // Spawn away from center
    if (random(2)) {
      x = random(2) ? -10 : SCREEN_WIDTH + 10;
      y = random(PLAY_AREA_TOP, PLAY_AREA_BOTTOM);
    } else {
      x = random(0, SCREEN_WIDTH);
      y = random(2) ? PLAY_AREA_TOP - 10 : PLAY_AREA_BOTTOM + 10;
    }
    spawnAsteroid(x, y, random(6), true);
  }
}

void updatePlayer() {
  if (!player.alive) return;
  
  M5.update();
  
  // Input handling
  // Center or A: Thrust
  bool thrusting = (digitalRead(PIN_CENTER) == LOW || M5.BtnA.isPressed());
  if (thrusting) {
    float rad = player.angle * PI / 180.0f;
    player.vx += sin(rad) * THRUST_POWER;
    player.vy -= cos(rad) * THRUST_POWER;
    
    // Cap speed
    float speed = sqrt(player.vx * player.vx + player.vy * player.vy);
    if (speed > MAX_SPEED) {
      player.vx = (player.vx / speed) * MAX_SPEED;
      player.vy = (player.vy / speed) * MAX_SPEED;
    }
  } else {
    player.vx *= FRICTION;
    player.vy *= FRICTION;
  }
  
  // Rotation
  if (digitalRead(PIN_LEFT) == LOW) player.angle -= 8.0f;
  if (digitalRead(PIN_RIGHT) == LOW) player.angle += 8.0f;
  
  // Fire
  if (digitalRead(PIN_UP) == LOW) fire();
  
  // Warp
  if (digitalRead(PIN_DOWN) == LOW) warp();
  
  // God mode toggle - BtnB hold
  if (M5.BtnB.isPressed()) {
    if (btnBHoldStart == 0) btnBHoldStart = millis();
    else if (millis() - btnBHoldStart > 1000) {
      godMode = !godMode;
      btnBHoldStart = 0;
      delay(500); // debounce
    }
  } else {
    btnBHoldStart = 0;
  }
  
  // Physics
  player.x += player.vx;
  player.y += player.vy;
  
  player.x = wrapX(player.x);
  player.y = wrapY(player.y);
  
  // Collision with asteroids
  if (millis() > player.invincibleUntil && !godMode) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
      if (asteroids[i].active) {
        float dx = player.x - asteroids[i].x;
        float dy = player.y - asteroids[i].y;
        float distSq = dx * dx + dy * dy;
        float minDist = asteroids[i].isLarge ? 18.0f : 8.0f;
        
        if (distSq < minDist * minDist) {
          player.alive = false;
          player.lives--;
          Serial.printf("Player HIT! Lives left: %d\n", player.lives);
          
          if (player.lives <= 0) {
            gameOver = true;
          }
          break;
        }
      }
    }
  }
}

void updateAsteroids() {
  bool anyActive = false;
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (asteroids[i].active) {
      anyActive = true;
      asteroids[i].x += asteroids[i].vx;
      asteroids[i].y += asteroids[i].vy;
      
      asteroids[i].x = wrapX(asteroids[i].x);
      asteroids[i].y = wrapY(asteroids[i].y);
      
      // Piece rotation
      // asteroids[i].rotation = (int)(millis() / 500) % 4; // Simple rotation
    }
  }
  
  if (!anyActive && !gameOver) {
    wave++;
    spawnWave();
  }
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].x += bullets[i].vx;
      bullets[i].y += bullets[i].vy;
      
      // Bounds check
      if (bullets[i].x < 0 || bullets[i].x >= SCREEN_WIDTH || 
          bullets[i].y < PLAY_AREA_TOP || bullets[i].y >= PLAY_AREA_BOTTOM) {
        bullets[i].active = false;
        continue;
      }
      
      // Collision with asteroids
      for (int j = 0; j < MAX_ASTEROIDS; j++) {
        if (asteroids[j].active) {
          float dx = bullets[i].x - asteroids[j].x;
          float dy = bullets[i].y - asteroids[j].y;
          float distSq = dx * dx + dy * dy;
          float minDist = asteroids[j].isLarge ? 12.0f : 6.0f;
          
          if (distSq < minDist * minDist) {
            bullets[i].active = false;
            asteroids[j].active = false;
            score += asteroids[j].isLarge ? 20 : 50;
            
            // Split if large
            if (asteroids[j].isLarge) {
              spawnAsteroid(asteroids[j].x, asteroids[j].y, asteroids[j].type, false);
              spawnAsteroid(asteroids[j].x, asteroids[j].y, asteroids[j].type, false);
            }
            break;
          }
        }
      }
    }
  }
}

void resetGame() {
  score = 0;
  wave = 1;
  gameOver = false;
  gameStarted = true;
  
  for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;
  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  
  initPlayer();
  spawnWave();
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
  
  M5.Lcd.fillScreen(0x0000);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(0xF81F); // Magenta
  M5.Lcd.setCursor(15, 60);
  M5.Lcd.print("TETROIDS");
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(20, 100);
  M5.Lcd.print("Tetris + Asteroids");
  M5.Lcd.setCursor(15, 140);
  M5.Lcd.print("Press M5 to Start");
}

void loop() {
  if (!gameStarted) {
    M5.update();
    if (M5.BtnA.wasPressed() || digitalRead(PIN_CENTER) == LOW) {
      resetGame();
    }
    return;
  }
  
  if (!gameOver) {
    if (player.alive) {
      updatePlayer();
    } else {
      delay(1000);
      respawnPlayer();
    }
    
    updateAsteroids();
    updateBullets();
    
    // Draw
    M5.Lcd.fillScreen(0x0000);
    drawPlayer();
    drawBullets();
    drawAsteroids();
    drawHUD();
    
    M5.Lcd.setCursor(60, 15);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.printf("Score: %d", score);
  } else {
    // Game Over
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(0xF800);
    M5.Lcd.setCursor(15, 100);
    M5.Lcd.print("GAME OVER");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setCursor(30, 130);
    M5.Lcd.printf("Final Score: %d", score);
    M5.Lcd.setCursor(20, 150);
    M5.Lcd.print("Press M5 to Restart");
    
    M5.update();
    if (M5.BtnA.wasPressed() || digitalRead(PIN_CENTER) == LOW) {
      resetGame();
    }
  }
  
  delay(20);
}

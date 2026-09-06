#include <M5StickCPlus2.h>

// Display configuration
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define PLAY_AREA_TOP 20
#define PLAY_AREA_BOTTOM 220
#define BLOCK_SIZE 6

// 5-way tactile switch pins
#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

// Game constants
#define MAX_BULLETS 3
#define MAX_ENEMY_BULLETS 10
#define MAX_ENEMIES 150  // More blocks now (9x4x4 = 144)
#define MAX_PARTICLES 20
#define GRID_COLS 11
#define GRID_ROWS 5
#define ENEMY_COLS 9
#define ENEMY_ROWS 4

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
#define COLOR_BG 0x0000        // Black

// Enemy tetrimino types (small pieces)
const uint16_t ENEMY_COLORS[6] = {COLOR_I, COLOR_O, COLOR_L, COLOR_J, COLOR_S, COLOR_Z};

// Small enemy shapes - 2x2 or 3-block patterns [type][block][x/y]
const int8_t ENEMY_SHAPES[6][4][2] = {
  {{0,0},{1,0},{2,0},{-1,-1}},   // I-piece (horizontal line)
  {{0,0},{1,0},{0,1},{1,1}},     // O-piece (square)
  {{0,0},{1,0},{0,1},{-1,-1}},   // L-piece (corner)
  {{0,0},{1,0},{1,1},{-1,-1}},   // J-piece (reverse corner)
  {{0,0},{1,0},{1,1},{-1,-1}},   // S-piece (step)
  {{0,0},{1,0},{0,1},{-1,-1}}    // Z-piece (reverse step)
};

// Game objects
struct Bullet {
  float x, y;
  float vy;
  bool active;
};

struct EnemyBullet {
  float x, y;
  float vy;
  bool active;
};

struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  unsigned long endTime;
  bool active;
};

struct Enemy {
  int gridX, gridY;  // Grid position (in formation)
  int offsetX, offsetY; // Random offset within cell
  int type;          // 0-5 (tetrimino type for color)
  bool active;
};

struct Player {
  float x, y;
  int lives;
  bool alive;
  unsigned long invincibleUntil;
} player;

Bullet bullets[MAX_BULLETS];
EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];
Enemy enemies[MAX_ENEMIES];
Particle particles[MAX_PARTICLES];

// Enemy formation state
int enemyDirection = 1;  // 1=right, -1=left
float enemyGroupX = 0;   // Offset from starting position
float enemyGroupY = 0;
float enemySpeed = 0.3f;
unsigned long lastEnemyMoveTime = 0;
int enemyMoveDelay = 800;  // milliseconds between moves

// Game state
int score = 0;
int wave = 1;
int highScore = 0;
int lastBonusScore = 0;  // Track when last bonus life was awarded
unsigned long bonusLifeDisplayUntil = 0;  // Show bonus life message
unsigned long lastShootTime = 0;
unsigned long lastEnemyShootTime = 0;
bool gameOver = false;
bool godMode = false;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

float randomFloat(float min, float max) {
  return min + (float)random(10000) / 10000.0f * (max - min);
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

void drawBlock(int x, int y, uint16_t color) {
  M5.Lcd.fillRect(x, y, BLOCK_SIZE, BLOCK_SIZE, color);
  M5.Lcd.drawRect(x, y, BLOCK_SIZE, BLOCK_SIZE, 0x2104);
}

void drawPlayer() {
  if (!player.alive) return;
  
  bool invincible = (millis() < player.invincibleUntil);
  if (invincible && (millis() / 200) % 2 == 0) return;
  
  // Draw T-piece pointing up
  int cx = (int)player.x;
  int cy = (int)player.y;
  
  // T shape: stem at top, cross at bottom
  drawBlock(cx, cy - BLOCK_SIZE, COLOR_PLAYER);  // Top
  drawBlock(cx - BLOCK_SIZE, cy, COLOR_PLAYER);  // Left
  drawBlock(cx, cy, COLOR_PLAYER);               // Center
  drawBlock(cx + BLOCK_SIZE, cy, COLOR_PLAYER);  // Right
}

void drawBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      M5.Lcd.fillRect((int)bullets[i].x - 1, (int)bullets[i].y - 2, 3, 4, COLOR_BULLET);
    }
  }
  
  // Draw enemy bullets (red)
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (enemyBullets[i].active) {
      M5.Lcd.fillRect((int)enemyBullets[i].x - 1, (int)enemyBullets[i].y - 2, 3, 4, 0xF800);
    }
  }
}

void drawEnemy(Enemy* enemy) {
  if (!enemy->active) return;
  
  // Calculate screen position from grid position
  int startX = 10;
  int startY = PLAY_AREA_TOP + 10;
  int spacingX = 12;
  int spacingY = 12;
  
  int screenX = startX + (int)(enemy->gridX * spacingX + enemyGroupX + enemy->offsetX);
  int screenY = startY + (int)(enemy->gridY * spacingY + enemyGroupY + enemy->offsetY);
  
  uint16_t color = ENEMY_COLORS[enemy->type];
  
  // Draw single block (each enemy is now one block)
  drawBlock(screenX, screenY, color);
}

void drawEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    drawEnemy(&enemies[i]);
  }
}

void drawParticles() {
  unsigned long now = millis();
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active && now < particles[i].endTime) {
      M5.Lcd.fillRect((int)particles[i].x, (int)particles[i].y, 2, 2, particles[i].color);
    }
  }
}

void spawnParticles(float x, float y, uint16_t color) {
  // Spawn 4-6 particles flying outward
  int count = random(4, 7);
  for (int i = 0; i < count; i++) {
    // Find inactive particle
    for (int j = 0; j < MAX_PARTICLES; j++) {
      if (!particles[j].active || millis() >= particles[j].endTime) {
        float angle = random(360) * PI / 180.0f;
        float speed = randomFloat(1.5f, 3.5f);
        particles[j].x = x;
        particles[j].y = y;
        particles[j].vx = cos(angle) * speed;
        particles[j].vy = sin(angle) * speed;
        particles[j].color = color;
        particles[j].endTime = millis() + random(300, 600);
        particles[j].active = true;
        break;
      }
    }
  }
}

void drawHUD() {
  M5.Lcd.fillRect(0, 0, SCREEN_WIDTH, PLAY_AREA_TOP, COLOR_BG);
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.printf("W%d", wave);
  
  // Score in middle
  M5.Lcd.setCursor(40, 2);
  M5.Lcd.printf("S:%d", score);
  
  if (godMode) {
    M5.Lcd.fillRect(0, PLAY_AREA_TOP - 3, SCREEN_WIDTH, 3, 0xFFE0);
  }
  
  // Lives at top right
  for (int i = 0; i < player.lives; i++) {
    M5.Lcd.fillRect(SCREEN_WIDTH - 8 - i * 8, 2, 6, 6, 0xF800);
  }
  
  // Show enemy bullet indicator when they start shooting
  if (wave >= 2) {
    M5.Lcd.setCursor(2, 12);
    M5.Lcd.setTextColor(0xF800);
    M5.Lcd.print("!");
  }
  
  // Show bonus life message
  if (millis() < bonusLifeDisplayUntil) {
    M5.Lcd.setCursor(40, 12);
    M5.Lcd.setTextColor(0x07E0); // Green
    M5.Lcd.print("BONUS LIFE!");
  }
}

// ============================================================================
// GAME LOGIC
// ============================================================================

void fire() {
  if (millis() - lastShootTime < 400) return;
  
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = player.x;
      bullets[i].y = player.y - BLOCK_SIZE;
      bullets[i].vy = -3.5f;
      bullets[i].active = true;
      lastShootTime = millis();
      break;
    }
  }
}

void initEnemies() {
  int index = 0;
  for (int r = 0; r < ENEMY_ROWS; r++) {
    for (int c = 0; c < ENEMY_COLS; c++) {
      enemies[index].gridX = c;
      enemies[index].gridY = r;
      enemies[index].offsetX = random(-2, 3);
      enemies[index].offsetY = random(-2, 3);
      enemies[index].type = random(6);
      enemies[index].active = true;
      index++;
    }
  }
  
  enemyDirection = 1;
  enemyGroupX = 0;
  enemyGroupY = 0;
  enemySpeed = 0.3f + (wave * 0.1f);
  enemyMoveDelay = max(100, 800 - (wave * 50));
}

void initPlayer() {
  player.x = SCREEN_WIDTH / 2;
  player.y = PLAY_AREA_BOTTOM - 10;
  player.lives = 3;
  player.alive = true;
  player.invincibleUntil = 0;
}

void handleInput() {
  M5.update();
  
  // Movement
  if (digitalRead(PIN_LEFT) == LOW && player.x > 15) {
    player.x -= 2.0f;
  }
  if (digitalRead(PIN_RIGHT) == LOW && player.x < SCREEN_WIDTH - 15) {
    player.x += 2.0f;
  }
  
  // Fire - Center or Button A
  if (digitalRead(PIN_CENTER) == LOW || M5.BtnA.isPressed()) {
    fire();
  }
  
  // God mode toggle - Button B
  if (M5.BtnB.wasPressed()) {
    godMode = !godMode;
  }
}

void enemyFire() {
  // Only shoot in later waves, or rarely in wave 1
  float chance = (wave == 1) ? 0.005f : 0.015f + (wave * 0.005f);
  
  if (randomFloat(0, 1) < chance) {
    // Pick a random active enemy to shoot
    int activeIndices[MAX_ENEMIES];
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active) {
        activeIndices[count++] = i;
      }
    }
    
    if (count > 0) {
      int idx = activeIndices[random(count)];
      // Spawn enemy bullet
      for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemyBullets[i].active) {
          int startX = 10;
          int startY = PLAY_AREA_TOP + 10;
          int spacingX = 12;
          int spacingY = 12;
          enemyBullets[i].x = startX + (int)(enemies[idx].gridX * spacingX + enemyGroupX + enemies[idx].offsetX);
          enemyBullets[i].y = startY + (int)(enemies[idx].gridY * spacingY + enemyGroupY + enemies[idx].offsetY);
          enemyBullets[i].vy = 2.0f + (wave * 0.2f);
          enemyBullets[i].active = true;
          break;
        }
      }
    }
  }
}

void updateGame() {
  // Update bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].y += bullets[i].vy;
      if (bullets[i].y < PLAY_AREA_TOP) bullets[i].active = false;
      
      // Check collision with enemies
      for (int j = 0; j < MAX_ENEMIES; j++) {
        if (enemies[j].active) {
          int startX = 10;
          int startY = PLAY_AREA_TOP + 10;
          int spacingX = 12;
          int spacingY = 12;
          float ex = startX + enemies[j].gridX * spacingX + enemyGroupX + enemies[j].offsetX;
          float ey = startY + enemies[j].gridY * spacingY + enemyGroupY + enemies[j].offsetY;
          
          if (abs(bullets[i].x - ex) < 6 && abs(bullets[i].y - ey) < 6) {
            bullets[i].active = false;
            enemies[j].active = false;
            score += 10;
            spawnParticles(ex, ey, ENEMY_COLORS[enemies[j].type]);
            break;
          }
        }
      }
    }
  }
  
  // Update enemy bullets
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (enemyBullets[i].active) {
      enemyBullets[i].y += enemyBullets[i].vy;
      if (enemyBullets[i].y > PLAY_AREA_BOTTOM) enemyBullets[i].active = false;
      
      // Collision with player
      if (player.alive && millis() > player.invincibleUntil && !godMode) {
        if (abs(enemyBullets[i].x - player.x) < 10 && abs(enemyBullets[i].y - player.y) < 10) {
          enemyBullets[i].active = false;
          player.alive = false;
          player.lives--;
          spawnParticles(player.x, player.y, COLOR_PLAYER);
          if (player.lives <= 0) gameOver = true;
          else player.invincibleUntil = millis() + 2000;
        }
      }
    }
  }
  
  // Update enemy formation
  if (millis() - lastEnemyMoveTime > enemyMoveDelay) {
    enemyGroupX += enemyDirection * enemySpeed * 10.0f;
    
    // Check bounds for reversal
    bool reverse = false;
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active) {
        int startX = 10;
        float ex = startX + enemies[i].gridX * 12 + enemyGroupX;
        if (ex < 10 || ex > SCREEN_WIDTH - 10) {
          reverse = true;
          break;
        }
      }
    }
    
    if (reverse) {
      enemyDirection *= -1;
      enemyGroupY += 5.0f;  // Drop down
      enemyGroupX += enemyDirection * enemySpeed * 10.0f; // Re-apply move
    }
    
    lastEnemyMoveTime = millis();
    
    // Check if enemies reached bottom
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active) {
        int startY = PLAY_AREA_TOP + 10;
        float ey = startY + enemies[i].gridY * 12 + enemyGroupY;
        if (ey > player.y - 10) {
          gameOver = true;
          break;
        }
      }
    }
  }
  
  // Enemy shooting
  enemyFire();
  
  // Check for wave complete
  bool anyActive = false;
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      anyActive = true;
      break;
    }
  }
  
  if (!anyActive) {
    wave++;
    initEnemies();
  }
  
  // Update particles
  // (Handling logic is in drawParticles)
  
  // Check for bonus life
  if (score >= lastBonusScore + 1000) {
    player.lives++;
    lastBonusScore += 1000;
    bonusLifeDisplayUntil = millis() + 2000; // Show for 2s
  }
}

void resetGame() {
  score = 0;
  wave = 1;
  lastBonusScore = 0;
  gameOver = false;
  
  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = false;
  for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
  
  initPlayer();
  initEnemies();
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
  if (!gameOver) {
    handleInput();
    updateGame();
    
    // Respawn player if dead
    if (!player.alive && !gameOver && millis() > player.invincibleUntil - 1000) {
      player.alive = true;
      player.x = SCREEN_WIDTH / 2;
    }
    
    // Draw
    M5.Lcd.fillScreen(COLOR_BG);
    drawPlayer();
    drawEnemies();
    drawBullets();
    drawParticles();
    drawHUD();
  } else {
    // Game Over
    if (score > highScore) highScore = score;
    
    M5.Lcd.fillScreen(COLOR_BG);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(0xF800);
    M5.Lcd.setCursor(15, 100);
    M5.Lcd.print("GAME OVER");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.setCursor(30, 130);
    M5.Lcd.printf("Score: %d", score);
    M5.Lcd.setCursor(20, 150);
    M5.Lcd.print("Press M5 to Restart");
    
    M5.update();
    if (M5.BtnA.wasPressed() || digitalRead(PIN_CENTER) == LOW) {
      resetGame();
    }
  }
  
  delay(10);
}

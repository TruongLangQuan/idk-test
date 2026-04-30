/*
 * TETRIMINO SPACE INVADERS
 * M5StickC Plus2 with Mini JoyC Hat
 * 
 * Player: T-piece ship at bottom
 * Enemies: Small Tetrimino pieces in formation
 * 
 * Controls:
 * - Joystick Left/Right: Move ship
 * - Joystick Button: Shoot
 * - BtnB: Toggle God Mode
 */

#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"

// Display configuration
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define PLAY_AREA_TOP 20
#define PLAY_AREA_BOTTOM 220
#define BLOCK_SIZE 6

// Game constants
#define MAX_BULLETS 3
#define MAX_ENEMY_BULLETS 10
#define MAX_ENEMIES 150  // More blocks now (9x4x4 = 144)
#define MAX_PARTICLES 20
#define GRID_COLS 11
#define GRID_ROWS 5
#define ENEMY_COLS 9
#define ENEMY_ROWS 4

// JoyC
UNIT_JOYC Joystick;
#define JoyC_ADDR 0x54

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
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(0x07E0);
    M5.Lcd.setCursor(35, 120);
    M5.Lcd.print("BONUS LIFE!");
  }
}

// ============================================================================
// GAME LOGIC
// ============================================================================

void initPlayer() {
  player.x = SCREEN_WIDTH / 2;
  player.y = PLAY_AREA_BOTTOM - 15;
  player.lives = 3;
  player.alive = true;
  player.invincibleUntil = millis() + 2000;
}

void respawnPlayer() {
  player.x = SCREEN_WIDTH / 2;
  player.y = PLAY_AREA_BOTTOM - 15;
  player.alive = true;
  player.invincibleUntil = millis() + 2000;
}

void shootBullet() {
  unsigned long now = millis();
  if (now - lastShootTime < 300) return;
  
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = player.x;
      bullets[i].y = player.y - BLOCK_SIZE;
      bullets[i].vy = -4.0f;  // Shoot upward
      bullets[i].active = true;
      lastShootTime = now;
      return;
    }
  }
}

void spawnEnemyFormation() {
  int enemyIdx = 0;
  
  // Each "position" now spawns 4 individual blocks in a small tetromino pattern
  for (int row = 0; row < ENEMY_ROWS && enemyIdx < MAX_ENEMIES - 4; row++) {
    int pieceType = row % 6;  // Vary by row
    for (int col = 0; col < ENEMY_COLS && enemyIdx < MAX_ENEMIES - 4; col++) {
      // Spawn 3-4 blocks in a small pattern for this grid position
      int numBlocks = (pieceType == 1) ? 4 : 3;  // O-piece has 4, others 3
      
      for (int b = 0; b < numBlocks; b++) {
        if (enemyIdx >= MAX_ENEMIES) break;
        
        enemies[enemyIdx].gridX = col;
        enemies[enemyIdx].gridY = row;
        enemies[enemyIdx].type = pieceType;
        
        // Random offset within the cell to create clustered look
        enemies[enemyIdx].offsetX = random(-2, 3);
        enemies[enemyIdx].offsetY = random(-2, 3);
        
        enemies[enemyIdx].active = true;
        enemyIdx++;
      }
    }
  }
  
  // Reset formation position
  enemyGroupX = 0;
  enemyGroupY = 0;
  enemyDirection = 1;
  
  // Increase speed with each wave
  enemyMoveDelay = 800 - (wave - 1) * 50;
  if (enemyMoveDelay < 200) enemyMoveDelay = 200;
}

void updatePlayer() {
  if (!player.alive) return;
  
  int joyX = Joystick.getADCValue(0);
  bool joyBtn = (Joystick.getButtonStatus() == 0);
  
  // Move left/right
  float moveSpeed = 2.5f;
  if (joyX < 1350) {
    player.x -= moveSpeed;
  } else if (joyX > 2950) {
    player.x += moveSpeed;
  }
  
  // Keep player on screen
  if (player.x < BLOCK_SIZE * 2) player.x = BLOCK_SIZE * 2;
  if (player.x > SCREEN_WIDTH - BLOCK_SIZE * 2) player.x = SCREEN_WIDTH - BLOCK_SIZE * 2;
  
  // Shoot
  if (joyBtn) {
    shootBullet();
  }
  
  // God mode toggle
  static bool lastBtnB = false;
  if (M5.BtnB.isPressed() && !lastBtnB) {
    godMode = !godMode;
    delay(200);
  }
  lastBtnB = M5.BtnB.isPressed();
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;
    
    bullets[i].y += bullets[i].vy;
    
    // Deactivate if off screen
    if (bullets[i].y < PLAY_AREA_TOP) {
      bullets[i].active = false;
    }
  }
  
  // Update enemy bullets
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (!enemyBullets[i].active) continue;
    
    enemyBullets[i].y += enemyBullets[i].vy;
    
    // Deactivate if off screen
    if (enemyBullets[i].y > PLAY_AREA_BOTTOM) {
      enemyBullets[i].active = false;
    }
  }
}

void updateParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (!particles[i].active) continue;
    
    particles[i].x += particles[i].vx;
    particles[i].y += particles[i].vy;
  }
}

void updateEnemies() {
  unsigned long now = millis();
  
  // Move enemy group
  if (now - lastEnemyMoveTime > enemyMoveDelay) {
    lastEnemyMoveTime = now;
    
    // Move horizontally
    enemyGroupX += enemyDirection * 6;
    
    // Check boundaries - find leftmost and rightmost active enemies
    int minX = 999, maxX = -999;
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active) {
        if (enemies[i].gridX < minX) minX = enemies[i].gridX;
        if (enemies[i].gridX > maxX) maxX = enemies[i].gridX;
      }
    }
    
    int startX = 10;
    int spacingX = 12;
    int leftEdge = startX + (int)(minX * spacingX + enemyGroupX);
    int rightEdge = startX + (int)(maxX * spacingX + enemyGroupX) + BLOCK_SIZE * 2;
    
    // Hit boundary - move down and reverse
    if (leftEdge < 5 || rightEdge > SCREEN_WIDTH - 5) {
      enemyDirection *= -1;
      enemyGroupY += 8;  // Move down
    }
  }
  
  // Enemy shooting (starts at wave 2)
  if (wave >= 2 && now - lastEnemyShootTime > 2000) {
    // Shooting gets more frequent with higher waves
    int shootChance = wave >= 5 ? 80 : (wave >= 3 ? 50 : 30);
    
    if (random(100) < shootChance) {
      // Pick a random active enemy in the front row
      int attempts = 0;
      while (attempts++ < 20) {
        int idx = random(MAX_ENEMIES);
        if (enemies[idx].active) {
          // Check if this enemy is in the frontmost row for its column
          bool isFront = true;
          for (int j = 0; j < MAX_ENEMIES; j++) {
            if (enemies[j].active && 
                enemies[j].gridX == enemies[idx].gridX && 
                enemies[j].gridY > enemies[idx].gridY) {
              isFront = false;
              break;
            }
          }
          
          if (isFront) {
            // Shoot!
            for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
              if (!enemyBullets[i].active) {
                int startX = 10;
                int startY = PLAY_AREA_TOP + 10;
                int spacingX = 12;
                int spacingY = 12;
                
                enemyBullets[i].x = startX + enemies[idx].gridX * spacingX + enemyGroupX + enemies[idx].offsetX;
                enemyBullets[i].y = startY + enemies[idx].gridY * spacingY + enemyGroupY + enemies[idx].offsetY + BLOCK_SIZE;
                enemyBullets[i].vy = 3.0f;  // Downward
                enemyBullets[i].active = true;
                lastEnemyShootTime = now;
                break;
              }
            }
            break;
          }
        }
      }
    }
  }
}

void checkCollisions() {
  // Bullet vs Enemy
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;
    
    for (int j = 0; j < MAX_ENEMIES; j++) {
      if (!enemies[j].active) continue;
      
      // Calculate enemy screen position (with offset now)
      int startX = 10;
      int startY = PLAY_AREA_TOP + 10;
      int spacingX = 12;
      int spacingY = 12;
      
      int enemyX = startX + (int)(enemies[j].gridX * spacingX + enemyGroupX + enemies[j].offsetX);
      int enemyY = startY + (int)(enemies[j].gridY * spacingY + enemyGroupY + enemies[j].offsetY);
      
      // Collision with single block
      float dx = bullets[i].x - enemyX;
      float dy = bullets[i].y - enemyY;
      float dist = sqrt(dx*dx + dy*dy);
      
      if (dist < BLOCK_SIZE) {
        // Spawn particles for explosion
        spawnParticles(enemyX, enemyY, ENEMY_COLORS[enemies[j].type]);
        
        enemies[j].active = false;
        bullets[i].active = false;
        score += 5;  // Less points per block now
        
        // Bonus life every 1000 points
        int currentThousands = score / 1000;
        int lastThousands = lastBonusScore / 1000;
        
        if (currentThousands > lastThousands) {
          player.lives++;
          if (player.lives > 5) player.lives = 5;  // Cap at 5 lives
          bonusLifeDisplayUntil = millis() + 2000;  // Show message for 2 seconds
          lastBonusScore = score;
        }
        
        break;
      }
    }
  }
  
  // Enemy bullet vs Player
  if (!godMode && millis() > player.invincibleUntil) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
      if (!enemyBullets[i].active) continue;
      
      float dx = enemyBullets[i].x - player.x;
      float dy = enemyBullets[i].y - player.y;
      float dist = sqrt(dx*dx + dy*dy);
      
      if (dist < BLOCK_SIZE * 1.5f) {
        enemyBullets[i].active = false;
        
        // Player death explosion
        spawnParticles(player.x, player.y, COLOR_PLAYER);
        spawnParticles(player.x - BLOCK_SIZE, player.y, COLOR_PLAYER);
        spawnParticles(player.x + BLOCK_SIZE, player.y, COLOR_PLAYER);
        
        player.lives--;
        player.alive = false;  // Hide player during respawn
        player.invincibleUntil = millis() + 2500;  // 2.5s to see explosion + respawn
        
        if (player.lives <= 0) {
          gameOver = true;
        }
        return;
      }
    }
  }
  
  // Check if enemies reached bottom
  if (!godMode) {
    int startY = PLAY_AREA_TOP + 10;
    int spacingY = 12;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (!enemies[i].active) continue;
      
      int enemyY = startY + (int)(enemies[i].gridY * spacingY + enemyGroupY);
      
      if (enemyY + BLOCK_SIZE * 2 >= PLAY_AREA_BOTTOM - 20) {
        // Enemy reached bottom - game over
        player.lives = 0;
        player.alive = false;
        gameOver = true;
        return;
      }
    }
    
    // Check if enemy touches player (simplified)
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (!enemies[i].active) continue;
      
      int enemyX = 10 + (int)(enemies[i].gridX * 12 + enemyGroupX + enemies[i].offsetX);
      int enemyY = PLAY_AREA_TOP + 10 + (int)(enemies[i].gridY * 12 + enemyGroupY + enemies[i].offsetY);
      
      float dx = enemyX - player.x;
      float dy = enemyY - player.y;
      float dist = sqrt(dx*dx + dy*dy);
      
      if (dist < BLOCK_SIZE * 1.5f && millis() > player.invincibleUntil) {
        // Player death explosion
        spawnParticles(player.x, player.y, COLOR_PLAYER);
        spawnParticles(player.x - BLOCK_SIZE, player.y, COLOR_PLAYER);
        spawnParticles(player.x + BLOCK_SIZE, player.y, COLOR_PLAYER);
        
        player.lives--;
        player.alive = false;
        player.invincibleUntil = millis() + 2500;
        
        if (player.lives <= 0) {
          gameOver = true;
        }
        return;
      }
    }
  }
}

void checkWaveComplete() {
  // Count active enemies
  int activeCount = 0;
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) activeCount++;
  }
  
  if (activeCount == 0) {
    // Wave complete!
    wave++;
    score += 100;
    
    M5.Lcd.fillScreen(COLOR_BG);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(0x07E0);
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.printf("WAVE %d", wave);
    delay(1500);
    
    spawnEnemyFormation();
  }
}

void showGameOver() {
  // Update high score
  if (score > highScore) {
    highScore = score;
  }
  
  M5.Lcd.fillScreen(COLOR_BG);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(0xF800);
  M5.Lcd.setCursor(15, 80);
  M5.Lcd.print("GAME OVER");
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(25, 110);
  M5.Lcd.printf("Wave: %d", wave);
  M5.Lcd.setCursor(25, 125);
  M5.Lcd.printf("Score: %d", score);
  
  if (score == highScore && score > 0) {
    M5.Lcd.setTextColor(0x07E0);
    M5.Lcd.setCursor(25, 140);
    M5.Lcd.print("NEW HIGH!");
  } else if (highScore > 0) {
    M5.Lcd.setTextColor(0x7BEF);
    M5.Lcd.setCursor(25, 140);
    M5.Lcd.printf("High: %d", highScore);
  }
  
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(15, 160);
  M5.Lcd.print("Press BtnA");
  M5.Lcd.setCursor(15, 175);
  M5.Lcd.print("to restart");
  
  while (!M5.BtnA.isPressed()) {
    M5.update();
    delay(50);
  }
  
  delay(200);
}

void resetGame() {
  score = 0;
  wave = 1;
  gameOver = false;
  
  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = false;
  for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
  for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
  
  initPlayer();
  spawnEnemyFormation();
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(COLOR_BG);
  M5.Lcd.setTextSize(1);
  
  Serial.begin(115200);
  Serial.println("Tetrimino Space Invaders Starting...");
  
  // Init JoyC
  Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL);
  
  // Title screen
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(COLOR_PLAYER);
  M5.Lcd.setCursor(10, 60);
  M5.Lcd.print("TETRIMINO");
  M5.Lcd.setCursor(25, 85);
  M5.Lcd.print("SPACE");
  M5.Lcd.setCursor(15, 110);
  M5.Lcd.print("INVADERS");
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(0xFFFF);
  M5.Lcd.setCursor(15, 150);
  M5.Lcd.print("Press BtnA");
  
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
  
  // Check if player should respawn after death explosion
  if (!player.alive && player.lives > 0 && millis() > player.invincibleUntil) {
    respawnPlayer();
  }
  
  // Update
  updatePlayer();
  updateBullets();
  updateEnemies();
  updateParticles();
  checkCollisions();
  checkWaveComplete();
  
  // Draw
  M5.Lcd.fillRect(0, PLAY_AREA_TOP, SCREEN_WIDTH, PLAY_AREA_BOTTOM - PLAY_AREA_TOP, COLOR_BG);
  
  // Draw bottom border/floor
  M5.Lcd.fillRect(0, PLAY_AREA_BOTTOM, SCREEN_WIDTH, SCREEN_HEIGHT - PLAY_AREA_BOTTOM, 0x2104);  // Dark grey border
  M5.Lcd.drawLine(0, PLAY_AREA_BOTTOM, SCREEN_WIDTH, PLAY_AREA_BOTTOM, 0x7BEF);  // Light grey line
  
  drawPlayer();
  drawBullets();
  drawEnemies();
  drawParticles();
  drawHUD();
  
  delay(20);
}

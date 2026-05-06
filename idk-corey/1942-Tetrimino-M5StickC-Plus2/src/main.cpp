/*
 * 1942 CLONE - Tetrimino Edition
 * M5StickC Plus2 with Mini JoyC Hat
 * 
 * Player: T-piece airplane at bottom
 * Enemies: Tetrimino planes attacking from above
 * Gameplay: Vertical scrolling shooter with wave-based enemy formations
 * 
 * Controls:
 * - Joystick: Move in 8 directions
 * - Button: Fire bullets upward
 */

#include <M5StickCPlus2.h>

#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

// Display configuration
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define BLOCK_SIZE 4

// Game constants
#define MAX_BULLETS 10
#define MAX_ENEMY_BULLETS 20
#define MAX_ENEMIES 30
#define MAX_PARTICLES 30
#define MAX_EXPLOSIONS 10
#define MAX_POWERUPS 1
#define MAX_WING_BULLETS 18  // 3 volleys of 6 bullets (3 power-ups x 2 wings)

// JoyC removed
#define POS_X 0
#define POS_Y 1

// Colors (Tetris themed)
#define COLOR_PLAYER 0xF81F    // Magenta (T-piece)
#define COLOR_BULLET 0xFFFF    // White
#define COLOR_ENEMY_BULLET 0xFD20  // Orange
#define COLOR_I 0x07FF         // Cyan
#define COLOR_O 0xFFE0         // Yellow
#define COLOR_L 0xFD20         // Orange
#define COLOR_J 0x001F         // Blue
#define COLOR_S 0x07E0         // Green
#define COLOR_Z 0xF800         // Red
#define COLOR_BG 0x0000        // Black
#define COLOR_GRAY 0x7BEF      // Gray

const uint16_t ENEMY_COLORS[6] = {COLOR_I, COLOR_O, COLOR_L, COLOR_J, COLOR_S, COLOR_Z};

// Enemy tetrimino shapes [type][block][x/y]
const int8_t ENEMY_SHAPES[6][4][2] = {
  {{0,0},{1,0},{2,0},{-1,-1}},   // I-piece (line)
  {{0,0},{1,0},{0,1},{1,1}},     // O-piece (square)
  {{0,0},{1,0},{2,0},{0,1}},     // L-piece
  {{0,0},{1,0},{2,0},{2,1}},     // J-piece
  {{1,0},{2,0},{0,1},{1,1}},     // S-piece
  {{0,0},{1,0},{1,1},{2,1}}      // Z-piece
};

// TETRIS Letter patterns (5x7 grid for Wave 6)
static const uint8_t LETTER_T[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t LETTER_E[7] = {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
static const uint8_t LETTER_T2[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
static const uint8_t LETTER_R[7] = {0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001};
static const uint8_t LETTER_I[7] = {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111};
static const uint8_t LETTER_S[7] = {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};

static const uint8_t* LETTERS[6] = {LETTER_T, LETTER_E, LETTER_T2, LETTER_R, LETTER_I, LETTER_S};

static const uint16_t LETTER_COLORS[6] = {
  0x07FF,  // T - Cyan
  0xFFE0,  // E - Yellow
  0x07E0,  // T - Green
  0xF800,  // R - Red
  0xFD20,  // I - Orange
  0xF81F   // S - Magenta
};

// Strange symbol patterns (5x5 grid for Wave 6)
static const uint8_t SHAPE_PLUS[5] = {0b00100, 0b00100, 0b11111, 0b00100, 0b00100};
static const uint8_t SHAPE_DIAMOND[5] = {0b00100, 0b01010, 0b10001, 0b01010, 0b00100};
static const uint8_t SHAPE_ARROW[5] = {0b00100, 0b01110, 0b11111, 0b00100, 0b00100};
static const uint8_t SHAPE_HEART[5] = {0b01010, 0b11111, 0b11111, 0b01110, 0b00100};
static const uint8_t SHAPE_ZIG[5] = {0b11000, 0b01100, 0b00110, 0b00011, 0b00001};
static const uint8_t SHAPE_CROSS[5] = {0b10001, 0b01010, 0b00100, 0b01010, 0b10001};
static const uint8_t SHAPE_STAIRS[5] = {0b00001, 0b00011, 0b00111, 0b01111, 0b11111};
static const uint8_t SHAPE_SPIRAL[5] = {0b11110, 0b00010, 0b11010, 0b10000, 0b11110};

static const uint8_t* SYMBOL_SHAPES[8] = {
  SHAPE_PLUS, SHAPE_DIAMOND, SHAPE_ARROW, SHAPE_HEART,
  SHAPE_ZIG, SHAPE_CROSS, SHAPE_STAIRS, SHAPE_SPIRAL
};

static const uint16_t SYMBOL_COLORS[8] = {
  0xFFE0,  // Yellow
  0x07FF,  // Cyan
  0xF81F,  // Magenta
  0xF800,  // Red
  0x07E0,  // Green
  0xFD20,  // Orange
  0x001F,  // Blue
  0xFFFF   // White
};

// Wave 6 tracking
static int nextLetterIndex = 0;

// Player T-piece shape
const int8_t PLAYER_SHAPE[4][2] = {{1,0},{0,1},{1,1},{2,1}};

// Game objects
struct Bullet {
  float x, y;
  bool active;
};

struct EnemyBullet {
  float x, y;
  bool active;
};

struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  unsigned long endTime;
  bool active;
};

struct Explosion {
  float x, y;
  uint8_t frame;
  unsigned long nextFrameTime;
  bool active;
};

struct Enemy {
  float x, y;
  float vx, vy;
  int type;
  int formationType;  // 0=straight, 1=arc, 2=swoop
  float pathParam;
  bool active;
  unsigned long lastShotTime;
  bool isBoss;
  int bossHealth;
  float bossSize;  // Boss size multiplier (scales with wave)
  bool isLarge;  // Double-sized enemy
  bool isLetter;  // TETRIS letter (Wave 6)
  bool isSymbol;  // Strange symbol (Wave 6)
  int letterIndex;  // Which letter/symbol (0-5 for letters, 0-7 for symbols)
};

struct Powerup {
  float x, y;
  float vy;
  bool active;
};

struct Player {
  float x, y;
  int lives;
  bool alive;
  unsigned long invincibleUntil;
  unsigned long lastShotTime;
  int powerLevel;  // 0-3 power-ups
} player;

Bullet bullets[MAX_BULLETS];
Bullet wingBullets[MAX_WING_BULLETS];  // Bullets from wing guns
EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];
Enemy enemies[MAX_ENEMIES];
Particle particles[MAX_PARTICLES];
Explosion explosions[MAX_EXPLOSIONS];
Powerup powerups[MAX_POWERUPS];

// Game state
int score = 0;
int wave = 1;
unsigned long lastSpawnTime = 0;
unsigned long spawnInterval = 1200;  // Faster spawning for 60 second waves
int enemiesThisWave = 0;
int enemiesPerWave = 5;
float scrollY = 0;
bool godMode = false;
unsigned long waveStartTime = 0;
bool isBossWave = false;
bool bossActive = false;
int bossMaxHealth = 20;
unsigned long lastPowerupTime = 0;
unsigned long nextPowerupDelay = 30000;  // First powerup in 30 seconds

// Statistics tracking
int shotsFired = 0;
int enemiesDestroyed = 0;

// Game states
enum GameState { TITLE_SCREEN, PLAYING, GAME_OVER };
GameState gameState = TITLE_SCREEN;

// Input
int joyX = 0, joyY = 0;
bool btnPressed = false, btnLastPressed = false;
bool btnBPressed = false, btnBLastPressed = false;
bool btnAPressed = false, btnALastPressed = false;
bool autoFire = false;  // Auto-fire toggle state

void initInput() {
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_CENTER, INPUT_PULLUP);
}

void updateInput() {
  // Read 5-way tactile switch (Active-LOW)
  joyX = 0;
  joyY = 0;
  if (digitalRead(PIN_UP) == LOW) joyY = 100;
  if (digitalRead(PIN_DOWN) == LOW) joyY = -100;
  if (digitalRead(PIN_LEFT) == LOW) joyX = -100;
  if (digitalRead(PIN_RIGHT) == LOW) joyX = 100;
  
  btnLastPressed = btnPressed;
  btnPressed = (digitalRead(PIN_CENTER) == LOW); 
  btnBLastPressed = btnBPressed;
  btnBPressed = (M5.BtnB.isPressed());
  btnALastPressed = btnAPressed;
  btnAPressed = (M5.BtnA.isPressed());
  
  // Toggle god mode (Button B)
  if (btnBPressed && !btnBLastPressed) {
    godMode = !godMode;
  }
  
  // Toggle auto-fire (Button A)
  if (btnAPressed && !btnALastPressed) {
    autoFire = !autoFire;
  }
}

void initGame() {
  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(COLOR_BG);
  
  player.x = SCREEN_WIDTH / 2;
  player.y = SCREEN_HEIGHT - 30;
  player.lives = 3;
  player.alive = true;
  player.invincibleUntil = 0;
  player.lastShotTime = 0;
  player.powerLevel = 0;
  
  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  for (int i = 0; i < MAX_WING_BULLETS; i++) wingBullets[i].active = false;
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = false;
  for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
  for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
  for (int i = 0; i < MAX_EXPLOSIONS; i++) explosions[i].active = false;
  for (int i = 0; i < MAX_POWERUPS; i++) powerups[i].active = false;
  
  score = 0;
  wave = 1;
  enemiesThisWave = 0;
  enemiesPerWave = 5;
  lastSpawnTime = millis();
  spawnInterval = 1200;
  scrollY = 0;
  waveStartTime = millis();
  isBossWave = false;
  bossActive = false;
  lastPowerupTime = millis();
  nextPowerupDelay = random(20000, 45000);  // 20-45 seconds
  
  // Reset statistics
  shotsFired = 0;
  enemiesDestroyed = 0;
}

void spawnParticles(float x, float y, uint16_t color, int count) {
  for (int i = 0; i < count; i++) {
    for (int p = 0; p < MAX_PARTICLES; p++) {
      if (!particles[p].active) {
        particles[p].x = x;
        particles[p].y = y;
        float angle = random(360) * 0.0174533;
        float speed = random(10, 30) / 10.0;
        particles[p].vx = cos(angle) * speed;
        particles[p].vy = sin(angle) * speed;
        particles[p].color = color;
        particles[p].endTime = millis() + 500;
        particles[p].active = true;
        break;
      }
    }
  }
}

void spawnExplosion(float x, float y) {
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    if (!explosions[i].active) {
      explosions[i].x = x;
      explosions[i].y = y;
      explosions[i].frame = 0;
      explosions[i].nextFrameTime = millis() + 50;
      explosions[i].active = true;
      break;
    }
  }
}

void spawnEnemy(int formationType) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].formationType = formationType;
      enemies[i].active = true;
      enemies[i].lastShotTime = 0;
      enemies[i].pathParam = 0;
      enemies[i].isBoss = false;
      enemies[i].bossHealth = 0;
      enemies[i].isLetter = false;
      enemies[i].isSymbol = false;
      
      // Wave 6: Special letter and symbol enemies
      // Wave 7+: Mix of everything
      if (wave == 6 || wave >= 7) {
        int enemyTypeRoll = random(3);  // 0=tetrimino, 1=letter, 2=symbol
        
        if (wave >= 7 && enemyTypeRoll == 0) {
          // Tetrimino enemy
          enemies[i].type = random(6);
          enemies[i].isLarge = (random(100) < 30);  // 30% chance
        } else if (enemyTypeRoll == 1 || (wave == 6 && random(2) == 0)) {
          // Letter enemy
          enemies[i].isLetter = true;
          enemies[i].letterIndex = nextLetterIndex;
          nextLetterIndex = (nextLetterIndex + 1) % 6;
          enemies[i].type = 0;
          enemies[i].isLarge = false;
        } else {
          // Symbol enemy
          enemies[i].isSymbol = true;
          enemies[i].letterIndex = random(8);
          enemies[i].type = 0;
          enemies[i].isLarge = false;
        }
      } else {
        // Regular tetrimino enemies for waves 1-5
        enemies[i].type = random(6);
        // Wave 2+: 30% chance of large enemy
        enemies[i].isLarge = (wave >= 2 && wave <= 5 && random(100) < 30);
      }
      
      // Adjust speed based on wave with cap at 3.0x
      // Wave 1: 1.0x, Wave 2: 1.15x, Wave 3: 1.3x ... Wave 14+: 3.0x (maxed)
      float speedMultiplier = min(1.0 + (wave - 1) * 0.15, 3.0);
      
      // Mix of formations
      if (formationType == 0) {
        // Straight down
        enemies[i].x = random(10, SCREEN_WIDTH - 20);
        enemies[i].y = -15;
        enemies[i].vx = 0;
        enemies[i].vy = 1.5 * speedMultiplier;
      } else if (formationType == 1) {
        // Arc from side
        bool fromLeft = random(2);
        enemies[i].x = fromLeft ? -10 : SCREEN_WIDTH + 10;
        enemies[i].y = random(20, 60);
        enemies[i].vx = (fromLeft ? 1.8 : -1.8) * speedMultiplier;
        enemies[i].vy = 0.6 * speedMultiplier;
      } else if (formationType == 2) {
        // Swoop pattern
        enemies[i].x = random(20, SCREEN_WIDTH - 20);
        enemies[i].y = -15;
        enemies[i].vx = (random(2) ? 1 : -1) * 1.0 * speedMultiplier;
        enemies[i].vy = 1.5 * speedMultiplier;
      }
      
      enemiesThisWave++;
      break;
    }
  }
}

void spawnBoss() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].type = random(6);
      enemies[i].active = true;
      enemies[i].isBoss = true;
      enemies[i].isLarge = false;
      
      // Boss health scales: Wave 1 = 20, Wave 2 = 25, Wave 3 = 30, etc
      enemies[i].bossHealth = 20 + (wave - 1) * 5;
      
      // Boss size scales progressively: Wave 1 = 2x, Wave 5 = 4x, Wave 10 = 6x, max at 8x
      enemies[i].bossSize = min(2.0 + (wave - 1) * 0.5, 8.0);
      
      enemies[i].lastShotTime = 0;
      enemies[i].pathParam = 0;
      
      // Boss starts at top center
      enemies[i].x = SCREEN_WIDTH / 2;
      enemies[i].y = 30;
      
      // Boss speed increases slightly per wave
      enemies[i].vx = 1.0 + (wave - 1) * 0.2;
      enemies[i].vy = 0;
      enemies[i].formationType = 0;
      
      bossActive = true;
      bossMaxHealth = enemies[i].bossHealth;  // Update for display
      break;
    }
  }
}

void spawnPowerup() {
  for (int i = 0; i < MAX_POWERUPS; i++) {
    if (!powerups[i].active) {
      powerups[i].x = random(20, SCREEN_WIDTH - 20);
      powerups[i].y = -10;
      powerups[i].vy = 1.0;
      powerups[i].active = true;
      lastPowerupTime = millis();
      nextPowerupDelay = random(20000, 45000);  // Next one in 20-45 seconds
      break;
    }
  }
}

void updatePowerups() {
  for (int i = 0; i < MAX_POWERUPS; i++) {
    if (powerups[i].active) {
      powerups[i].y += powerups[i].vy;
      
      // Remove if off screen
      if (powerups[i].y > SCREEN_HEIGHT + 10) {
        powerups[i].active = false;
      }
    }
  }
  
  // Check if time to spawn new powerup
  if (millis() - lastPowerupTime > nextPowerupDelay) {
    spawnPowerup();
  }
}

void updateWaveSpawning() {
  unsigned long now = millis();
  unsigned long waveElapsed = now - waveStartTime;
  
  // Check if it's time for a boss wave (every 60 seconds)
  if (!isBossWave && waveElapsed >= 60000) {
    // Clear remaining regular enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active && !enemies[i].isBoss) {
        enemies[i].active = false;
      }
    }
    isBossWave = true;
    spawnBoss();
    return;
  }
  
  // Boss wave - check if boss defeated
  if (isBossWave) {
    if (!bossActive) {
      // Boss defeated! Next wave
      wave++;
      waveStartTime = millis();
      isBossWave = false;
      enemiesThisWave = 0;
      lastSpawnTime = millis();
    }
    return;
  }
  
  // Regular wave - spawn enemies continuously for 60 seconds
  if (waveElapsed < 60000 && now - lastSpawnTime > spawnInterval) {
    // Progressively faster spawn rate (more enemies), capped at 600ms
    unsigned long adjustedInterval = max(600UL, spawnInterval - (wave - 1) * 100);  // Gets faster each wave
    
    if (now - lastSpawnTime > adjustedInterval) {
      // Mix of formation types (like wave 2-3)
      int formationType = random(3);  // 0=straight, 1=arc, 2=swoop
      spawnEnemy(formationType);
      lastSpawnTime = now;
    }
  }
}

void updatePlayer() {
  if (!player.alive) return;
  
  // Movement (8 directions)
  float moveSpeed = 2.5;
  if (joyX < -30) player.x -= moveSpeed;
  if (joyX > 30) player.x += moveSpeed;
  if (joyY < -30) player.y += moveSpeed;  // Swapped: joystick up = move down
  if (joyY > 30) player.y -= moveSpeed;   // Swapped: joystick down = move up
  
  // Boundaries
  player.x = constrain(player.x, 10, SCREEN_WIDTH - 10);
  player.y = constrain(player.y, 20, SCREEN_HEIGHT - 20);
  
  // Shooting (manual button press OR auto-fire enabled)
  if ((btnPressed || autoFire) && millis() - player.lastShotTime > 150) {
    // Main gun
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].x = player.x;
        bullets[i].y = player.y - 8;
        bullets[i].active = true;
        player.lastShotTime = millis();
        shotsFired++;  // Track shot
        break;
      }
    }
    
    // Wing guns (if powered up)
    if (player.powerLevel > 0) {
      int wingsActive = player.powerLevel;
      int bulletSlot = 0;
      
      // Spawn all wing bullets at once
      for (int w = 1; w <= wingsActive; w++) {
        // Left wing bullet
        if (bulletSlot < MAX_WING_BULLETS) {
          // Find next available slot
          for (int i = bulletSlot; i < MAX_WING_BULLETS; i++) {
            if (!wingBullets[i].active) {
              wingBullets[i].x = player.x - (w * 6) + 1;
              wingBullets[i].y = player.y - 8;
              wingBullets[i].active = true;
              bulletSlot = i + 1;
              break;
            }
          }
        }
        
        // Right wing bullet
        if (bulletSlot < MAX_WING_BULLETS) {
          // Find next available slot
          for (int i = bulletSlot; i < MAX_WING_BULLETS; i++) {
            if (!wingBullets[i].active) {
              wingBullets[i].x = player.x + (w * 6) + 11;
              wingBullets[i].y = player.y - 8;
              wingBullets[i].active = true;
              bulletSlot = i + 1;
              break;
            }
          }
        }
      }
    }
  }
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].y -= 4;
      if (bullets[i].y < -5) bullets[i].active = false;
    }
  }
  
  // Update wing bullets
  for (int i = 0; i < MAX_WING_BULLETS; i++) {
    if (wingBullets[i].active) {
      wingBullets[i].y -= 4;
      if (wingBullets[i].y < -5) wingBullets[i].active = false;
    }
  }
}

void updateEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      // Boss behavior - moves side to side at top
      if (enemies[i].isBoss) {
        enemies[i].x += enemies[i].vx;
        enemies[i].pathParam += 0.05;
        
        // Bounce off sides
        if (enemies[i].x < 20 || enemies[i].x > SCREEN_WIDTH - 20) {
          enemies[i].vx *= -1;
        }
        
        // Add slight vertical movement
        enemies[i].y = 30 + sin(enemies[i].pathParam) * 10;
        
        // Boss shoots more frequently (20% faster per wave, capped)
        int bossShotDelay = max(200, 500 - (wave - 1) * 100);  // 500ms -> 200ms min
        if (random(100) < 5 && millis() - enemies[i].lastShotTime > bossShotDelay) {
          for (int b = 0; b < MAX_ENEMY_BULLETS; b++) {
            if (!enemyBullets[b].active) {
              enemyBullets[b].x = enemies[i].x;
              enemyBullets[b].y = enemies[i].y + 20;
              enemyBullets[b].active = true;
              enemies[i].lastShotTime = millis();
              break;
            }
          }
        }
      } else {
        // Regular enemy movement
        enemies[i].x += enemies[i].vx;
        enemies[i].y += enemies[i].vy;
        enemies[i].pathParam += 0.1;
        
        // Add some sine wave to swooping enemies
        if (enemies[i].formationType == 2) {
          enemies[i].x += sin(enemies[i].pathParam) * 0.5;
        }
        
        // Remove if off screen
        if (enemies[i].y > SCREEN_HEIGHT + 20 || 
            enemies[i].x < -30 || enemies[i].x > SCREEN_WIDTH + 30) {
          enemies[i].active = false;
        }
        
        // Regular enemy shooting (5% faster per wave, capped at 400ms)
        int enemyShotDelay = max(400, 1000 - (wave - 1) * 50);  // 1000ms -> 400ms min
        if (random(100) < 2 && millis() - enemies[i].lastShotTime > enemyShotDelay) {
          for (int b = 0; b < MAX_ENEMY_BULLETS; b++) {
            if (!enemyBullets[b].active) {
              enemyBullets[b].x = enemies[i].x;
              enemyBullets[b].y = enemies[i].y + 10;
              enemyBullets[b].active = true;
              enemies[i].lastShotTime = millis();
              break;
            }
          }
        }
      }
    }
  }
}

void updateEnemyBullets() {
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (enemyBullets[i].active) {
      enemyBullets[i].y += 2.5;
      if (enemyBullets[i].y > SCREEN_HEIGHT + 5) {
        enemyBullets[i].active = false;
      }
    }
  }
}

void updateParticles() {
  unsigned long now = millis();
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      if (now > particles[i].endTime) {
        particles[i].active = false;
      }
    }
  }
}

void updateExplosions() {
  unsigned long now = millis();
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    if (explosions[i].active) {
      if (now > explosions[i].nextFrameTime) {
        explosions[i].frame++;
        explosions[i].nextFrameTime = now + 50;
        if (explosions[i].frame > 4) {
          explosions[i].active = false;
        }
      }
    }
  }
}

void checkCollisions() {
  if (!player.alive) return;
  
  // Powerup collection
  for (int p = 0; p < MAX_POWERUPS; p++) {
    if (powerups[p].active) {
      float dx = player.x - powerups[p].x;
      float dy = player.y - powerups[p].y;
      if (abs(dx) < 10 && abs(dy) < 10) {
        powerups[p].active = false;
        if (player.powerLevel < 3) {
          player.powerLevel++;
          spawnParticles(powerups[p].x, powerups[p].y, COLOR_O, 10);
        }
      }
    }
  }
  
  if (millis() < player.invincibleUntil) return;
  
  // Bullet vs Enemy (main gun)
  for (int b = 0; b < MAX_BULLETS; b++) {
    if (bullets[b].active) {
      for (int e = 0; e < MAX_ENEMIES; e++) {
        if (enemies[e].active) {
          // Adjust hitbox based on enemy size
          float hitRange = 8;
          if (enemies[e].isBoss) {
            hitRange = 10 * enemies[e].bossSize;  // Scales with boss size
          } else if (enemies[e].isLarge) {
            hitRange = 12;
          }
          
          float dx = bullets[b].x - enemies[e].x;
          float dy = bullets[b].y - enemies[e].y;
          if (abs(dx) < hitRange && abs(dy) < hitRange) {
            bullets[b].active = false;
            
            if (enemies[e].isBoss) {
              // Boss takes damage
              enemies[e].bossHealth--;
              score += 5;
              spawnParticles(enemies[e].x, enemies[e].y, ENEMY_COLORS[enemies[e].type], 3);
              
              if (enemies[e].bossHealth <= 0) {
                // Boss defeated!
                enemies[e].active = false;
                bossActive = false;
                score += 100;  // Bonus for boss kill
                enemiesDestroyed++;  // Track boss kill
                spawnParticles(enemies[e].x, enemies[e].y, ENEMY_COLORS[enemies[e].type], 20);
                spawnExplosion(enemies[e].x, enemies[e].y);
              }
            } else {
              // Regular or large enemy dies in one hit
              enemies[e].active = false;
              score += enemies[e].isLarge ? 15 : 10;  // Large enemies worth more
              enemiesDestroyed++;  // Track enemy kill
              spawnParticles(enemies[e].x, enemies[e].y, ENEMY_COLORS[enemies[e].type], enemies[e].isLarge ? 8 : 6);
              spawnExplosion(enemies[e].x, enemies[e].y);
            }
            break;
          }
        }
      }
    }
  }
  
  // Wing bullets vs Enemy
  for (int b = 0; b < MAX_WING_BULLETS; b++) {
    if (wingBullets[b].active) {
      for (int e = 0; e < MAX_ENEMIES; e++) {
        if (enemies[e].active) {
          // Adjust hitbox based on enemy size
          float hitRange = 8;
          if (enemies[e].isBoss) {
            hitRange = 10 * enemies[e].bossSize;  // Scales with boss size
          } else if (enemies[e].isLarge) {
            hitRange = 12;
          }
          
          float dx = wingBullets[b].x - enemies[e].x;
          float dy = wingBullets[b].y - enemies[e].y;
          if (abs(dx) < hitRange && abs(dy) < hitRange) {
            wingBullets[b].active = false;
            
            if (enemies[e].isBoss) {
              enemies[e].bossHealth--;
              score += 5;
              spawnParticles(enemies[e].x, enemies[e].y, ENEMY_COLORS[enemies[e].type], 3);
              
              if (enemies[e].bossHealth <= 0) {
                enemies[e].active = false;
                bossActive = false;
                score += 100;
                enemiesDestroyed++;  // Track boss kill
                spawnParticles(enemies[e].x, enemies[e].y, ENEMY_COLORS[enemies[e].type], 20);
                spawnExplosion(enemies[e].x, enemies[e].y);
              }
            } else {
              enemies[e].active = false;
              score += enemies[e].isLarge ? 15 : 10;
              enemiesDestroyed++;  // Track enemy kill
              spawnParticles(enemies[e].x, enemies[e].y, ENEMY_COLORS[enemies[e].type], enemies[e].isLarge ? 8 : 6);
              spawnExplosion(enemies[e].x, enemies[e].y);
            }
            break;
          }
        }
      }
    }
  }
  
  if (godMode) return;
  
  // Enemy collision with player
  for (int e = 0; e < MAX_ENEMIES; e++) {
    if (enemies[e].active) {
      float dx = player.x - enemies[e].x;
      float dy = player.y - enemies[e].y;
      if (abs(dx) < 10 && abs(dy) < 10) {
        enemies[e].active = false;
        
        // Check if player has power-ups to absorb hit
        if (player.powerLevel > 0) {
          player.powerLevel--;  // Lose one power-up
          spawnParticles(player.x, player.y, COLOR_O, 10);
        } else {
          player.lives--;
          spawnParticles(player.x, player.y, COLOR_PLAYER, 10);
          if (player.lives <= 0) {
            player.alive = false;
            
          }
        }
        player.invincibleUntil = millis() + 2000;
        break;
      }
    }
  }
  
  // Enemy bullet vs player
  for (int b = 0; b < MAX_ENEMY_BULLETS; b++) {
    if (enemyBullets[b].active) {
      float dx = player.x - enemyBullets[b].x;
      float dy = player.y - enemyBullets[b].y;
      if (abs(dx) < 8 && abs(dy) < 8) {
        enemyBullets[b].active = false;
        
        // Check if player has power-ups to absorb hit
        if (player.powerLevel > 0) {
          player.powerLevel--;  // Lose one power-up
          spawnParticles(player.x, player.y, COLOR_O, 10);
        } else {
          player.lives--;
          spawnParticles(player.x, player.y, COLOR_PLAYER, 10);
          if (player.lives <= 0) {
            player.alive = false;
            
          }
        }
        player.invincibleUntil = millis() + 2000;
        break;
      }
    }
  }
}

void drawBackground() {
  // Scrolling starfield effect
  scrollY += 1;
  if (scrollY > 20) scrollY = 0;
  
  for (int i = 0; i < 15; i++) {
    int x = (i * 23 + 7) % SCREEN_WIDTH;
    int y = (int)((i * 37 + scrollY)) % SCREEN_HEIGHT;
    M5.Lcd.drawPixel(x, y, COLOR_GRAY);
  }
}

void drawPlayer() {
  if (!player.alive) return;
  
  bool blink = (millis() < player.invincibleUntil) && ((millis() / 100) % 2);
  if (blink) return;
  
  // Main ship
  for (int i = 0; i < 4; i++) {
    int px = player.x + PLAYER_SHAPE[i][0] * BLOCK_SIZE;
    int py = player.y + PLAYER_SHAPE[i][1] * BLOCK_SIZE;
    M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_PLAYER);
  }
  
  // Wing guns (mini T-pieces on sides)
  if (player.powerLevel > 0) {
    for (int w = 1; w <= player.powerLevel; w++) {
      // Left wing
      int leftX = player.x - (w * 6);
      M5.Lcd.fillRect(leftX, player.y, 2, 2, COLOR_PLAYER);
      M5.Lcd.fillRect(leftX - 1, player.y + 2, 4, 2, COLOR_PLAYER);
      
      // Right wing (+6 more = +10 total offset)
      int rightX = player.x + (w * 6) + 10;
      M5.Lcd.fillRect(rightX, player.y, 2, 2, COLOR_PLAYER);
      M5.Lcd.fillRect(rightX - 1, player.y + 2, 4, 2, COLOR_PLAYER);
    }
  }
}

void drawEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      // Letter enemies (Wave 6)
      if (enemies[i].isLetter) {
        const uint8_t* letter = LETTERS[enemies[i].letterIndex];
        uint16_t color = LETTER_COLORS[enemies[i].letterIndex];
        
        for (int row = 0; row < 7; row++) {
          for (int col = 0; col < 5; col++) {
            if (letter[row] & (1 << (4 - col))) {
              int px = enemies[i].x + (col * 2) - 5;
              int py = enemies[i].y + (row * 2) - 7;
              M5.Lcd.fillRect(px, py, 2, 2, color);
            }
          }
        }
        continue;
      }
      
      // Symbol enemies (Wave 6)
      if (enemies[i].isSymbol) {
        const uint8_t* symbol = SYMBOL_SHAPES[enemies[i].letterIndex];
        uint16_t color = SYMBOL_COLORS[enemies[i].letterIndex];
        
        for (int row = 0; row < 5; row++) {
          for (int col = 0; col < 5; col++) {
            if (symbol[row] & (1 << (4 - col))) {
              int px = enemies[i].x + (col * 2) - 5;
              int py = enemies[i].y + (row * 2) - 5;
              M5.Lcd.fillRect(px, py, 2, 2, color);
            }
          }
        }
        continue;
      }
      
      // Regular tetrimino enemies
      uint16_t color = ENEMY_COLORS[enemies[i].type];
      const int8_t (*shape)[2] = ENEMY_SHAPES[enemies[i].type];
      
      // Calculate block size based on enemy type
      int blockSize = BLOCK_SIZE;
      if (enemies[i].isBoss) {
        // Boss size scales with wave: 2x -> 8x
        blockSize = BLOCK_SIZE * enemies[i].bossSize;
      } else if (enemies[i].isLarge) {
        blockSize = BLOCK_SIZE + 2;  // 6 pixels instead of 4
      }
      
      for (int b = 0; b < 4; b++) {
        if (shape[b][0] == -1) break;
        int px = enemies[i].x + shape[b][0] * blockSize;
        int py = enemies[i].y + shape[b][1] * blockSize;
        M5.Lcd.fillRect(px, py, blockSize, blockSize, color);
      }
    }
  }
}

void drawBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      M5.Lcd.fillRect(bullets[i].x - 1, bullets[i].y, 2, 4, COLOR_BULLET);
    }
  }
  
  // Draw wing bullets
  for (int i = 0; i < MAX_WING_BULLETS; i++) {
    if (wingBullets[i].active) {
      M5.Lcd.fillRect(wingBullets[i].x - 1, wingBullets[i].y, 2, 4, COLOR_BULLET);
    }
  }
}

void drawPowerups() {
  for (int i = 0; i < MAX_POWERUPS; i++) {
    if (powerups[i].active) {
      // Draw circle with P
      M5.Lcd.drawCircle(powerups[i].x, powerups[i].y, 5, COLOR_O);
      M5.Lcd.fillCircle(powerups[i].x, powerups[i].y, 4, COLOR_BG);
      M5.Lcd.setTextColor(COLOR_O);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(powerups[i].x - 3, powerups[i].y - 3);
      M5.Lcd.print("P");
    }
  }
}

void drawEnemyBullets() {
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (enemyBullets[i].active) {
      M5.Lcd.fillRect(enemyBullets[i].x - 1, enemyBullets[i].y, 2, 4, COLOR_ENEMY_BULLET);
    }
  }
}

void drawParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      M5.Lcd.drawPixel(particles[i].x, particles[i].y, particles[i].color);
    }
  }
}

void drawExplosions() {
  for (int i = 0; i < MAX_EXPLOSIONS; i++) {
    if (explosions[i].active) {
      int size = (explosions[i].frame + 1) * 3;
      uint16_t color = (explosions[i].frame < 2) ? 0xFFFF : COLOR_ENEMY_BULLET;
      M5.Lcd.drawCircle(explosions[i].x, explosions[i].y, size, color);
    }
  }
}

void drawHUD() {
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(1);
  
  // Score
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.printf("S:%d", score);
  
  // Wave
  M5.Lcd.setCursor(2, 12);
  if (isBossWave) {
    M5.Lcd.print("BOSS!");
  } else {
    unsigned long waveElapsed = millis() - waveStartTime;
    int secondsLeft = (60000 - waveElapsed) / 1000;
    M5.Lcd.printf("W:%d T:%d", wave, secondsLeft);
  }
  
  // Lives
  for (int i = 0; i < player.lives; i++) {
    M5.Lcd.fillRect(SCREEN_WIDTH - 10 - (i * 6), 3, 4, 4, COLOR_PLAYER);
  }
  
  // Status indicators
  M5.Lcd.setCursor(2, 22);
  if (godMode) {
    M5.Lcd.print("GOD");
  }
  if (autoFire) {
    M5.Lcd.setCursor(godMode ? 26 : 2, 22);
    M5.Lcd.print("AUTO");
  }
}

void drawTitleScreen() {
  static unsigned long startTime = millis();
  static unsigned long lastShotTime = 0;
  static int bulletY = -100;  // Bullet position
  static bool bulletActive = false;
  unsigned long elapsed = millis() - startTime;
  
  M5.Lcd.fillScreen(COLOR_BG);
  
  // Animated scrolling background stars
  for (int i = 0; i < 20; i++) {
    int x = (i * 23 + 7) % SCREEN_WIDTH;
    int y = (int)((i * 37 + elapsed / 10)) % SCREEN_HEIGHT;
    M5.Lcd.drawPixel(x, y, COLOR_GRAY);
  }
  
  // Flying Tetrimino formations (animated)
  int offset = (elapsed / 30) % 300 - 50;
  
  // Left formation - I pieces
  for (int i = 0; i < 3; i++) {
    int py = 40 + i * 20 + offset;
    if (py > -20 && py < SCREEN_HEIGHT) {
      for (int b = 0; b < 3; b++) {
        M5.Lcd.fillRect(20 + b * BLOCK_SIZE, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_I);
      }
    }
  }
  
  // Right formation - Z pieces
  for (int i = 0; i < 3; i++) {
    int py = 60 + i * 20 + offset;
    if (py > -20 && py < SCREEN_HEIGHT) {
      M5.Lcd.fillRect(SCREEN_WIDTH - 30, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_Z);
      M5.Lcd.fillRect(SCREEN_WIDTH - 26, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_Z);
      M5.Lcd.fillRect(SCREEN_WIDTH - 26, py + BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, COLOR_Z);
      M5.Lcd.fillRect(SCREEN_WIDTH - 22, py + BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, COLOR_Z);
    }
  }
  
  // Game Title - "1942"
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(35, 80);
  M5.Lcd.print("1942");
  
  // Subtitle - "TETRIMINO"
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(COLOR_I);
  M5.Lcd.setCursor(15, 110);
  M5.Lcd.print("TETRIMINO");
  
  // Edition text
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_GRAY);
  M5.Lcd.setCursor(35, 130);
  M5.Lcd.print("EDITION");
  
  // Animated player plane at bottom
  int planeX = SCREEN_WIDTH / 2 + sin(elapsed / 200.0) * 30;
  for (int i = 0; i < 4; i++) {
    int px = planeX + PLAYER_SHAPE[i][0] * BLOCK_SIZE;
    int py = 180 + PLAYER_SHAPE[i][1] * BLOCK_SIZE;
    M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_PLAYER);
  }
  
  // Player plane shooting randomly
  if (!bulletActive && millis() - lastShotTime > random(2000, 4000)) {
    bulletActive = true;
    bulletY = 175;  // Start just above plane
    lastShotTime = millis();
  }
  
  // Update and draw bullet
  if (bulletActive) {
    bulletY -= 4;
    if (bulletY < -10) {
      bulletActive = false;
    } else {
      M5.Lcd.fillRect(planeX + 4 - 1, bulletY, 2, 4, COLOR_BULLET);
    }
  }
  
  // Pulsing "PRESS ANY BUTTON"
  int brightness = 128 + sin(elapsed / 300.0) * 127;
  uint16_t pulseColor = M5.Lcd.color565(brightness, brightness, brightness);
  M5.Lcd.setTextColor(pulseColor);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 210);
  M5.Lcd.print("PRESS ANY BUTTON");
  
  // Instructions at bottom
  M5.Lcd.setTextColor(COLOR_GRAY);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(25, 225);
  M5.Lcd.print("A:AUTO B:GOD");
}

void drawGameOver() {
  M5.Lcd.fillRect(10, 60, 115, 120, TFT_BLACK);
  M5.Lcd.drawRect(10, 60, 115, 120, TFT_WHITE);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(30, 65);
  M5.Lcd.print("GAME");
  M5.Lcd.setCursor(30, 80);
  M5.Lcd.print("OVER");
  
  // Show score and statistics
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(15, 100);
  M5.Lcd.printf("Score: %d", score);
  
  M5.Lcd.setCursor(15, 112);
  M5.Lcd.printf("Wave: %d", wave);
  
  M5.Lcd.setCursor(15, 124);
  M5.Lcd.printf("Shots: %d", shotsFired);
  
  M5.Lcd.setCursor(15, 136);
  M5.Lcd.printf("Kills: %d", enemiesDestroyed);
  
  // Calculate accuracy if shots were fired
  if (shotsFired > 0) {
    int accuracy = (enemiesDestroyed * 100) / shotsFired;
    if (accuracy > 100) accuracy = 100;  // Cap at 100% (wing guns can make it higher)
    M5.Lcd.setCursor(15, 148);
    M5.Lcd.printf("Hit: %d%%", accuracy);
  }
  
  // Press to continue
  M5.Lcd.setTextColor(COLOR_GRAY);
  M5.Lcd.setCursor(15, 165);
  M5.Lcd.print("Press to retry");
}

void setup() {
  M5.begin();
  initInput();
  gameState = TITLE_SCREEN;  // Start at title screen
}

void loop() {
  M5.update();
  updateInput();
  
  // Title Screen
  if (gameState == TITLE_SCREEN) {
    drawTitleScreen();
    // Any button starts game
    if ((btnPressed && !btnLastPressed) || (btnAPressed && !btnALastPressed) || (btnBPressed && !btnBLastPressed)) {
      initGame();
      gameState = PLAYING;
    }
    delay(16);
    return;
  }
  
  // Game Over Screen
  if (gameState == GAME_OVER) {
    drawGameOver();
    if (btnPressed && !btnLastPressed) {
      gameState = TITLE_SCREEN;  // Back to title
    }
    delay(16);
    return;
  }
  
  // Playing
  if (gameState == PLAYING) {
    // Update game logic
    updatePlayer();
    updateBullets();
    updateEnemies();
    updateEnemyBullets();
    updatePowerups();
    updateParticles();
    updateExplosions();
    updateWaveSpawning();
    checkCollisions();
    
    // Check for game over
    if (!player.alive) {
      gameState = GAME_OVER;
    }
    
    // Draw everything
    M5.Lcd.fillScreen(COLOR_BG);
    drawBackground();
    drawPlayer();
    drawEnemies();
    drawBullets();
    drawPowerups();
    drawEnemyBullets();
    drawParticles();
    drawExplosions();
    drawHUD();
  }
  
  delay(16);  // ~60 FPS
}

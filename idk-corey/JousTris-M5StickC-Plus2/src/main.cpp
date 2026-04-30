/*
 * JOUSTRIS - Joust meets Tetris
 * M5StickC Plus2 with Mini JoyC Hat
 * 
 * Ride S and Z tetromino pieces as flying birds!
 * Combat is determined by vertical position: higher Y wins
 * Defeat enemies to drop eggs, collect them before they hatch
 * 
 * Controls:
 * - Joystick Left/Right: Move horizontally
 * - Button: Flap wings (upward impulse)
 * - Button A: Toggle auto-flap
 * - Button B: Toggle god mode
 */

#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"

// Display configuration
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240
#define BLOCK_SIZE 3

// Physics constants
#define GRAVITY 0.3
#define FLAP_POWER 4.5
#define MAX_VY 8.0
#define GROUND_LEVEL 230
#define HORIZONTAL_SPEED 2.0
#define DRAG 0.95

// Game constants
#define MAX_ENEMIES 8
#define MAX_EGGS 6
#define MAX_PARTICLES 40
#define MAX_PLATFORMS 5
#define MAX_PTERODACTYLS 2

// JoyC
UNIT_JOYC Joystick;
#define JoyC_ADDR 0x54
#define POS_X 0
#define POS_Y 1

// Colors (Tetris themed)
#define COLOR_BG 0x0000        // Black
#define COLOR_S 0x07E0         // Green (S-piece player)
#define COLOR_Z 0xF800         // Red (Z-piece enemy)
#define COLOR_PLATFORM 0x7BEF  // Gray
#define COLOR_EGG 0xFFE0       // Yellow
#define COLOR_LAVA 0xF800      // Red
#define COLOR_PARTICLE 0xFFFF  // White

// S-piece shape (facing RIGHT)
const int8_t S_SHAPE[4][2] = {{1,0},{2,0},{0,1},{1,1}};

// Z-piece shape (facing LEFT - mirror of S)
const int8_t Z_SHAPE[4][2] = {{0,0},{1,0},{1,1},{2,1}};

// Platform structure
struct Platform {
  int x, y, width;
};

Platform platforms[MAX_PLATFORMS];

// Particle system
struct Particle {
  float x, y, vx, vy;
  uint16_t color;
  unsigned long endTime;
  bool active;
};

Particle particles[MAX_PARTICLES];

// Player structure
struct Player {
  float x, y;
  float vx, vy;
  int lives;
  bool alive;
  unsigned long invincibleUntil;
  bool onPlatform;
  bool facingRight;
  bool respawning;
  unsigned long respawnTime;
} player;

// Enemy structure
struct Enemy {
  float x, y;
  float vx, vy;
  bool active;
  bool onPlatform;
  int aiState;
  unsigned long nextAIChange;
  bool facingRight;
  bool isHunter;  // Is this enemy actively hunting the player?
};

Enemy enemies[MAX_ENEMIES];

// Egg structure
struct Egg {
  float x, y;
  float vy;
  bool active;
  unsigned long hatchTime;
  bool onGround;
};

Egg eggs[MAX_EGGS];

// Pterodactyl structure (anti-stall hazard)
struct Pterodactyl {
  float x, y;
  float vx, vy;
  bool active;
  bool fromLeft;
};

Pterodactyl pterodactyls[MAX_PTERODACTYLS];

// Game state
int score = 0;
int wave = 1;
int enemiesThisWave = 0;
int enemiesActive = 0;
int eggsActive = 0;
bool waveComplete = false;
unsigned long waveStartTime = 0;
unsigned long lastActionTime = 0;  // Track player activity for pterodactyl spawn
int pterodactylsActive = 0;

// Game modes
bool godMode = false;
bool autoFlap = false;
unsigned long lastAutoFlapTime = 0;  // Track auto-flap timing

// Game states
enum GameState { TITLE_SCREEN, PLAYING, WAVE_COMPLETE, GAME_OVER };
GameState gameState = TITLE_SCREEN;

// Input
int joyX = 0, joyY = 0;
bool btnPressed = false, btnLastPressed = false;
bool btnBPressed = false, btnBLastPressed = false;
bool btnAPressed = false, btnALastPressed = false;

void initInput() {
  Wire.begin(0, 26, 100000UL);
  Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL);
}

void updateInput() {
  uint16_t rawX = Joystick.getADCValue(POS_X);
  uint16_t rawY = Joystick.getADCValue(POS_Y);
  
  joyX = (int)((rawX - 2048) / 16);
  joyY = (int)((rawY - 2048) / 16);
  
  btnLastPressed = btnPressed;
  btnPressed = (Joystick.getButtonStatus() == 0);
  btnBLastPressed = btnBPressed;
  btnBPressed = (M5.BtnB.isPressed());
  btnALastPressed = btnAPressed;
  btnAPressed = (M5.BtnA.isPressed());
  
  if (btnBPressed && !btnBLastPressed) {
    godMode = !godMode;
  }
  
  if (btnAPressed && !btnALastPressed) {
    autoFlap = !autoFlap;
  }
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

void generatePlatforms() {
  // Clear existing platforms
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    platforms[i].width = 0;
  }
  
  // Always have a safe bottom platform (mid-low)
  platforms[0].x = 40;
  platforms[0].y = 180;
  platforms[0].width = 55;
  
  // Generate 2-4 additional platforms based on wave
  int platformCount = min(2 + wave / 2, 4);  // More platforms in later waves
  
  for (int i = 1; i <= platformCount; i++) {
    // Random position with constraints
    platforms[i].x = random(10, 80);
    platforms[i].y = random(70, 160);  // Mid-height range
    
    // Width decreases slightly with wave difficulty
    int widthBase = random(30, 50);
    platforms[i].width = max(25, widthBase - wave);  // Never too small
    
    // Ensure platforms don't overlap too much vertically
    for (int j = 0; j < i; j++) {
      if (abs(platforms[i].y - platforms[j].y) < 30) {
        platforms[i].y = platforms[j].y + random(35, 45);
        if (platforms[i].y > 170) platforms[i].y = platforms[j].y - random(35, 45);
      }
    }
    
    // Keep within bounds
    platforms[i].y = constrain(platforms[i].y, 60, 180);
  }
}

void spawnEgg(float x, float y) {
  for (int i = 0; i < MAX_EGGS; i++) {
    if (!eggs[i].active) {
      eggs[i].x = x;
      eggs[i].y = y;
      eggs[i].vy = 0;
      eggs[i].active = true;
      eggs[i].onGround = false;
      // Aggressive hatch timing: Wave 1=6s, Wave 2=5s, Wave 5=3s, Wave 10+=2s
      int hatchMs = max(2000, 6000 - wave * 500);
      eggs[i].hatchTime = millis() + hatchMs;
      eggsActive++;
      break;
    }
  }
}

void spawnEnemy(float x, float y) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].x = x;
      enemies[i].y = y;
      enemies[i].vx = random(-20, 20) / 10.0;
      enemies[i].vy = 0;
      enemies[i].active = true;
      enemies[i].onPlatform = false;
      enemies[i].aiState = 0;
      enemies[i].nextAIChange = millis() + random(1000, 3000);
      enemies[i].facingRight = (enemies[i].vx > 0);
      enemies[i].isHunter = false;  // Start as wanderer, assigned later
      enemiesActive++;
      enemiesThisWave++;
      break;
    }
  }
}

void assignHunters() {
  // Calculate how many hunters based on wave (1-3 max)
  int maxHunters = 1;  // Wave 1-2: 1 hunter
  if (wave >= 3 && wave < 9) {
    maxHunters = 2;  // Wave 3-8: 2 hunters
  } else if (wave >= 9) {
    maxHunters = 3;  // Wave 9+: 3 hunters
  }
  
  // Count current hunters
  int currentHunters = 0;
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active && enemies[i].isHunter) {
      currentHunters++;
    }
  }
  
  // Assign new hunters if below max
  if (currentHunters < maxHunters) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (enemies[i].active && !enemies[i].isHunter) {
        enemies[i].isHunter = true;
        currentHunters++;
        if (currentHunters >= maxHunters) break;
      }
    }
  }
}

void spawnPterodactyl() {
  for (int i = 0; i < MAX_PTERODACTYLS; i++) {
    if (!pterodactyls[i].active) {
      pterodactyls[i].fromLeft = random(2);
      pterodactyls[i].x = pterodactyls[i].fromLeft ? -20 : SCREEN_WIDTH + 20;
      pterodactyls[i].y = random(40, 120);
      pterodactyls[i].vx = 0;
      pterodactyls[i].vy = 0;
      pterodactyls[i].active = true;
      pterodactylsActive++;
      break;
    }
  }
}

void initGame() {
  M5.Lcd.setRotation(0);
  M5.Lcd.fillScreen(COLOR_BG);
  
  player.x = SCREEN_WIDTH / 2;
  player.y = 100;
  player.vx = 0;
  player.vy = 0;
  player.lives = 3;
  player.alive = true;
  player.invincibleUntil = 0;
  player.onPlatform = false;
  player.facingRight = true;
  player.respawning = false;
  player.respawnTime = 0;
  
  for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
  for (int i = 0; i < MAX_EGGS; i++) eggs[i].active = false;
  for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
  for (int i = 0; i < MAX_PTERODACTYLS; i++) pterodactyls[i].active = false;
  
  score = 0;
  wave = 1;
  enemiesThisWave = 0;
  enemiesActive = 0;
  eggsActive = 0;
  pterodactylsActive = 0;
  waveComplete = false;
  waveStartTime = millis();
  lastActionTime = millis();
  
  // Generate initial platforms
  generatePlatforms();
  
  int startEnemies = 3;
  for (int i = 0; i < startEnemies; i++) {
    spawnEnemy(random(20, SCREEN_WIDTH - 20), random(20, 80));
  }
  
  // Assign initial hunters
  assignHunters();
}

bool isSpawnPointSafe(float x, float y) {
  // Check if spawn location is clear of enemies
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      float dx = x - enemies[i].x;
      float dy = y - enemies[i].y;
      if (abs(dx) < 20 && abs(dy) < 20) {
        return false;
      }
    }
  }
  
  // Check if spawn location is clear of eggs
  for (int i = 0; i < MAX_EGGS; i++) {
    if (eggs[i].active) {
      float dx = x - eggs[i].x;
      float dy = y - eggs[i].y;
      if (abs(dx) < 15 && abs(dy) < 15) {
        return false;
      }
    }
  }
  
  // Check if spawn location is clear of pterodactyls
  for (int i = 0; i < MAX_PTERODACTYLS; i++) {
    if (pterodactyls[i].active) {
      float dx = x - pterodactyls[i].x;
      float dy = y - pterodactyls[i].y;
      if (abs(dx) < 25 && abs(dy) < 25) {
        return false;
      }
    }
  }
  
  return true;
}

void respawnPlayer() {
  // Try safe spawn locations (center platform area)
  float spawnX = SCREEN_WIDTH / 2;
  float spawnY = 100;
  
  // If center not safe, try alternate locations
  if (!isSpawnPointSafe(spawnX, spawnY)) {
    float tryLocations[][2] = {
      {SCREEN_WIDTH / 2, 60},
      {40, 120},
      {90, 120},
      {SCREEN_WIDTH / 2, 150}
    };
    
    bool foundSafe = false;
    for (int i = 0; i < 4; i++) {
      if (isSpawnPointSafe(tryLocations[i][0], tryLocations[i][1])) {
        spawnX = tryLocations[i][0];
        spawnY = tryLocations[i][1];
        foundSafe = true;
        break;
      }
    }
    
    // If no safe spot, delay respawn
    if (!foundSafe) {
      player.respawnTime = millis() + 200;  // Try again in 200ms
      return;
    }
  }
  
  // Spawn at safe location
  player.x = spawnX;
  player.y = spawnY;
  player.vx = 0;
  player.vy = 0;
  player.alive = true;
  player.respawning = false;
  player.invincibleUntil = millis() + 2000;  // 2 seconds invulnerability
  player.onPlatform = false;
  player.facingRight = true;
}

void checkRespawn() {
  if (player.respawning && millis() >= player.respawnTime) {
    respawnPlayer();
  }
}

void updatePlayer() {
  if (!player.alive) return;
  
  if (joyX < -30) {
    player.vx = -HORIZONTAL_SPEED;
    player.facingRight = false;
  } else if (joyX > 30) {
    player.vx = HORIZONTAL_SPEED;
    player.facingRight = true;
  } else {
    player.vx *= DRAG;
  }
  
  // Flapping logic:
  // Normal mode: Single press per button press (btnPressed && !btnLastPressed)
  // Auto-flap mode: Continuous while holding button (btnPressed with delay)
  if (autoFlap) {
    // Auto-flap: Continuous flapping while button held (with reasonable delay)
    // 250ms delay = ~4 flaps per second (prevents stuck at ceiling)
    if (btnPressed && millis() - lastAutoFlapTime > 250) {
      player.vy = -FLAP_POWER;
      player.onPlatform = false;
      lastAutoFlapTime = millis();
    }
  } else {
    // Normal mode: Single press only
    if (btnPressed && !btnLastPressed) {
      player.vy = -FLAP_POWER;
      player.onPlatform = false;
    }
  }
  
  if (!player.onPlatform) {
    player.vy += GRAVITY;
  }
  
  player.vy = constrain(player.vy, -MAX_VY, MAX_VY);
  
  player.x += player.vx;
  player.y += player.vy;
  
  if (player.x < -10) player.x = SCREEN_WIDTH + 10;
  if (player.x > SCREEN_WIDTH + 10) player.x = -10;
  
  player.onPlatform = false;
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    if (player.x > platforms[i].x && player.x < platforms[i].x + platforms[i].width) {
      if (player.y >= platforms[i].y - 10 && player.y < platforms[i].y + 5 && player.vy > 0) {
        player.y = platforms[i].y - 10;
        player.vy = 0;
        player.onPlatform = true;
      }
    }
  }
  
  if (player.y > GROUND_LEVEL) {
    if (!godMode) {
      player.lives--;
      spawnParticles(player.x, player.y, COLOR_LAVA, 15);
      if (player.lives <= 0) {
        player.alive = false;
      } else {
        // Trigger respawn after delay
        player.alive = false;
        player.respawning = true;
        player.respawnTime = millis() + 800;  // 0.8s respawn delay
      }
    } else {
      // God mode - just teleport up
      player.y = 50;
      player.vy = 0;
    }
  }
  
  // Top boundary - bounce back down
  if (player.y < 10) {
    player.y = 10;
    player.vy = 2.0;  // Bounce down gently
    // Reset auto-flap timer when hitting ceiling to prevent spam
    lastAutoFlapTime = millis();
  }
}

void updateEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;
    
    unsigned long now = millis();
    
    // AI changes state more frequently in later waves (aggressive)
    int aiChangeDelay = max(800, 3000 - wave * 150);  // 3s -> 0.8s
    if (now > enemies[i].nextAIChange) {
      // HUNTERS: Aggressively chase player
      if (enemies[i].isHunter) {
        int chaseChance = min(75 + wave * 5, 95);  // 80% -> 95% chase
        if (random(100) < chaseChance) {
          enemies[i].aiState = 1;  // Chase
        } else {
          enemies[i].aiState = 0;  // Wander briefly
        }
      }
      // WANDERERS: Mostly random movement
      else {
        int chaseChance = min(15 + wave * 2, 30);  // 17% -> 30% chase (rarely)
        if (random(100) < chaseChance) {
          enemies[i].aiState = 1;  // Occasional chase
        } else {
          enemies[i].aiState = random(2) == 0 ? 0 : 2;  // Wander or flee
        }
      }
      enemies[i].nextAIChange = now + random(aiChangeDelay, aiChangeDelay + 500);
    }
    
    // Speed increases with wave (1.0x -> 2.5x max)
    float aiSpeed = min(1.0 + wave * 0.12, 2.5);
    
    switch (enemies[i].aiState) {
      case 0: // Wander
        enemies[i].vx = random(-15, 15) / 10.0 * aiSpeed;
        break;
      case 1: // Chase (more aggressive in later waves)
        if (player.x > enemies[i].x) {
          enemies[i].vx = 1.5 * aiSpeed;
        } else {
          enemies[i].vx = -1.5 * aiSpeed;
        }
        // Try to get above player (more frequent in later waves)
        if (player.y < enemies[i].y - 10 && !enemies[i].onPlatform) {
          enemies[i].vy = -FLAP_POWER * (0.8 + wave * 0.02);  // Flap harder
        }
        break;
      case 2: // Flee
        if (player.x > enemies[i].x) {
          enemies[i].vx = -1.2 * aiSpeed;
        } else {
          enemies[i].vx = 1.2 * aiSpeed;
        }
        break;
    }
    
    if (!enemies[i].onPlatform) {
      enemies[i].vy += GRAVITY;
    }
    
    // Smart flapping - avoid lava and stay in play area
    bool shouldFlap = false;
    
    // Flap if getting close to lava
    if (enemies[i].y > GROUND_LEVEL - 40) {
      shouldFlap = true;
    }
    // Random flapping to stay airborne
    else if (random(100) < 3 + wave) {
      shouldFlap = true;
    }
    // Flap if too low while chasing player
    else if (enemies[i].aiState == 1 && enemies[i].y > 150) {
      shouldFlap = true;
    }
    
    if (shouldFlap && !enemies[i].onPlatform) {
      enemies[i].vy = -FLAP_POWER * 0.7;
      enemies[i].onPlatform = false;
    }
    
    enemies[i].vy = constrain(enemies[i].vy, -MAX_VY, MAX_VY);
    
    enemies[i].x += enemies[i].vx;
    enemies[i].y += enemies[i].vy;
    
    // Update facing direction based on velocity
    if (enemies[i].vx > 0.5) {
      enemies[i].facingRight = true;
    } else if (enemies[i].vx < -0.5) {
      enemies[i].facingRight = false;
    }
    
    if (enemies[i].x < -10) enemies[i].x = SCREEN_WIDTH + 10;
    if (enemies[i].x > SCREEN_WIDTH + 10) enemies[i].x = -10;
    
    enemies[i].onPlatform = false;
    for (int p = 0; p < MAX_PLATFORMS; p++) {
      if (enemies[i].x > platforms[p].x && enemies[i].x < platforms[p].x + platforms[p].width) {
        if (enemies[i].y >= platforms[p].y - 10 && enemies[i].y < platforms[p].y + 5 && enemies[i].vy > 0) {
          enemies[i].y = platforms[p].y - 10;
          enemies[i].vy = 0;
          enemies[i].onPlatform = true;
        }
      }
    }
    
    if (enemies[i].y > GROUND_LEVEL) {
      enemies[i].active = false;
      enemiesActive--;
      spawnParticles(enemies[i].x, enemies[i].y, COLOR_LAVA, 10);
    }
    
    // Top boundary - bounce back down strongly (no camping!)
    if (enemies[i].y < 10) {
      enemies[i].y = 10;
      enemies[i].vy = 2.5;  // Strong bounce down
    }
  }
}

void updatePterodactyls() {
  for (int i = 0; i < MAX_PTERODACTYLS; i++) {
    if (!pterodactyls[i].active) continue;
    
    // Homing behavior - targets player position
    float targetX = player.x;
    float targetY = player.y;
    
    // Speed increases with wave (3.0 -> 5.0 max)
    float speed = min(3.0 + wave * 0.2, 5.0);
    
    // Smooth horizontal homing
    float dx = targetX - pterodactyls[i].x;
    if (abs(dx) > 5) {
      pterodactyls[i].vx = (dx > 0 ? 1 : -1) * speed;
    } else {
      pterodactyls[i].vx = dx * 0.2;
    }
    
    // Smooth vertical homing (slower than horizontal)
    float dy = targetY - pterodactyls[i].y;
    if (abs(dy) > 5) {
      pterodactyls[i].vy = (dy > 0 ? 1 : -1) * speed * 0.6;
    } else {
      pterodactyls[i].vy = dy * 0.2;
    }
    
    pterodactyls[i].x += pterodactyls[i].vx;
    pterodactyls[i].y += pterodactyls[i].vy;
    
    // Remove if off screen edges (player escaped)
    if (pterodactyls[i].x < -50 || pterodactyls[i].x > SCREEN_WIDTH + 50 ||
        pterodactyls[i].y < -50 || pterodactyls[i].y > SCREEN_HEIGHT + 50) {
      pterodactyls[i].active = false;
      pterodactylsActive--;
    }
  }
}

void checkPterodactylSpawn() {
  unsigned long now = millis();
  unsigned long waveTime = now - waveStartTime;
  unsigned long idleTime = now - lastActionTime;
  
  // Spawn conditions (anti-stall) - gets more aggressive each wave
  bool shouldSpawn = false;
  
  // Wave timer shortens each wave (30s -> 15s min)
  int waveTimeLimit = max(15000, 30000 - wave * 1000);
  if (waveTime > waveTimeLimit && pterodactylsActive == 0) {
    shouldSpawn = true;
  }
  
  // Idle timer shortens each wave (15s -> 8s min)
  int idleTimeLimit = max(8000, 15000 - wave * 500);
  if (enemiesActive == 0 && eggsActive > 0 && idleTime > idleTimeLimit && pterodactylsActive == 0) {
    shouldSpawn = true;
  }
  
  // Later waves: spawn multiple pterodactyls sooner
  if (shouldSpawn) {
    int spawnCount = min(1 + wave / 3, MAX_PTERODACTYLS);  // Start with 1, add more
    for (int i = 0; i < spawnCount; i++) {
      spawnPterodactyl();
    }
  }
}

void updateEggs() {
  for (int i = 0; i < MAX_EGGS; i++) {
    if (!eggs[i].active) continue;
    
    unsigned long now = millis();
    
    if (!eggs[i].onGround) {
      eggs[i].vy += GRAVITY * 0.5;
      eggs[i].y += eggs[i].vy;
      
      for (int p = 0; p < MAX_PLATFORMS; p++) {
        if (eggs[i].x > platforms[p].x && eggs[i].x < platforms[p].x + platforms[p].width) {
          if (eggs[i].y >= platforms[p].y - 5 && eggs[i].y < platforms[p].y + 5) {
            eggs[i].y = platforms[p].y - 5;
            eggs[i].vy = 0;
            eggs[i].onGround = true;
          }
        }
      }
      
      if (eggs[i].y >= GROUND_LEVEL - 5) {
        eggs[i].y = GROUND_LEVEL - 5;
        eggs[i].vy = 0;
        eggs[i].onGround = true;
      }
    }
    
    if (now > eggs[i].hatchTime) {
      spawnEnemy(eggs[i].x, eggs[i].y);
      eggs[i].active = false;
      eggsActive--;
      spawnParticles(eggs[i].x, eggs[i].y, COLOR_EGG, 8);
    }
    
    float dx = player.x - eggs[i].x;
    float dy = player.y - eggs[i].y;
    if (abs(dx) < 8 && abs(dy) < 8) {
      eggs[i].active = false;
      eggsActive--;
      score += 25;
      spawnParticles(eggs[i].x, eggs[i].y, COLOR_EGG, 10);
    }
  }
}

void updateParticles() {
  unsigned long now = millis();
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].vy += GRAVITY * 0.3;
      if (now > particles[i].endTime) {
        particles[i].active = false;
      }
    }
  }
}

void checkCollisions() {
  if (!player.alive || millis() < player.invincibleUntil) return;
  
  // Player vs Enemy
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;
    
    float dx = player.x - enemies[i].x;
    float dy = player.y - enemies[i].y;
    
    if (abs(dx) < 10 && abs(dy) < 12) {
      // GOD MODE: Always win, any direction!
      if (godMode) {
        enemies[i].active = false;
        enemiesActive--;
        score += 50;
        spawnParticles(enemies[i].x, enemies[i].y, COLOR_Z, 12);
        spawnEgg(enemies[i].x, enemies[i].y);
        lastActionTime = millis();
      }
      // Normal mode: Height-based combat
      else if (player.y < enemies[i].y - 3) {
        enemies[i].active = false;
        enemiesActive--;
        score += 50;
        spawnParticles(enemies[i].x, enemies[i].y, COLOR_Z, 12);
        spawnEgg(enemies[i].x, enemies[i].y);
        lastActionTime = millis();
      } else if (player.y > enemies[i].y + 3) {
        player.lives--;
        spawnParticles(player.x, player.y, COLOR_S, 12);
        lastActionTime = millis();
        if (player.lives <= 0) {
          player.alive = false;
        } else {
          // Trigger respawn after delay
          player.alive = false;
          player.respawning = true;
          player.respawnTime = millis() + 800;
        }
      } else {
        float bounceX = (player.x - enemies[i].x) / 5.0;
        player.vx = bounceX;
        enemies[i].vx = -bounceX;
        player.vy = -FLAP_POWER * 0.5;
        enemies[i].vy = -FLAP_POWER * 0.5;
      }
    }
  }
  
  // Pterodactyl collision
  for (int i = 0; i < MAX_PTERODACTYLS; i++) {
    if (!pterodactyls[i].active) continue;
    
    float dx = player.x - pterodactyls[i].x;
    float dy = player.y - pterodactyls[i].y;
    
    if (abs(dx) < 12 && abs(dy) < 12) {
      // GOD MODE: Always win!
      if (godMode) {
        pterodactyls[i].active = false;
        pterodactylsActive--;
        score += 100;
        spawnParticles(pterodactyls[i].x, pterodactyls[i].y, 0xFFFF, 20);
        lastActionTime = millis();
      }
      // Normal mode: Precise head hitbox required
      else if (player.y < pterodactyls[i].y - 5) {
        pterodactyls[i].active = false;
        pterodactylsActive--;
        score += 100;
        spawnParticles(pterodactyls[i].x, pterodactyls[i].y, 0xFFFF, 20);
        lastActionTime = millis();
      } else {
        player.lives--;
        spawnParticles(player.x, player.y, COLOR_S, 12);
        if (player.lives <= 0) {
          player.alive = false;
        } else {
          // Trigger respawn after delay
          player.alive = false;
          player.respawning = true;
          player.respawnTime = millis() + 800;
        }
      }
    }
  }
}

void checkWaveComplete() {
  // Wave complete when all enemies and eggs are cleared, and at least initial enemies were spawned
  if (enemiesActive == 0 && eggsActive == 0 && enemiesThisWave >= 3) {
    waveComplete = true;
    gameState = WAVE_COMPLETE;
  }
}

void nextWave() {
  wave++;
  enemiesThisWave = 0;
  waveComplete = false;
  waveStartTime = millis();
  lastActionTime = millis();
  
  // Clear remaining pterodactyls from previous wave
  for (int i = 0; i < MAX_PTERODACTYLS; i++) {
    pterodactyls[i].active = false;
  }
  pterodactylsActive = 0;
  
  // Regenerate platform layout for variety
  generatePlatforms();
  
  // Spawn more enemies each wave (3 -> 8 max)
  int enemiesToSpawn = min(3 + wave, 8);
  
  // Spawn at safe locations (not overlapping player)
  for (int i = 0; i < enemiesToSpawn; i++) {
    // Spawn above platforms, spread out
    float spawnX = random(20, SCREEN_WIDTH - 20);
    float spawnY = random(20, 80);
    
    // Don't spawn on player
    while (abs(spawnX - player.x) < 30 && abs(spawnY - player.y) < 30) {
      spawnX = random(20, SCREEN_WIDTH - 20);
      spawnY = random(20, 80);
    }
    
    spawnEnemy(spawnX, spawnY);
  }
  
  // Assign hunters based on wave level
  assignHunters();
  
  gameState = PLAYING;
}

void drawPlatforms() {
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    if (platforms[i].width > 0) {  // Only draw active platforms
      M5.Lcd.fillRect(platforms[i].x, platforms[i].y, platforms[i].width, 3, COLOR_PLATFORM);
    }
  }
}

void drawLavaBottom() {
  unsigned long frame = millis() / 100;
  for (int x = 0; x < SCREEN_WIDTH; x += 4) {
    int height = 3 + (int)(sin((x + frame) * 0.3) * 2);
    M5.Lcd.fillRect(x, GROUND_LEVEL, 4, height, COLOR_LAVA);
  }
}

void drawPlayer() {
  if (!player.alive) return;
  
  bool blink = (millis() < player.invincibleUntil) && ((millis() / 100) % 2);
  if (blink) return;
  
  // Choose shape based on facing direction
  const int8_t (*shape)[2] = player.facingRight ? S_SHAPE : Z_SHAPE;
  
  for (int i = 0; i < 4; i++) {
    int px = player.x + shape[i][0] * BLOCK_SIZE;
    int py = player.y + shape[i][1] * BLOCK_SIZE;
    M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_S);
  }
}

void drawEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;
    
    // Choose shape based on facing direction
    const int8_t (*shape)[2] = enemies[i].facingRight ? S_SHAPE : Z_SHAPE;
    
    for (int b = 0; b < 4; b++) {
      int px = enemies[i].x + shape[b][0] * BLOCK_SIZE;
      int py = enemies[i].y + shape[b][1] * BLOCK_SIZE;
      M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_Z);
    }
  }
}

void drawEggs() {
  for (int i = 0; i < MAX_EGGS; i++) {
    if (!eggs[i].active) continue;
    
    unsigned long timeLeft = eggs[i].hatchTime - millis();
    int pulse = (millis() / 200) % 2;
    int size = 5 + pulse;
    
    // Flash red when about to hatch
    uint16_t eggColor = (timeLeft < 1500) ? COLOR_LAVA : COLOR_EGG;
    
    M5.Lcd.fillCircle(eggs[i].x, eggs[i].y, size, eggColor);
    M5.Lcd.drawCircle(eggs[i].x, eggs[i].y, size + 1, COLOR_BG);
    
    // Warning bar in last 3 seconds
    if (timeLeft < 3000) {
      int barWidth = (timeLeft * 10) / 3000;
      M5.Lcd.fillRect(eggs[i].x - 5, eggs[i].y - 10, barWidth, 2, COLOR_LAVA);
    }
  }
}

void drawPterodactyls() {
  for (int i = 0; i < MAX_PTERODACTYLS; i++) {
    if (!pterodactyls[i].active) continue;
    
    // Draw pterodactyl as elongated triangle (danger!)
    // Body (orange/yellow)
    M5.Lcd.fillTriangle(
      pterodactyls[i].x - 6, pterodactyls[i].y + 3,
      pterodactyls[i].x + 6, pterodactyls[i].y + 3,
      pterodactyls[i].x, pterodactyls[i].y - 6,
      0xFD20  // Orange
    );
    
    // Head hitbox indicator (small red dot at tip)
    M5.Lcd.fillCircle(pterodactyls[i].x, pterodactyls[i].y - 6, 2, COLOR_LAVA);
    
    // Wings (flapping animation)
    int wingFlap = (millis() / 100) % 2;
    int wingOffset = wingFlap ? 4 : 6;
    M5.Lcd.drawLine(pterodactyls[i].x - wingOffset, pterodactyls[i].y, pterodactyls[i].x, pterodactyls[i].y - 2, 0xFD20);
    M5.Lcd.drawLine(pterodactyls[i].x + wingOffset, pterodactyls[i].y, pterodactyls[i].x, pterodactyls[i].y - 2, 0xFD20);
  }
}

void drawParticles() {
  for (int i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].active) {
      M5.Lcd.drawPixel(particles[i].x, particles[i].y, particles[i].color);
    }
  }
}

void drawHUD() {
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(1);
  
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.printf("S:%d", score);
  
  M5.Lcd.setCursor(2, 12);
  M5.Lcd.printf("W:%d", wave);
  
  M5.Lcd.setCursor(2, 22);
  M5.Lcd.printf("E:%d+%d", enemiesActive, eggsActive);
  
  for (int i = 0; i < player.lives; i++) {
    int lx = SCREEN_WIDTH - 12 - (i * 10);
    M5.Lcd.fillRect(lx + 1, 3, 2, 2, COLOR_S);
    M5.Lcd.fillRect(lx + 3, 3, 2, 2, COLOR_S);
    M5.Lcd.fillRect(lx, 5, 2, 2, COLOR_S);
    M5.Lcd.fillRect(lx + 1, 5, 2, 2, COLOR_S);
  }
  
  M5.Lcd.setCursor(40, 2);
  if (godMode) M5.Lcd.print("GOD");
  if (autoFlap) {
    M5.Lcd.setCursor(godMode ? 64 : 40, 2);
    M5.Lcd.print("AUTO");
  }
}

void drawTitleScreen() {
  static unsigned long startTime = millis();
  unsigned long elapsed = millis() - startTime;
  
  M5.Lcd.fillScreen(COLOR_BG);
  
  drawPlatforms();
  drawLavaBottom();
  
  int offset = (elapsed / 20) % 200;
  
  int sX = 30 + (offset / 2);
  int sY = 60 + sin(elapsed / 500.0) * 15;
  for (int i = 0; i < 4; i++) {
    int px = sX + S_SHAPE[i][0] * BLOCK_SIZE;
    int py = sY + S_SHAPE[i][1] * BLOCK_SIZE;
    M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_S);
  }
  
  int zX = 90 - (offset / 2);
  int zY = 80 + cos(elapsed / 400.0) * 15;
  for (int i = 0; i < 4; i++) {
    int px = zX + Z_SHAPE[i][0] * BLOCK_SIZE;
    int py = zY + Z_SHAPE[i][1] * BLOCK_SIZE;
    M5.Lcd.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_Z);
  }
  
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(10, 120);
  M5.Lcd.print("JOUST");
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(COLOR_S);
  M5.Lcd.setCursor(30, 145);
  M5.Lcd.print("TRIS");
  
  M5.Lcd.setTextColor(COLOR_PLATFORM);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(15, 175);
  M5.Lcd.print("Higher Y Wins!");
  
  M5.Lcd.setCursor(10, 188);
  M5.Lcd.print("Collect Eggs!");
  
  int brightness = 128 + sin(elapsed / 300.0) * 127;
  uint16_t pulseColor = M5.Lcd.color565(brightness, brightness, brightness);
  M5.Lcd.setTextColor(pulseColor);
  M5.Lcd.setCursor(5, 210);
  M5.Lcd.print("PRESS TO START");
  
  M5.Lcd.setTextColor(COLOR_PLATFORM);
  M5.Lcd.setCursor(15, 225);
  M5.Lcd.print("A:AUTO B:GOD");
}

void drawWaveComplete() {
  M5.Lcd.fillRect(20, 90, 95, 60, TFT_BLACK);
  M5.Lcd.drawRect(20, 90, 95, 60, TFT_WHITE);
  
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(30, 95);
  M5.Lcd.print("WAVE");
  M5.Lcd.setCursor(25, 113);
  M5.Lcd.printf("%d DONE", wave);
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(25, 135);
  M5.Lcd.print("Press to go");
}

void drawGameOver() {
  M5.Lcd.fillRect(15, 70, 105, 100, TFT_BLACK);
  M5.Lcd.drawRect(15, 70, 105, 100, TFT_WHITE);
  
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(30, 75);
  M5.Lcd.print("GAME");
  M5.Lcd.setCursor(30, 93);
  M5.Lcd.print("OVER");
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(20, 115);
  M5.Lcd.printf("Score: %d", score);
  
  M5.Lcd.setCursor(20, 128);
  M5.Lcd.printf("Wave: %d", wave);
  
  M5.Lcd.setTextColor(COLOR_PLATFORM);
  M5.Lcd.setCursor(20, 150);
  M5.Lcd.print("Press to retry");
}

void setup() {
  M5.begin();
  initInput();
  gameState = TITLE_SCREEN;
}

void loop() {
  M5.update();
  updateInput();
  
  if (gameState == TITLE_SCREEN) {
    drawTitleScreen();
    if ((btnPressed && !btnLastPressed) || (btnAPressed && !btnALastPressed) || (btnBPressed && !btnBLastPressed)) {
      initGame();
      gameState = PLAYING;
    }
    delay(16);
    return;
  }
  
  if (gameState == WAVE_COMPLETE) {
    drawWaveComplete();
    if (btnPressed && !btnLastPressed) {
      nextWave();
    }
    delay(16);
    return;
  }
  
  if (gameState == GAME_OVER) {
    drawGameOver();
    if (btnPressed && !btnLastPressed) {
      gameState = TITLE_SCREEN;
    }
    delay(16);
    return;
  }
  
  if (gameState == PLAYING) {
    updatePlayer();
    updateEnemies();
    updatePterodactyls();
    updateEggs();
    updateParticles();
    assignHunters();  // Reassign hunters if one died
    checkPterodactylSpawn();
    checkCollisions();
    checkWaveComplete();
    checkRespawn();
    
    if (!player.alive && !player.respawning) {
      gameState = GAME_OVER;
    }
    
    M5.Lcd.fillScreen(COLOR_BG);
    drawPlatforms();
    drawLavaBottom();
    drawPlayer();
    drawEnemies();
    drawPterodactyls();
    drawEggs();
    drawParticles();
    drawHUD();
  }
  
  delay(16);
}

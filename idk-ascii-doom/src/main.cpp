#include <M5Unified.h>
#include <math.h>
#include <vector>

namespace {

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
#if defined(STICKS3)
static constexpr int kPinUp     = 1;
static constexpr int kPinDown   = 2;
static constexpr int kPinLeft   = 3;
static constexpr int kPinRight  = 8;
static constexpr int kPinCenter = 43;
#elif defined(PCBFUN)
static constexpr int kPinUp     = 1;
static constexpr int kPinDown   = 2;
static constexpr int kPinLeft   = 3;
static constexpr int kPinRight  = 4;
static constexpr int kPinCenter = 5;
#else
static constexpr int kPinUp     = 32;
static constexpr int kPinDown   = 33;
static constexpr int kPinLeft   = 25;
static constexpr int kPinRight  = 26;
static constexpr int kPinCenter = 0;
#endif

// ─── Game State ────────────────────────────────────────────────
struct Player {
  float x, y;      // position
  float angle;     // view angle (radians)
  int hp, maxHp;
  int ammo;
  int weapon;      // 0=pistol, 1=shotgun, 2=plasma
  bool isFiring;
  int fireAnim;
};

struct Enemy {
  float x, y;
  int hp;
  int type;        // 0=demon, 1=cyborg, 2=spider, 3=spectre
  int state;       // 0=idle, 1=chasing, 2=attacking, 3=dead
  uint32_t lastAttack;
};

enum GameState {
  IN_GAME,
  MENU,
  DEAD,
  WON,
};

static Player g_player = {.x = 1.5f, .y = 1.5f, .angle = 0.0f, .hp = 100, .maxHp = 100, .ammo = 30, .weapon = 0, .isFiring = false, .fireAnim = 0};
static std::vector<Enemy> g_enemies;
static GameState g_state = IN_GAME;
static int g_level = 1;
static int g_kills = 0;
static uint32_t g_last_shot = 0;
static uint32_t g_frame = 0;
static bool g_needs_redraw = true;  // Dirty flag to avoid full redraws

// ─── Dungeon Map (16x16 grid, 0=open, 1=wall, 2=exit) ─────────
static constexpr int kMapSize = 16;
static uint8_t g_map[kMapSize][kMapSize];

// ─── Constants ─────────────────────────────────────────────────
static constexpr float kFOV = 1.047f; // 60 degrees in radians
static constexpr int kNumRays = 60; // Better resolution
static constexpr int kScreenW = 240;
static constexpr int kScreenH = 135;
static constexpr int kHalfH = 67;

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void generateDungeon() {
  for (int y = 0; y < kMapSize; ++y) {
    for (int x = 0; x < kMapSize; ++x) {
      if (x == 0 || y == 0 || x == kMapSize - 1 || y == kMapSize - 1) {
        g_map[y][x] = 1;  // border walls
      } else if (esp_random() % 100 < 20 && (x != 1 && y != 1)) {
        g_map[y][x] = 1;  // random pillars
      } else {
        g_map[y][x] = 0;
      }
    }
  }
  g_map[1][1] = 0; // ensure start is open
  g_player.x = 1.5f;
  g_player.y = 1.5f;
  g_player.angle = 0.785f; // 45 deg
  g_map[kMapSize - 2][kMapSize - 2] = 2; // exit
}

void spawnEnemies(int count) {
  g_enemies.clear();
  for (int i = 0; i < count; ++i) {
    int x, y;
    do {
      x = 2 + (esp_random() % (kMapSize - 3));
      y = 2 + (esp_random() % (kMapSize - 3));
    } while (g_map[y][x] != 0 || (x < 4 && y < 4));

    Enemy e = {
        .x = x + 0.5f,
        .y = y + 0.5f,
        .hp = 20 + (esp_random() % (10 + g_level * 10)),
        .type = (int)(esp_random() % 4),
        .state = 0,
        .lastAttack = 0,
    };
    g_enemies.push_back(e);
  }
}

// ─── DDA Raycasting ────────────────────────────────────────────
void castRays() {
  if (!g_needs_redraw) return;
  g_needs_redraw = false;
  
  M5.Display.startWrite();  // Reduce SPI overhead
  
  // Draw floor and ceiling
  M5.Display.fillRect(0, 0, kScreenW, kHalfH, M5.Display.color565(20, 20, 20));
  M5.Display.fillRect(0, kHalfH, kScreenW, kHalfH, M5.Display.color565(40, 40, 40));

  float zBuffer[kScreenW]; // To handle sprite occlusion

  // 1. Raycast walls
  for (int x = 0; x < kScreenW; ++x) {
    float cameraX = 2 * x / (float)kScreenW - 1; // -1 to 1
    float rayDirX = cosf(g_player.angle) + sinf(g_player.angle) * cameraX;
    float rayDirY = sinf(g_player.angle) - cosf(g_player.angle) * cameraX;

    int mapX = (int)g_player.x;
    int mapY = (int)g_player.y;

    float sideDistX, sideDistY;
    float deltaDistX = fabsf(1 / rayDirX);
    float deltaDistY = fabsf(1 / rayDirY);
    float perpWallDist;

    int stepX, stepY;
    int hit = 0;
    int side;

    if (rayDirX < 0) {
      stepX = -1;
      sideDistX = (g_player.x - mapX) * deltaDistX;
    } else {
      stepX = 1;
      sideDistX = (mapX + 1.0f - g_player.x) * deltaDistX;
    }
    if (rayDirY < 0) {
      stepY = -1;
      sideDistY = (g_player.y - mapY) * deltaDistY;
    } else {
      stepY = 1;
      sideDistY = (mapY + 1.0f - g_player.y) * deltaDistY;
    }

    // DDA loop
    while (hit == 0) {
      if (sideDistX < sideDistY) {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;
      } else {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;
      }
      if (mapX < 0 || mapX >= kMapSize || mapY < 0 || mapY >= kMapSize) break;
      if (g_map[mapY][mapX] > 0) hit = g_map[mapY][mapX];
    }

    if (side == 0) perpWallDist = (mapX - g_player.x + (1 - stepX) / 2) / rayDirX;
    else           perpWallDist = (mapY - g_player.y + (1 - stepY) / 2) / rayDirY;

    zBuffer[x] = perpWallDist; // Save for sprite rendering

    int lineHeight = (int)(kScreenH / perpWallDist);
    int drawStart = -lineHeight / 2 + kHalfH;
    if (drawStart < 0) drawStart = 0;
    int drawEnd = lineHeight / 2 + kHalfH;
    if (drawEnd >= kScreenH) drawEnd = kScreenH - 1;

    uint16_t color;
    if (hit == 1) { // Normal wall
      // Fake shading based on side
      if (side == 1) color = M5.Display.color565(120, 120, 120);
      else color = M5.Display.color565(80, 80, 80);
      // Distance fading
      int fade = std::max(0, 255 - (int)(perpWallDist * 20));
      color = M5.Display.color565(fade/2, fade/2, fade/2);
    } else if (hit == 2) { // Exit
      color = TFT_MAGENTA;
    } else {
      color = TFT_BLACK;
    }

    M5.Display.drawFastVLine(x, drawStart, drawEnd - drawStart, color);
  }

  // 2. Render Enemies (Sprites)
  // Calculate distances for sorting
  struct SpriteDist { int index; float dist; };
  std::vector<SpriteDist> spList;
  for (size_t i = 0; i < g_enemies.size(); i++) {
    if (g_enemies[i].state == 3) continue; // dead
    float dx = g_player.x - g_enemies[i].x;
    float dy = g_player.y - g_enemies[i].y;
    spList.push_back({(int)i, dx*dx + dy*dy});
  }

  // Sort sprites from far to close
  std::sort(spList.begin(), spList.end(), [](const SpriteDist& a, const SpriteDist& b) {
    return a.dist > b.dist;
  });

  M5.Display.setTextSize(2);
  M5.Display.setTextDatum(MC_DATUM);

  for (auto& sd : spList) {
    Enemy& enemy = g_enemies[sd.index];
    
    // Transform sprite
    float spriteX = enemy.x - g_player.x;
    float spriteY = enemy.y - g_player.y;
    
    // Inverse camera matrix
    float invDet = 1.0f / (1.0f); // Simplification, actually need plane vectors
    
    // Simpler sprite projection
    float eangle = atan2f(spriteY, spriteX) - g_player.angle;
    while (eangle < -PI) eangle += 2 * PI;
    while (eangle > PI) eangle -= 2 * PI;
    
    // Is sprite in front of us?
    if (fabsf(eangle) < PI/2) {
      float dist = sqrtf(sd.dist);
      int screenX = (int)((0.5f * (eangle / (kFOV/2)) + 0.5f) * kScreenW);
      int spriteScale = (int)(kScreenH / dist);
      
      if (screenX > 0 && screenX < kScreenW && dist < zBuffer[screenX]) {
        // Draw ASCII Enemy
        char eChar = 'D';
        uint16_t eColor = TFT_RED;
        if (enemy.type == 1) { eChar = 'C'; eColor = TFT_ORANGE; }
        else if (enemy.type == 2) { eChar = 'S'; eColor = TFT_PURPLE; }
        else if (enemy.type == 3) { eChar = '&'; eColor = TFT_CYAN; }

        int ypos = kHalfH + spriteScale/4;
        
        M5.Display.setTextColor(eColor, TFT_BLACK);
        M5.Display.setCursor(screenX - 5, ypos - 10);
        if (enemy.state == 2) M5.Display.print("!"); // Attacking
        else M5.Display.print(eChar);
      }
    }
  }

  // 3. Draw Minimap
  for (int y = 0; y < kMapSize; y++) {
    for (int x = 0; x < kMapSize; x++) {
      int dx = kScreenW - 40 + x * 2;
      int dy = 4 + y * 2;
      if (g_map[y][x] == 1) M5.Display.fillRect(dx, dy, 2, 2, TFT_WHITE);
      else if (g_map[y][x] == 2) M5.Display.fillRect(dx, dy, 2, 2, TFT_MAGENTA);
    }
  }
  // Player on minimap
  M5.Display.fillRect(kScreenW - 40 + (int)(g_player.x)*2, 4 + (int)(g_player.y)*2, 2, 2, TFT_GREEN);

  // 4. Draw Weapon & HUD
  M5.Display.setTextDatum(TL_DATUM);
  M5.Display.setTextSize(1);

  // Weapon Animation
  int wX = kScreenW / 2;
  int wY = kScreenH - 20;
  
  if (g_player.isFiring) {
    wX += (esp_random() % 4) - 2;
    wY += (esp_random() % 4);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(wX - 5, wY - 15);
    M5.Display.print("*BANG*");
    
    g_player.fireAnim++;
    if (g_player.fireAnim > 3) {
      g_player.isFiring = false;
      g_player.fireAnim = 0;
    }
  }

  M5.Display.setTextColor(TFT_LIGHTGREY);
  M5.Display.setCursor(wX - 10, wY);
  if (g_player.weapon == 0) M5.Display.print("=|--o");
  else if (g_player.weapon == 1) M5.Display.print("=[==o");
  
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("HP:%d AMO:%d LV:%d", g_player.hp, g_player.ammo, g_level);
  M5.Display.setCursor(4, 120);
  M5.Display.printf("W:%d K:%d FPS:30", g_player.weapon, g_kills);

  M5.Display.endWrite();  // End SPI transaction
}

void updateEnemies() {
  uint32_t now = millis();
  for (auto& enemy : g_enemies) {
    if (enemy.state == 3) continue;

    float dx = g_player.x - enemy.x;
    float dy = g_player.y - enemy.y;
    float dist = sqrtf(dx*dx + dy*dy);

    if (dist < 5.0f) {
      enemy.state = 1; // Chase
      
      if (dist > 1.2f) {
        // Move towards player
        float nx = enemy.x + (dx/dist) * 0.05f;
        float ny = enemy.y + (dy/dist) * 0.05f;
        if (g_map[(int)ny][(int)nx] == 0) {
          enemy.x = nx;
          enemy.y = ny;
        }
      } else {
        enemy.state = 2; // Attack
        if (now - enemy.lastAttack > 1000) {
          g_player.hp -= (5 + g_level*2);
          enemy.lastAttack = now;
          if (g_player.hp <= 0) {
            g_player.hp = 0;
            g_state = DEAD;
          }
        }
      }
    } else {
      enemy.state = 0; // Idle
    }
  }
}

void handleGameInput() {
  const float moveSpeed = 0.12f;
  const float turnSpeed = 0.08f;
  bool input_received = false;

  if (readPressed(kPinUp)) {
    float nx = g_player.x + cosf(g_player.angle) * moveSpeed;
    float ny = g_player.y + sinf(g_player.angle) * moveSpeed;
    if (g_map[(int)ny][(int)g_player.x] == 0 || g_map[(int)ny][(int)g_player.x] == 2) g_player.y = ny;
    if (g_map[(int)g_player.y][(int)nx] == 0 || g_map[(int)g_player.y][(int)nx] == 2) g_player.x = nx;
    input_received = true;
  }
  if (readPressed(kPinDown)) {
    float nx = g_player.x - cosf(g_player.angle) * moveSpeed;
    float ny = g_player.y - sinf(g_player.angle) * moveSpeed;
    if (g_map[(int)ny][(int)g_player.x] == 0) g_player.y = ny;
    if (g_map[(int)g_player.y][(int)nx] == 0) g_player.x = nx;
    input_received = true;
  }
  // FIXED: Swapped LEFT/RIGHT rotation direction
  if (readPressed(kPinLeft)) {
    g_player.angle += turnSpeed;  // was -= turnSpeed
    input_received = true;
  }
  if (readPressed(kPinRight)) {
    g_player.angle -= turnSpeed;  // was += turnSpeed
    input_received = true;
  }

  // Handle Exit
  if (g_map[(int)g_player.y][(int)g_player.x] == 2) {
    g_level++;
    if (g_level > 5) {
      g_state = WON;
    } else {
      generateDungeon();
      spawnEnemies(5 + g_level * 2);
    }
  }

  if (readPressed(kPinCenter)) {
    uint32_t now = millis();
    if (now - g_last_shot > 300 && g_player.ammo > 0) {
      g_player.isFiring = true;
      g_player.ammo--;
      
      // Simple hitscan
      for (auto& enemy : g_enemies) {
        if (enemy.state == 3) continue;
        float edx = enemy.x - g_player.x;
        float edy = enemy.y - g_player.y;
        float eangle = atan2f(edy, edx) - g_player.angle;
        while (eangle < -PI) eangle += 2 * PI;
        while (eangle > PI) eangle -= 2 * PI;
        
        if (fabsf(eangle) < 0.2f) {
          enemy.hp -= 25;
          if (enemy.hp <= 0) {
            enemy.state = 3;
            g_kills++;
            g_player.ammo += 5; // drop ammo
          }
          break; // Only hit one per shot
        }
      }
      g_last_shot = now;
      input_received = true;
    }
  }

  if (input_received) g_needs_redraw = true;
  if (M5.BtnPWR.wasPressed()) {
    g_state = MENU;
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
#if defined(STICKS3)
  M5.Display.setRotation(1);
#else
  M5.Display.setRotation(3);
#endif
  M5.Display.setBrightness(180);

  Serial.begin(115200);
  delay(200);
  Serial.println("idk-ascii-doom starting...");

  // Initialize 5-way pins
  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);

  generateDungeon();
  spawnEnemies(5);
}

void loop() {
  M5.update();
  g_frame++;

  switch (g_state) {
    case IN_GAME:
      handleGameInput();
      updateEnemies();
      castRays();
      break;
    case MENU:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setTextSize(2);
      M5.Display.setCursor(60, 60);
      M5.Display.print("PAUSED");
      M5.Display.setTextSize(1);
      M5.Display.setCursor(40, 90);
      M5.Display.print("BtnA to Resume");
      if (M5.BtnA.wasPressed()) g_state = IN_GAME;
      break;
    case DEAD:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_RED);
      M5.Display.setTextSize(2);
      M5.Display.setCursor(40, 60);
      M5.Display.print("YOU DIED");
      if (M5.BtnA.wasPressed() || M5.BtnPWR.wasPressed()) ESP.restart();
      break;
    case WON:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.setTextSize(2);
      M5.Display.setCursor(50, 60);
      M5.Display.print("VICTORY!");
      if (M5.BtnA.wasPressed() || M5.BtnPWR.wasPressed()) ESP.restart();
      break;
  }

  delay(20);
}

#include <M5Unified.h>
#include <math.h>
#include <vector>

namespace {

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

// ─── Game State ────────────────────────────────────────────────
struct Player {
  float x, y;      // position
  float angle;     // view angle (radians)
  int hp, maxHp;
  int ammo;
  int weapon;      // 0=pistol, 1=shotgun, 2=plasma
};

struct Enemy {
  float x, y;
  int hp;
  int type;        // 0=demon, 1=cyborg, 2=spider, 3=spectre
  bool visible;
};

enum GameState {
  IN_GAME,
  MENU,
  DEAD,
  WON,
};

static Player g_player = {.x = 50, .y = 50, .angle = 0, .hp = 100, .maxHp = 100, .ammo = 30, .weapon = 0};
static std::vector<Enemy> g_enemies;
static GameState g_state = IN_GAME;
static int g_level = 1;
static int g_kills = 0;
static uint32_t g_last_shot = 0;

// ─── Dungeon Map (simple 32x32 grid, 0=open, 1=wall) ──────────
static uint8_t g_map[32][32];

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void generateDungeon() {
  // Simple maze generation (placeholder)
  for (int y = 0; y < 32; ++y) {
    for (int x = 0; x < 32; ++x) {
      if (x == 0 || y == 0 || x == 31 || y == 31) {
        g_map[y][x] = 1;  // border walls
      } else if ((x % 4) == 0 || (y % 4) == 0) {
        g_map[y][x] = esp_random() % 2;
      } else {
        g_map[y][x] = 0;
      }
    }
  }
  g_map[1][1] = 0;  // player start
  g_map[30][30] = 0;  // exit
}

void spawnEnemies(int count) {
  g_enemies.clear();
  for (int i = 0; i < count; ++i) {
    int x = 10 + (esp_random() % 12);
    int y = 10 + (esp_random() % 12);
    if (g_map[y][x] == 0) {
      Enemy e = {
          .x = (float)x,
          .y = (float)y,
          .hp = 20 + (esp_random() % 20),
          .type = (int)(esp_random() % 4),
          .visible = false,
      };
      g_enemies.push_back(e);
    }
  }
}

void castRay(float angle, int& distanceOut, int& typeOut) {
  // Simple raycasting for DOOM-like effect
  float dx = cosf(angle);
  float dy = sinf(angle);

  for (int dist = 1; dist < 20; ++dist) {
    float px = g_player.x + dx * dist;
    float py = g_player.y + dy * dist;
    int mx = (int)px;
    int my = (int)py;

    if (mx < 0 || mx >= 32 || my < 0 || my >= 32 || g_map[my][mx] == 1) {
      distanceOut = dist;
      typeOut = 0;  // wall
      return;
    }

    // Check for enemies
    for (auto& enemy : g_enemies) {
      float edx = enemy.x - g_player.x;
      float edy = enemy.y - g_player.y;
      float edist = sqrtf(edx * edx + edy * edy);
      float eangle = atan2f(edy, edx);

      if (fabsf(eangle - angle) < 0.3f && edist < dist) {
        distanceOut = edist;
        typeOut = 1;  // enemy
        return;
      }
    }
  }

  distanceOut = 20;
  typeOut = -1;  // nothing
}

void drawGameScreen() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(1);

  // First-person view (16 rays)
  const int rayCount = 16;
  for (int i = 0; i < rayCount; ++i) {
    float rayAngle = g_player.angle - 0.5f + (float)i / rayCount;
    int dist, type;
    castRay(rayAngle, dist, type);

    // ASCII wall rendering based on distance
    char wallChar = ' ';
    uint16_t color = TFT_WHITE;
    
    if (type == 0) {  // wall
      if (dist < 5) {
        wallChar = '#';
        color = TFT_WHITE;
      } else if (dist < 10) {
        wallChar = 'H';
        color = TFT_LIGHTGREY;
      } else {
        wallChar = '-';
        color = TFT_DARKGREY;
      }
    } else if (type == 1) {  // enemy
      if (dist < 5) {
        wallChar = 'D';
        color = TFT_RED;
      } else {
        wallChar = 'd';
        color = TFT_ORANGE;
      }
    }

    M5.Display.setTextColor(color);
    M5.Display.setCursor(15 * i, 50);
    M5.Display.print(wallChar);
  }

  // HUD
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("HP:%d  AMO:%d  LV:%d", g_player.hp, g_player.ammo, g_level);

  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(4, 120);
  M5.Display.printf("W:%d  K:%d  FPS:30", g_player.weapon, g_kills);
}

void handleGameInput() {
  const float moveSpeed = 0.5f;
  const float turnSpeed = 0.1f;

  if (readPressed(kPinUp)) {
    g_player.x += cosf(g_player.angle) * moveSpeed;
    g_player.y += sinf(g_player.angle) * moveSpeed;
  }
  if (readPressed(kPinDown)) {
    g_player.x -= cosf(g_player.angle) * moveSpeed;
    g_player.y -= sinf(g_player.angle) * moveSpeed;
  }
  if (readPressed(kPinLeft)) {
    g_player.angle -= turnSpeed;
  }
  if (readPressed(kPinRight)) {
    g_player.angle += turnSpeed;
  }

  if (readPressed(kPinCenter)) {
    uint32_t now = millis();
    if (now - g_last_shot > 200) {
      // Fire weapon
      for (auto& enemy : g_enemies) {
        float edx = enemy.x - g_player.x;
        float edy = enemy.y - g_player.y;
        float eangle = atan2f(edy, edx);
        if (fabsf(eangle - g_player.angle) < 0.2f) {
          enemy.hp -= 15;  // damage
          if (enemy.hp <= 0) g_kills++;
        }
      }
      g_last_shot = now;
      g_player.ammo--;
    }
  }

  if (M5.BtnPWR.wasPressed()) {
    g_state = MENU;
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
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

  switch (g_state) {
    case IN_GAME:
      handleGameInput();
      drawGameScreen();
      break;
    case MENU:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(50, 60);
      M5.Display.print("PAUSED");
      if (M5.BtnA.wasPressed()) g_state = IN_GAME;
      break;
    case DEAD:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_RED);
      M5.Display.setCursor(40, 60);
      M5.Display.print("YOU DIED");
      break;
    case WON:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.setCursor(50, 60);
      M5.Display.print("VICTORY!");
      break;
  }

  delay(5);
}

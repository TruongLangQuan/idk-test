#include <M5Unified.h>
#include <vector>
#include <cmath>
#include <SPIFFS.h>

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

// ─── Game Objects ──────────────────────────────────────────────
struct Player {
  float x, y;
  float vy;
  int width, height;
  bool jumping;
  bool alive;
  int score;
  int jumpsLeft;       // for double jump
  bool gravityFlipped; // gravity flip mode
  float rotation;      // visual rotation
};

struct Obstacle {
  float x;
  int type;     // 0=spike, 1=block, 2=platform, 3=gravity_orb, 4=speed_gate, 5=coin
  int height;   // variable height
  uint16_t color;
  bool collected; // for coins/orbs
};

struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  int life;
};

struct Trail {
  float x, y;
  int life;
};

enum GameMode {
  MENU,
  LEVEL_SELECT,
  PLAYING,
  GAME_OVER,
  PAUSED,
};

// ─── Global State ──────────────────────────────────────────────
static Player g_player;
static std::vector<Obstacle> g_obstacles;
static std::vector<Particle> g_particles;
static std::vector<Trail> g_trail;
static GameMode g_mode = MENU;
static uint32_t g_frame = 0;
static int g_level = 1;
static float g_scroll = 0;
static float g_speed = 2.0f;
static int g_best_score = 0;
static int g_coins = 0;
static int g_menu_idx = 0;
static float g_bg_scroll = 0;

// ─── Constants ─────────────────────────────────────────────────
static constexpr float kGravity = 0.45f;
static constexpr float kJumpForce = -6.5f;
static constexpr int kScreenW = 240;
static constexpr int kScreenH = 135;
static constexpr int kGroundY = 105;
static constexpr int kMaxLevel = 5;

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void resetPlayer() {
  g_player = {30, (float)(kGroundY - 8), 0, 8, 8, false, true, 0, 2, false, 0};
}

void spawnParticles(float x, float y, uint16_t color, int count) {
  for (int i = 0; i < count && g_particles.size() < 30; ++i) {
    Particle p = {
        x, y,
        (float)(esp_random() % 5) - 2.5f,
        (float)(esp_random() % 5) - 3.0f,
        color, 15 + (int)(esp_random() % 10),
    };
    g_particles.push_back(p);
  }
}

void generateLevel(int level) {
  g_obstacles.clear();
  g_scroll = 0;
  g_speed = 1.5f + level * 0.3f;
  g_coins = 0;
  resetPlayer();

  int obstacleCount = 15 + level * 5;
  float spacing = 25.0f - level * 1.5f;
  if (spacing < 18) spacing = 18;

  for (int i = 0; i < obstacleCount; ++i) {
    float obstacleX = 200 + i * spacing;
    int type;
    // More variety at higher levels
    if (level <= 2) {
      type = esp_random() % 3;  // spike, block, platform
    } else {
      type = esp_random() % 6;  // all types
    }

    int h = 8 + (esp_random() % 12);
    uint16_t color;
    switch (type) {
      case 0: color = TFT_RED; break;
      case 1: color = M5.Display.color565(100, 100, 200); break;
      case 2: color = TFT_GREEN; break;
      case 3: color = TFT_MAGENTA; break;  // gravity orb
      case 4: color = TFT_CYAN; break;     // speed gate
      case 5: color = TFT_YELLOW; break;   // coin
      default: color = TFT_WHITE;
    }
    g_obstacles.push_back({obstacleX, type, h, color, false});
  }
}

void updateParticles() {
  for (auto& p : g_particles) {
    p.x += p.vx;
    p.y += p.vy;
    p.vy += 0.1f;
    p.life--;
  }
  g_particles.erase(
      std::remove_if(g_particles.begin(), g_particles.end(),
                     [](const Particle& p) { return p.life <= 0; }),
      g_particles.end());
}

void updateTrail() {
  if (g_player.alive && g_frame % 2 == 0 && g_trail.size() < 20) {
    g_trail.push_back({g_player.x, g_player.y + g_player.height / 2.0f, 8});
  }
  for (auto& t : g_trail) t.life--;
  g_trail.erase(
      std::remove_if(g_trail.begin(), g_trail.end(),
                     [](const Trail& t) { return t.life <= 0; }),
      g_trail.end());
}

void updatePlayer() {
  float grav = g_player.gravityFlipped ? -kGravity : kGravity;
  g_player.vy += grav;
  g_player.y += g_player.vy;

  // Scroll
  g_player.x += g_speed;
  g_scroll = g_player.x - 30;

  // Ground/ceiling collision
  float groundLimit = g_player.gravityFlipped ? 14.0f : (float)(kGroundY - g_player.height);
  float ceilLimit = g_player.gravityFlipped ? (float)(kGroundY - g_player.height) : 14.0f;

  if (!g_player.gravityFlipped && g_player.y >= groundLimit) {
    g_player.y = groundLimit;
    g_player.vy = 0;
    g_player.jumping = false;
    g_player.jumpsLeft = 2;
  }
  if (g_player.gravityFlipped && g_player.y <= 14.0f) {
    g_player.y = 14.0f;
    g_player.vy = 0;
    g_player.jumping = false;
    g_player.jumpsLeft = 2;
  }

  // Visual rotation
  if (g_player.jumping) g_player.rotation += 5.0f;

  // Obstacle collision
  for (auto& obs : g_obstacles) {
    float sx = obs.x - g_scroll;
    if (sx < -20 || sx > kScreenW + 20) continue;

    float obsW = (obs.type == 5) ? 6 : 12;
    float obsH = (obs.type == 0) ? 10 : obs.height;
    float obsY = kGroundY - obsH;
    if (obs.type == 2) obsY = kGroundY - 15;  // platform

    bool overlap = g_player.x + g_player.width > obs.x &&
                   g_player.x < obs.x + obsW &&
                   g_player.y + g_player.height > obsY &&
                   g_player.y < obsY + obsH;

    if (!overlap) continue;

    switch (obs.type) {
      case 0:  // spike - death
      case 1:  // block - death
        g_player.alive = false;
        spawnParticles(g_player.x, g_player.y, TFT_RED, 10);
        break;
      case 2:  // platform - land on top
        if (g_player.vy > 0 && g_player.y + g_player.height - g_player.vy <= obsY) {
          g_player.y = obsY - g_player.height;
          g_player.vy = 0;
          g_player.jumping = false;
          g_player.jumpsLeft = 2;
        }
        break;
      case 3:  // gravity orb
        if (!obs.collected) {
          g_player.gravityFlipped = !g_player.gravityFlipped;
          g_player.vy = g_player.gravityFlipped ? kJumpForce : -kJumpForce;
          obs.collected = true;
          spawnParticles(obs.x, obsY, TFT_MAGENTA, 5);
        }
        break;
      case 4:  // speed gate
        if (!obs.collected) {
          g_speed += 0.5f;
          obs.collected = true;
          spawnParticles(obs.x, obsY, TFT_CYAN, 5);
        }
        break;
      case 5:  // coin
        if (!obs.collected) {
          g_coins++;
          g_player.score += 50;
          obs.collected = true;
          spawnParticles(obs.x, obsY, TFT_YELLOW, 3);
        }
        break;
    }
  }

  g_player.score++;

  // Level complete
  if (g_obstacles.size() > 0) {
    float lastX = g_obstacles.back().x;
    if (g_player.x > lastX + 100) {
      if (g_level < kMaxLevel) {
        g_level++;
        generateLevel(g_level);
      } else {
        g_mode = GAME_OVER;
        if (g_player.score > g_best_score) g_best_score = g_player.score;
      }
    }
  }
}

void handleInput() {
  bool jumpPress = readPressed(kPinCenter) || readPressed(kPinUp) || M5.BtnA.wasPressed();

  if (jumpPress) {
    if (g_player.jumpsLeft > 0) {
      float force = g_player.gravityFlipped ? -kJumpForce : kJumpForce;
      g_player.vy = force;
      g_player.jumping = true;
      g_player.jumpsLeft--;
      spawnParticles(g_player.x, g_player.y + g_player.height, TFT_WHITE, 2);
    }
  }

  if (readPressed(kPinDown)) {
    g_player.vy += g_player.gravityFlipped ? -0.3f : 0.3f;
  }

  if (M5.BtnPWR.wasPressed()) {
    g_mode = (g_mode == PLAYING) ? PAUSED : PLAYING;
  }
}

void drawBackground() {
  // Parallax background
  g_bg_scroll += 0.5f;
  for (int i = 0; i < 8; ++i) {
    int x = (int)(i * 35 - (int)g_bg_scroll % 35);
    M5.Display.drawFastVLine(x, 14, kGroundY - 14, M5.Display.color565(15, 15, 25));
  }
  // Ground line
  M5.Display.drawFastHLine(0, kGroundY, kScreenW, M5.Display.color565(60, 60, 80));
  M5.Display.drawFastHLine(0, kGroundY + 1, kScreenW, M5.Display.color565(40, 40, 60));
}

void drawGameScreen() {
  M5.Display.fillScreen(TFT_BLACK);
  drawBackground();

  // Trail
  for (auto& t : g_trail) {
    int sx = (int)(t.x - g_scroll);
    if (sx >= 0 && sx < kScreenW) {
      int alpha = t.life * 30;
      M5.Display.drawPixel(sx, (int)t.y, M5.Display.color565(0, alpha, alpha));
    }
  }

  // Obstacles
  for (auto& obs : g_obstacles) {
    float sx = obs.x - g_scroll;
    if (sx < -20 || sx > kScreenW) continue;
    if (obs.collected) continue;

    int isx = (int)sx;
    switch (obs.type) {
      case 0:  // Spike
        M5.Display.fillTriangle(isx + 6, kGroundY - 10, isx, kGroundY, isx + 12, kGroundY, obs.color);
        break;
      case 1:  // Block
        M5.Display.fillRect(isx, kGroundY - obs.height, 12, obs.height, obs.color);
        M5.Display.drawRect(isx, kGroundY - obs.height, 12, obs.height, TFT_WHITE);
        break;
      case 2:  // Platform
        M5.Display.fillRect(isx, kGroundY - 15, 20, 4, obs.color);
        break;
      case 3:  // Gravity orb
        M5.Display.drawCircle(isx + 5, kGroundY - 10, 5, obs.color);
        M5.Display.drawPixel(isx + 5, kGroundY - 10, TFT_WHITE);
        break;
      case 4:  // Speed gate
        M5.Display.drawRect(isx, kGroundY - 20, 8, 20, obs.color);
        M5.Display.drawFastVLine(isx + 4, kGroundY - 18, 16, TFT_WHITE);
        break;
      case 5:  // Coin
        M5.Display.fillCircle(isx + 3, kGroundY - 8, 3, obs.color);
        break;
    }
  }

  // Player
  if (g_player.alive) {
    M5.Display.fillRect((int)g_player.x - (int)g_scroll, (int)g_player.y,
                        g_player.width, g_player.height, TFT_YELLOW);
    // Eyes
    M5.Display.drawPixel((int)g_player.x - (int)g_scroll + 2, (int)g_player.y + 2, TFT_BLACK);
    M5.Display.drawPixel((int)g_player.x - (int)g_scroll + 5, (int)g_player.y + 2, TFT_BLACK);
  }

  // Particles
  for (auto& p : g_particles) {
    int px = (int)(p.x - g_scroll);
    if (px >= 0 && px < kScreenW) {
      M5.Display.drawPixel(px, (int)p.y, p.color);
    }
  }

  // HUD
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("Score:%d  Lvl:%d/%d  Best:%d", g_player.score, g_level, kMaxLevel, g_best_score);
  M5.Display.setCursor(200, 4);
  M5.Display.printf("$%d", g_coins);

  // Gravity indicator
  if (g_player.gravityFlipped) {
    M5.Display.setTextColor(TFT_MAGENTA, TFT_BLACK);
    M5.Display.setCursor(4, 125);
    M5.Display.print("FLIP!");
  }

  if (g_mode == PAUSED) {
    M5.Display.fillRect(60, 45, 120, 40, M5.Display.color565(0, 0, 60));
    M5.Display.drawRect(60, 45, 120, 40, TFT_WHITE);
    M5.Display.setTextColor(TFT_WHITE, M5.Display.color565(0, 0, 60));
    M5.Display.setCursor(85, 60);
    M5.Display.print("PAUSED");
  }
}

void drawMenu() {
  M5.Display.fillScreen(TFT_BLACK);

  // Title with animation
  M5.Display.setTextSize(2);
  int titleY = 20 + (int)(sinf(g_frame * 0.05f) * 3);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(30, titleY);
  M5.Display.print("GEOMETRY");
  M5.Display.setCursor(70, titleY + 18);
  M5.Display.print("DASH");

  M5.Display.setTextSize(1);
  const char* items[] = {"Play", "Level Select", "Best Score"};
  for (int i = 0; i < 3; ++i) {
    M5.Display.setTextColor(i == g_menu_idx ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(70, 65 + i * 14);
    M5.Display.printf("%s %s", i == g_menu_idx ? ">" : " ", items[i]);
  }

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(20, 120);
  M5.Display.print("UP/DOWN:sel  CENTER:go  PWR:quit");
}

void drawGameOver() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(g_player.score > 0 ? TFT_GREEN : TFT_RED, TFT_BLACK);
  M5.Display.setCursor(40, 30);
  M5.Display.print(g_level > kMaxLevel ? "YOU WIN!" : "GAME OVER");

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(50, 65);
  M5.Display.printf("Score: %d", g_player.score);
  M5.Display.setCursor(50, 80);
  M5.Display.printf("Best: %d", g_best_score);
  M5.Display.setCursor(50, 95);
  M5.Display.printf("Coins: %d", g_coins);

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(40, 115);
  M5.Display.print("CENTER: retry  PWR: menu");
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

  // Initialize 5-way pins
  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);

  generateLevel(g_level);
}

void loop() {
  M5.update();
  g_frame++;

  switch (g_mode) {
    case MENU: {
      drawMenu();
      if (readPressed(kPinUp)) {
        static uint32_t last = 0;
        if (millis() - last > 200) { g_menu_idx = (g_menu_idx + 2) % 3; last = millis(); }
      }
      if (readPressed(kPinDown)) {
        static uint32_t last = 0;
        if (millis() - last > 200) { g_menu_idx = (g_menu_idx + 1) % 3; last = millis(); }
      }
      if (readPressed(kPinCenter)) {
        static uint32_t last = 0;
        if (millis() - last > 300) {
          if (g_menu_idx == 0) { g_level = 1; generateLevel(g_level); g_mode = PLAYING; }
          else if (g_menu_idx == 1) g_mode = LEVEL_SELECT;
          last = millis();
        }
      }
      break;
    }

    case LEVEL_SELECT: {
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextSize(1);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(60, 20);
      M5.Display.print("SELECT LEVEL");
      for (int i = 1; i <= kMaxLevel; ++i) {
        M5.Display.setTextColor(i == g_level ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(60, 35 + i * 14);
        M5.Display.printf("%s Level %d", i == g_level ? ">" : " ", i);
      }
      if (readPressed(kPinUp)) {
        static uint32_t last = 0;
        if (millis() - last > 200) { g_level = g_level > 1 ? g_level - 1 : kMaxLevel; last = millis(); }
      }
      if (readPressed(kPinDown)) {
        static uint32_t last = 0;
        if (millis() - last > 200) { g_level = g_level < kMaxLevel ? g_level + 1 : 1; last = millis(); }
      }
      if (readPressed(kPinCenter)) {
        static uint32_t last = 0;
        if (millis() - last > 300) { generateLevel(g_level); g_mode = PLAYING; last = millis(); }
      }
      if (M5.BtnPWR.wasPressed()) g_mode = MENU;
      break;
    }

    case PLAYING:
      handleInput();
      updatePlayer();
      updateParticles();
      updateTrail();
      drawGameScreen();
      if (!g_player.alive) {
        if (g_player.score > g_best_score) g_best_score = g_player.score;
        g_mode = GAME_OVER;
      }
      break;

    case PAUSED:
      drawGameScreen();
      if (M5.BtnPWR.wasPressed()) g_mode = PLAYING;
      break;

    case GAME_OVER:
      drawGameOver();
      if (readPressed(kPinCenter)) {
        static uint32_t last = 0;
        if (millis() - last > 300) { generateLevel(g_level); g_mode = PLAYING; last = millis(); }
      }
      if (M5.BtnPWR.wasPressed()) g_mode = MENU;
      break;
  }

  delay(16);  // ~60fps
}

#include <M5Unified.h>
#include <vector>
#include <cmath>

namespace {

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

// ─── Game Objects ──────────────────────────────────────────────
struct Player {
  float x, y;
  float vx, vy;
  int width, height;
  bool jumping;
  bool alive;
  int score;
};

struct Obstacle {
  float x;
  int type;   // 0=spike, 1=block, 2=platform
  uint16_t color;
};

struct Particle {
  float x, y;
  float vx, vy;
  uint16_t color;
  int life;
};

enum GameMode {
  MENU,
  PLAYING,
  GAME_OVER,
  PAUSED,
};

// ─── Global State ──────────────────────────────────────────────
static Player g_player = {30, 100, 0, 0, 8, 8, false, true, 0};
static std::vector<Obstacle> g_obstacles;
static std::vector<Particle> g_particles;
static GameMode g_mode = MENU;
static uint32_t g_frame = 0;
static int g_level = 1;
static float g_scroll = 0;

// ─── Constants ─────────────────────────────────────────────────
static constexpr float kGravity = 0.4f;
static constexpr float kJumpForce = -8.0f;
static constexpr int kScreenW = 240;
static constexpr int kScreenH = 135;
static constexpr int kGroundY = 100;

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void generateLevel(int level) {
  g_obstacles.clear();
  g_scroll = 0;
  g_player.x = 30;
  g_player.y = kGroundY;
  g_player.alive = true;

  // Procedural level generation
  for (int i = 0; i < 10 + level * 2; ++i) {
    float obstacleX = 100 + i * 30;
    int type = esp_random() % 3;
    uint16_t color = M5.Display.color565(
        50 + (esp_random() % 200),
        50 + (esp_random() % 200),
        50 + (esp_random() % 200)
    );
    Obstacle obs = {obstacleX, type, color};
    g_obstacles.push_back(obs);
  }
}

void updatePlayer() {
  // Gravity
  g_player.vy += kGravity;

  // Velocity
  g_player.x += g_player.vx;
  g_player.y += g_player.vy;

  // Ground collision
  if (g_player.y + g_player.height >= kGroundY) {
    g_player.y = kGroundY - g_player.height;
    g_player.vy = 0;
    g_player.jumping = false;
  }

  // Obstacle collision
  for (auto& obs : g_obstacles) {
    if (g_player.x + g_player.width > obs.x - 5 &&
        g_player.x < obs.x + 15 &&
        g_player.y + g_player.height > kGroundY - 5) {
      g_player.alive = false;
      // Create particles
      for (int i = 0; i < 5; ++i) {
        Particle p = {
            g_player.x + g_player.width / 2.0f,
            g_player.y + g_player.height / 2.0f,
            (esp_random() % 4 - 2) * 2.0f,
            (esp_random() % 4 - 2) * 2.0f,
            obs.color,
            20,
        };
        g_particles.push_back(p);
      }
    }
  }

  // Score increment
  g_player.score++;
}

void handleInput() {
  if (readPressed(kPinCenter) || readPressed(kPinUp)) {
    if (!g_player.jumping && g_player.y + g_player.height >= kGroundY) {
      g_player.vy = kJumpForce;
      g_player.jumping = true;
    }
  }

  if (readPressed(kPinDown)) {
    g_player.vy += 0.2f;  // fall faster
  }

  if (M5.BtnPWR.wasPressed()) {
    g_mode = (g_mode == PLAYING) ? PAUSED : PLAYING;
  }
}

void drawGameScreen() {
  M5.Display.fillScreen(TFT_BLACK);

  // Draw obstacles
  M5.Display.setTextColor(TFT_WHITE);
  for (auto& obs : g_obstacles) {
    if (obs.x - g_scroll > -20 && obs.x - g_scroll < kScreenW) {
      int sx = (int)(obs.x - g_scroll);
      switch (obs.type) {
        case 0:  // Spike
          M5.Display.fillTriangle(sx + 5, kGroundY, sx, kGroundY + 10, sx + 10, kGroundY + 10, obs.color);
          break;
        case 1:  // Block
          M5.Display.fillRect(sx, kGroundY, 15, 15, obs.color);
          break;
        case 2:  // Platform
          M5.Display.fillRect(sx, kGroundY - 8, 20, 5, obs.color);
          break;
      }
    }
  }

  // Draw player
  M5.Display.fillRect((int)g_player.x, (int)g_player.y, g_player.width, g_player.height, TFT_YELLOW);

  // Draw particles
  for (auto& p : g_particles) {
    M5.Display.drawPixel((int)p.x, (int)p.y, p.color);
    p.life--;
  }
  g_particles.erase(std::remove_if(g_particles.begin(), g_particles.end(),
                                   [](const Particle& p) { return p.life <= 0; }),
                    g_particles.end());

  // HUD
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("Score:%d  Lvl:%d", g_player.score, g_level);

  if (g_mode == PAUSED) {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(80, 60);
    M5.Display.print("PAUSED");
  }

  // Scroll logic
  if (g_player.x > 100) {
    g_scroll = g_player.x - 100;
  }
}

void drawMenu() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(40, 30);
  M5.Display.print("GEOMETRY");
  M5.Display.setCursor(60, 50);
  M5.Display.print("DASH");

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(50, 80);
  M5.Display.print("Level 1-5");

  M5.Display.setCursor(30, 100);
  M5.Display.print("CENTER: start  PWR: quit");
}

void drawGameOver() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setCursor(50, 40);
  M5.Display.print("GAME OVER");

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(50, 70);
  M5.Display.printf("Score: %d", g_player.score);

  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(40, 100);
  M5.Display.print("CENTER: retry");
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  Serial.begin(115200);
  delay(200);
  Serial.println("idk-geometry-dash starting...");

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

  switch (g_mode) {
    case MENU:
      drawMenu();
      if (readPressed(kPinCenter)) {
        g_mode = PLAYING;
        generateLevel(g_level);
      }
      if (M5.BtnPWR.wasPressed()) {
        ESP.restart();
      }
      break;

    case PLAYING:
      handleInput();
      updatePlayer();
      drawGameScreen();
      if (!g_player.alive) {
        g_mode = GAME_OVER;
      }
      g_frame++;
      break;

    case PAUSED:
      drawGameScreen();
      if (M5.BtnPWR.wasPressed()) {
        g_mode = PLAYING;
      }
      break;

    case GAME_OVER:
      drawGameOver();
      if (readPressed(kPinCenter)) {
        g_mode = MENU;
      }
      break;
  }

  delay(5);
}

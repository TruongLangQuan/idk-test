#include <M5Unified.h>
#include <SPIFFS.h>

namespace {

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

// ─── Game States ───────────────────────────────────────────────
enum GameMode {
  MENU,
  EXPLORING,
  COMBAT,
  INVENTORY,
  GAME_OVER,
};

struct PlayerStats {
  int level;
  int hp, maxHp;
  int xp, xpNext;
  int str, def, intel;
  int gold;
  int weaponDmg;
  int armorDef;
};

struct Enemy {
  const char* name;
  int hp, maxHp;
  int str, def;
  int xpReward;
  int goldReward;
};

// ─── Global State ──────────────────────────────────────────────
static GameMode g_mode = MENU;
static PlayerStats g_player = {
    .level = 1,
    .hp = 30,
    .maxHp = 30,
    .xp = 0,
    .xpNext = 100,
    .str = 5,
    .def = 2,
    .intel = 3,
    .gold = 0,
    .weaponDmg = 1,
    .armorDef = 0,
};

static int g_menu_idx = 0;
static int g_menu_max = 4;  // Continue, New Game, Load, Stats, Quit
static bool g_running = true;

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void drawMainMenu() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);

  M5.Display.setCursor(40, 10);
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.print("SANCTUARY RPG");

  M5.Display.setTextColor(TFT_WHITE);
  const char* items[] = {"New Game", "Continue", "Stats", "Credits", "Quit"};
  int count = 5;

  for (int i = 0; i < count; ++i) {
    int y = 40 + i * 15;
    if (i == g_menu_idx) {
      M5.Display.fillRect(0, y - 2, 240, 14, 0x0410);
      M5.Display.setTextColor(TFT_YELLOW, 0x0410);
    } else {
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    M5.Display.setCursor(20, y);
    M5.Display.print(items[i]);
  }

  M5.Display.setTextColor(TFT_DARKGREY);
  M5.Display.setCursor(4, 125);
  M5.Display.print("UP/DOWN: select  CENTER: choose");
}

void drawGameScreen() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);

  // Status bar
  M5.Display.setCursor(4, 4);
  M5.Display.printf("Lvl%d  HP:%d/%d  XP:%d/%d",
                    g_player.level, g_player.hp, g_player.maxHp,
                    g_player.xp, g_player.xpNext);

  // Game area (ASCII dungeon placeholder)
  M5.Display.setCursor(80, 40);
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.print("@");  // player character

  M5.Display.setTextColor(TFT_RED);
  M5.Display.setCursor(100, 40);
  M5.Display.print("G");  // dummy enemy

  // Stats display
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(4, 50);
  M5.Display.printf("STR:%d  DEF:%d  INT:%d", g_player.str, g_player.def, g_player.intel);

  M5.Display.setCursor(4, 70);
  M5.Display.printf("Gold: %d", g_player.gold);

  // Controls
  M5.Display.setTextColor(TFT_DARKGREY);
  M5.Display.setCursor(4, 125);
  M5.Display.print("Keys: move  PWR: menu");
}

void handleMenuInput() {
  if (readPressed(kPinUp) || readPressed(kPinLeft)) {
    static uint32_t lastUp = 0;
    uint32_t now = millis();
    if (now - lastUp > 200) {
      g_menu_idx = (g_menu_idx - 1 + g_menu_max) % g_menu_max;
      lastUp = now;
    }
  }

  if (readPressed(kPinDown) || readPressed(kPinRight)) {
    static uint32_t lastDown = 0;
    uint32_t now = millis();
    if (now - lastDown > 200) {
      g_menu_idx = (g_menu_idx + 1) % g_menu_max;
      lastDown = now;
    }
  }

  if (readPressed(kPinCenter)) {
    static uint32_t lastCenter = 0;
    uint32_t now = millis();
    if (now - lastCenter > 300) {
      switch (g_menu_idx) {
        case 0:  // New Game
          g_player = {
              .level = 1,
              .hp = 30,
              .maxHp = 30,
              .xp = 0,
              .xpNext = 100,
              .str = 5,
              .def = 2,
              .intel = 3,
              .gold = 0,
              .weaponDmg = 1,
              .armorDef = 0,
          };
          g_mode = EXPLORING;
          break;
        case 1:  // Continue
          // TODO: Load game
          g_mode = EXPLORING;
          break;
        case 2:  // Stats
          // TODO: Show stats screen
          break;
        case 3:  // Credits
          // TODO: Show credits
          break;
        case 4:  // Quit
          g_running = false;
          break;
      }
      lastCenter = now;
    }
  }

  if (M5.BtnPWR.wasPressed()) {
    // Back from menu would go here
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
  Serial.println("idk-sanctuaryrpg starting...");

  // Initialize 5-way pins
  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(10, 60);
    M5.Display.print("SPIFFS init failed!");
    delay(3000);
  }

  // TODO: Load save game if exists
}

void loop() {
  M5.update();

  switch (g_mode) {
    case MENU:
      handleMenuInput();
      drawMainMenu();
      break;
    case EXPLORING:
      drawGameScreen();
      break;
    case COMBAT:
      // TODO: Handle combat
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(50, 60);
      M5.Display.print("Combat!");
      break;
    case INVENTORY:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(50, 60);
      M5.Display.print("Inventory");
      break;
    case GAME_OVER:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_RED);
      M5.Display.setCursor(60, 60);
      M5.Display.print("GAME OVER");
      break;
  }

  delay(5);
  if (!g_running) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setCursor(50, 60);
    M5.Display.print("Goodbye!");
    delay(2000);
    ESP.restart();
  }
}

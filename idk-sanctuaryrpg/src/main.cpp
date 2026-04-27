#include <M5Unified.h>
#include <SPIFFS.h>
#include <vector>

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
  int potions;
};

struct Enemy {
  String name;
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
    .potions = 3,
};

static int g_menu_idx = 0;
static int g_menu_max = 5;  // Continue, New Game, Save, Load, Quit
static bool g_running = true;

// Map & Exploration
static char g_map[10][20];
static int g_playerX = 1;
static int g_playerY = 1;

// Combat
static Enemy g_enemy;
static int g_combat_menu_idx = 0;
static String g_combat_msg1 = "";
static String g_combat_msg2 = "";

// ─── Map Generation ────────────────────────────────────────────
void generateMap() {
  for (int y = 0; y < 10; ++y) {
    for (int x = 0; x < 20; ++x) {
      if (x == 0 || x == 19 || y == 0 || y == 9) g_map[y][x] = '#';
      else if (esp_random() % 100 < 15) g_map[y][x] = '#';
      else if (esp_random() % 100 < 5) g_map[y][x] = 'G';  // Gold
      else if (esp_random() % 100 < 3) g_map[y][x] = 'P';  // Potion
      else if (esp_random() % 100 < 10) g_map[y][x] = 'E'; // Enemy trigger
      else g_map[y][x] = '.';
    }
  }
  g_playerX = 1;
  g_playerY = 1;
  g_map[g_playerY][g_playerX] = '.';
  // Ensure exit exists
  g_map[8][18] = '>';
}

// ─── Save/Load ─────────────────────────────────────────────────
void saveGame() {
  File f = SPIFFS.open("/save.dat", FILE_WRITE);
  if (f) {
    f.write((uint8_t*)&g_player, sizeof(PlayerStats));
    f.close();
    g_combat_msg1 = "Game Saved!";
  } else {
    g_combat_msg1 = "Save Failed!";
  }
}

void loadGame() {
  File f = SPIFFS.open("/save.dat", FILE_READ);
  if (f) {
    f.read((uint8_t*)&g_player, sizeof(PlayerStats));
    f.close();
    generateMap();
    g_mode = EXPLORING;
  }
}

// ─── Combat System ─────────────────────────────────────────────
void spawnEnemy() {
  const char* names[] = {"Goblin", "Skeleton", "Orc", "Slime", "Bat"};
  int type = esp_random() % 5;
  g_enemy.name = names[type];
  int mult = g_player.level;
  g_enemy.maxHp = g_enemy.hp = 10 + esp_random() % (10 * mult);
  g_enemy.str = 2 + esp_random() % (3 * mult);
  g_enemy.def = 1 + esp_random() % (2 * mult);
  g_enemy.xpReward = 20 + esp_random() % (10 * mult);
  g_enemy.goldReward = 5 + esp_random() % (15 * mult);
}

void startCombat() {
  spawnEnemy();
  g_mode = COMBAT;
  g_combat_menu_idx = 0;
  g_combat_msg1 = "A wild " + g_enemy.name + " appears!";
  g_combat_msg2 = "";
}

void levelUp() {
  g_player.level++;
  g_player.maxHp += 10;
  g_player.hp = g_player.maxHp;
  g_player.str += 2;
  g_player.def += 1;
  g_player.xp -= g_player.xpNext;
  g_player.xpNext = (int)(g_player.xpNext * 1.5f);
  g_combat_msg2 = "LEVEL UP! Now Level " + String(g_player.level);
}

// ─── Input Helpers ─────────────────────────────────────────────
bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

// ─── Drawing Functions ─────────────────────────────────────────

void drawMainMenu() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);

  M5.Display.setCursor(40, 10);
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.print("SANCTUARY RPG");

  M5.Display.setTextColor(TFT_WHITE);
  const char* items[] = {"New Game", "Continue", "Save Game", "Load Game", "Quit"};
  
  for (int i = 0; i < g_menu_max; ++i) {
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

void drawExploring() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);

  // Status bar
  M5.Display.setCursor(4, 4);
  M5.Display.printf("Lvl%d HP:%d/%d Pts:%d Gold:%d",
                    g_player.level, g_player.hp, g_player.maxHp,
                    g_player.potions, g_player.gold);

  // Map
  M5.Display.setCursor(40, 20);
  for (int y = 0; y < 10; ++y) {
    for (int x = 0; x < 20; ++x) {
      if (x == g_playerX && y == g_playerY) {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.print('@');
      } else {
        char c = g_map[y][x];
        if (c == '#') M5.Display.setTextColor(M5.Display.color565(80,80,80));
        else if (c == 'G') M5.Display.setTextColor(TFT_YELLOW);
        else if (c == 'P') M5.Display.setTextColor(TFT_CYAN);
        else if (c == 'E') M5.Display.setTextColor(TFT_RED);
        else if (c == '>') M5.Display.setTextColor(TFT_MAGENTA);
        else M5.Display.setTextColor(M5.Display.color565(30,30,30));
        M5.Display.print(c);
      }
    }
    M5.Display.println();
    M5.Display.setCursor(40, M5.Display.getCursorY());
  }

  if (g_combat_msg1.length() > 0) {
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(4, 110);
    M5.Display.print(g_combat_msg1);
  }

  M5.Display.setTextColor(TFT_DARKGREY);
  M5.Display.setCursor(4, 125);
  M5.Display.print("Move: DPAD   Menu: PWR");
}

void drawCombat() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setCursor(140, 20);
  M5.Display.printf("%s", g_enemy.name.c_str());
  M5.Display.setCursor(140, 35);
  M5.Display.printf("HP: %d/%d", g_enemy.hp, g_enemy.maxHp);

  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setCursor(20, 20);
  M5.Display.printf("Player Lvl %d", g_player.level);
  M5.Display.setCursor(20, 35);
  M5.Display.printf("HP: %d/%d", g_player.hp, g_player.maxHp);

  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(20, 60);
  M5.Display.print(g_combat_msg1);
  M5.Display.setCursor(20, 75);
  M5.Display.print(g_combat_msg2);

  const char* opts[] = {"Attack", "Heal", "Flee"};
  for (int i = 0; i < 3; ++i) {
    M5.Display.setTextColor(i == g_combat_menu_idx ? TFT_YELLOW : TFT_WHITE);
    M5.Display.setCursor(40 + i * 60, 110);
    M5.Display.printf("%s %s", i == g_combat_menu_idx ? ">" : "", opts[i]);
  }
}

// ─── Input Handlers ────────────────────────────────────────────

void handleMenuInput() {
  static uint32_t lastInput = 0;
  uint32_t now = millis();

  if (now - lastInput < 200) return;

  if (readPressed(kPinUp) || readPressed(kPinLeft)) {
    g_menu_idx = (g_menu_idx - 1 + g_menu_max) % g_menu_max;
    lastInput = now;
  }
  if (readPressed(kPinDown) || readPressed(kPinRight)) {
    g_menu_idx = (g_menu_idx + 1) % g_menu_max;
    lastInput = now;
  }

  if (readPressed(kPinCenter)) {
    switch (g_menu_idx) {
      case 0:  // New Game
        generateMap();
        g_mode = EXPLORING;
        g_combat_msg1 = "Entered the dungeon...";
        break;
      case 1:  // Continue
        g_mode = EXPLORING;
        break;
      case 2:  // Save Game
        saveGame();
        break;
      case 3:  // Load Game
        loadGame();
        break;
      case 4:  // Quit
        g_running = false;
        break;
    }
    lastInput = now;
  }
}

void handleExploringInput() {
  static uint32_t lastInput = 0;
  uint32_t now = millis();
  if (now - lastInput < 150) return;

  int dx = 0, dy = 0;
  if (readPressed(kPinUp)) dy = -1;
  else if (readPressed(kPinDown)) dy = 1;
  else if (readPressed(kPinLeft)) dx = -1;
  else if (readPressed(kPinRight)) dx = 1;

  if (dx != 0 || dy != 0) {
    int nx = g_playerX + dx;
    int ny = g_playerY + dy;
    
    char tile = g_map[ny][nx];
    if (tile != '#') {
      g_playerX = nx;
      g_playerY = ny;
      g_combat_msg1 = "";

      if (tile == 'G') {
        int g = 5 + esp_random() % 10;
        g_player.gold += g;
        g_combat_msg1 = "Found " + String(g) + " gold!";
        g_map[ny][nx] = '.';
      } else if (tile == 'P') {
        g_player.potions++;
        g_combat_msg1 = "Found a potion!";
        g_map[ny][nx] = '.';
      } else if (tile == 'E') {
        g_map[ny][nx] = '.';
        startCombat();
      } else if (tile == '>') {
        g_combat_msg1 = "Descending deeper...";
        generateMap();
      } else if (esp_random() % 100 < 8) { // random encounter
        startCombat();
      }
    }
    lastInput = now;
  }

  if (M5.BtnPWR.wasPressed()) {
    g_mode = MENU;
  }
}

void handleCombatInput() {
  static uint32_t lastInput = 0;
  uint32_t now = millis();
  if (now - lastInput < 200) return;

  if (readPressed(kPinLeft)) {
    g_combat_menu_idx = (g_combat_menu_idx - 1 + 3) % 3;
    lastInput = now;
  }
  if (readPressed(kPinRight)) {
    g_combat_menu_idx = (g_combat_menu_idx + 1) % 3;
    lastInput = now;
  }

  if (readPressed(kPinCenter)) {
    bool enemyDead = false;
    bool playerTurnDone = false;

    if (g_combat_menu_idx == 0) { // Attack
      int dmg = (g_player.str + g_player.weaponDmg) - g_enemy.def;
      if (dmg < 1) dmg = 1;
      // crit chance
      if (esp_random() % 100 < 10) dmg *= 2; 
      g_enemy.hp -= dmg;
      g_combat_msg1 = "You hit " + g_enemy.name + " for " + String(dmg) + "!";
      playerTurnDone = true;
    } 
    else if (g_combat_menu_idx == 1) { // Heal
      if (g_player.potions > 0) {
        g_player.potions--;
        int heal = 15 + g_player.level * 5;
        g_player.hp = std::min(g_player.maxHp, g_player.hp + heal);
        g_combat_msg1 = "Healed " + String(heal) + " HP!";
        playerTurnDone = true;
      } else {
        g_combat_msg1 = "No potions left!";
      }
    }
    else if (g_combat_menu_idx == 2) { // Flee
      if (esp_random() % 100 < 50) {
        g_mode = EXPLORING;
        g_combat_msg1 = "Fled successfully!";
        lastInput = now;
        return;
      } else {
        g_combat_msg1 = "Failed to flee!";
        playerTurnDone = true;
      }
    }

    if (playerTurnDone) {
      if (g_enemy.hp <= 0) {
        g_enemy.hp = 0;
        g_player.xp += g_enemy.xpReward;
        g_player.gold += g_enemy.goldReward;
        g_combat_msg2 = "Victory! +" + String(g_enemy.xpReward) + "XP, +" + String(g_enemy.goldReward) + "G";
        if (g_player.xp >= g_player.xpNext) {
          levelUp();
        }
        g_mode = EXPLORING; // return on next update implicitly
      } else {
        int eDmg = g_enemy.str - g_player.def - g_player.armorDef;
        if (eDmg < 1) eDmg = 1;
        g_player.hp -= eDmg;
        g_combat_msg2 = g_enemy.name + " hits you for " + String(eDmg) + "!";
        if (g_player.hp <= 0) {
          g_player.hp = 0;
          g_mode = GAME_OVER;
        }
      }
    }
    lastInput = now;
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

  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);

  if (!SPIFFS.begin(true)) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(10, 60);
    M5.Display.print("SPIFFS init failed!");
    delay(3000);
  }
}

void loop() {
  M5.update();

  switch (g_mode) {
    case MENU:
      handleMenuInput();
      drawMainMenu();
      break;
    case EXPLORING:
      handleExploringInput();
      drawExploring();
      break;
    case COMBAT:
      handleCombatInput();
      drawCombat();
      break;
    case INVENTORY:
      // Simplified out for now
      g_mode = EXPLORING;
      break;
    case GAME_OVER:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_RED);
      M5.Display.setTextSize(2);
      M5.Display.setCursor(60, 60);
      M5.Display.print("GAME OVER");
      M5.Display.setTextSize(1);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(40, 100);
      M5.Display.print("PWR to return to menu");
      if (M5.BtnPWR.wasPressed()) g_mode = MENU;
      break;
  }

  delay(20);
  if (!g_running) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setCursor(50, 60);
    M5.Display.print("Goodbye!");
    delay(2000);
    ESP.restart();
  }
}

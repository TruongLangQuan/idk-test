#include <M5Unified.h>

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

// ─── Game States ───────────────────────────────────────────────
enum GameMode {
    MENU,       // Select Class
    STORY_DISPLAY, // Narrative & Choices
    GAME_OVER
};

struct PlayerStats {
    String pClass;
    int level;
    int hp, maxHp;
    int xp, xpNext;
    int gold;
    int potions;
};

// ─── Global State ──────────────────────────────────────────────
static GameMode g_mode = MENU;
static PlayerStats g_player = {
    .pClass = "Warrior",
    .level = 1,
    .hp = 40,
    .maxHp = 40,
    .xp = 0,
    .xpNext = 100,
    .gold = 10,
    .potions = 3
};

// UI & Display
static M5Canvas canvas(&M5.Display);
static int g_selected_choice = 0; // 0, 1, 2 for Choice 1, 2, 3
static bool g_autoplay = false;
static uint32_t g_autoplay_timer = 0;
static constexpr uint32_t AUTOPLAY_DELAY_MS = 3000;

// Cyberpunk Palette
static constexpr uint16_t COLOR_CYBER_BG = 0x0802;   // Deep dark indigo/purple
static constexpr uint16_t COLOR_CYBER_FG = 0x07FF;   // Neon Cyan
static constexpr uint16_t COLOR_ALERT_FG = 0xF81F;   // Neon Magenta
static constexpr uint16_t COLOR_SUCCESS_FG = 0x07E0; // Matrix Green
static constexpr uint16_t COLOR_GOLD_FG = 0xFDE0;    // Neon Gold

// AI Story Data
static String g_story_text = "";
static String g_choice1 = "";
static String g_choice2 = "";
static String g_choice3 = "";
static int g_best_choice = 1;
static String g_monster_name = "";
static int g_gold_reward = 0;
static int g_xp_reward = 0;
static int g_hp_change = 0;

static String g_previous_story = "The hero stands brave before the dark dungeon gates.";
static String g_selected_choice_text = "Enter the dungeon.";
static int g_last_choice_idx = 0; // index chosen in the previous scene

// ─── Offline Story Engine (no network) ─────────────────────────
static const char* kMonsters[] = {"Goblin", "Skeleton", "Orc", "Slime", "Dragon"};

static const char* kOutcomeGood[] = {
    "Your move succeeds brilliantly!",
    "Fortune favors you this time.",
    "You pull it off without a scratch.",
    "Well executed - the way is clear."
};
static const char* kOutcomeBad[] = {
    "It goes badly - pain flares up.",
    "You misjudge and take a hit.",
    "Danger catches you off guard.",
    "A costly mistake in the dark."
};

static String pickLine(const char** table, int n) {
    return String(table[random(n)]);
}

// Resolves the outcome of the previously chosen action, then rolls
// a fresh dungeon situation with 3 choices.
void generateStory() {
    bool success = (g_last_choice_idx + 1) == g_best_choice;

    if (success) {
        g_story_text = pickLine(kOutcomeGood, 4);
        g_gold_reward = random(2, 9);
        g_xp_reward = random(8, 20);
        g_hp_change = random(-1, 4);
    } else {
        g_story_text = pickLine(kOutcomeBad, 4);
        g_gold_reward = random(0, 3);
        g_xp_reward = random(3, 10);
        g_hp_change = -random(2, 7);
    }

    int roll = random(100);
    g_monster_name = "";
    if (roll < 55) {
        g_monster_name = kMonsters[random(5)];
        if (g_monster_name == "Dragon") g_hp_change -= random(0, 4); // dragons bite
        g_story_text += " A " + g_monster_name + " blocks the passage!";
        g_choice1 = "Strike the " + g_monster_name;
        g_choice2 = "Feint and counter";
        g_choice3 = "Retreat to safety";
        g_best_choice = 2;
    } else if (roll < 70) {
        g_story_text += " A dusty chest sits half-buried here.";
        g_choice1 = "Force the lock";
        g_choice2 = "Inspect for traps";
        g_choice3 = "Smash it open";
        g_best_choice = 2;
    } else if (roll < 82) {
        g_story_text += " Cracked flagstones hint at buried spikes.";
        g_choice1 = "Dash across quickly";
        g_choice2 = "Probe each stone first";
        g_choice3 = "Walk the outer edge";
        g_best_choice = 2;
    } else if (roll < 92) {
        g_story_text += " An old stairwell spirals down into black.";
        g_choice1 = "Descend cautiously";
        g_choice2 = "Light a torch first";
        g_choice3 = "Listen at the top";
        g_best_choice = 2;
    } else {
        g_story_text += " A silent shrine hums with faint power.";
        g_choice1 = "Kneel and pray";
        g_choice2 = "Touch the altar";
        g_choice3 = "Loot the offering bowl";
        g_best_choice = 1;
    }
}

// Generates the next scene, applies its effects to the player,
// and transitions to STORY_DISPLAY or GAME_OVER.
void advanceStory() {
    generateStory();

    // Limit choice lengths to prevent UI overflowing
    if (g_choice1.length() > 38) g_choice1 = g_choice1.substring(0, 35) + "...";
    if (g_choice2.length() > 38) g_choice2 = g_choice2.substring(0, 35) + "...";
    if (g_choice3.length() > 38) g_choice3 = g_choice3.substring(0, 35) + "...";

    // Apply results
    g_player.gold += g_gold_reward;
    g_player.xp += g_xp_reward;
    g_player.hp = constrain(g_player.hp + g_hp_change, 0, g_player.maxHp);

    // Level Up checks
    if (g_player.xp >= g_player.xpNext) {
        g_player.level++;
        g_player.xp -= g_player.xpNext;
        g_player.xpNext = (int)(g_player.xpNext * 1.5f);
        g_player.maxHp += 10;
        g_player.hp = g_player.maxHp;
    }

    g_previous_story = g_story_text;

    if (g_player.hp <= 0) {
        g_mode = GameMode::GAME_OVER;
    } else {
        g_selected_choice = 0;
        g_mode = GameMode::STORY_DISPLAY;
        g_autoplay_timer = millis(); // Reset autoplay timer
    }
}

// ─── Class and Monster ASCII Portraits ─────────────────────────
void drawAsciiPortrait(const String& target, int x, int y, uint16_t color) {
    canvas.setTextColor(color, COLOR_CYBER_BG);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
    
    if (target.equalsIgnoreCase("Goblin")) {
        canvas.drawCenterString(" /\\_/\\ ", x + 42, y + 25);
        canvas.drawCenterString("( o.o )", x + 42, y + 37);
        canvas.drawCenterString(" > ^ < ", x + 42, y + 49);
        canvas.drawCenterString("/  |  \\", x + 42, y + 61);
    } 
    else if (target.equalsIgnoreCase("Skeleton")) {
        canvas.drawCenterString(" .---. ", x + 42, y + 25);
        canvas.drawCenterString("| o o |", x + 42, y + 37);
        canvas.drawCenterString(" \\ - / ", x + 42, y + 49);
        canvas.drawCenterString("  | |  ", x + 42, y + 61);
        canvas.drawCenterString(" -^-^- ", x + 42, y + 73);
    } 
    else if (target.equalsIgnoreCase("Orc")) {
        canvas.drawCenterString(" (\\_/) ", x + 42, y + 25);
        canvas.drawCenterString("( O O )", x + 42, y + 37);
        canvas.drawCenterString("/|   |\\", x + 42, y + 49);
        canvas.drawCenterString("\\|___|/", x + 42, y + 61);
    } 
    else if (target.equalsIgnoreCase("Slime")) {
        canvas.drawCenterString("  ___  ", x + 42, y + 31);
        canvas.drawCenterString(" (o o) ", x + 42, y + 43);
        canvas.drawCenterString("(_____)", x + 42, y + 55);
    } 
    else if (target.equalsIgnoreCase("Dragon")) {
        canvas.drawCenterString(" /\\_/\\ ", x + 42, y + 15);
        canvas.drawCenterString("(=o_o=)", x + 42, y + 27);
        canvas.drawCenterString("/ \\_/ \\", x + 42, y + 39);
        canvas.drawCenterString("/     \\", x + 42, y + 51);
        canvas.drawCenterString("/|| ||\\", x + 42, y + 63);
    } 
    else if (g_player.pClass == "Warrior") {
        // Player Warrior Class
        canvas.drawCenterString("  /|   ", x + 42, y + 20);
        canvas.drawCenterString(" | |   ", x + 42, y + 32);
        canvas.drawCenterString(" | |   ", x + 42, y + 44);
        canvas.drawCenterString(" /_|_\\ ", x + 42, y + 56);
        canvas.drawCenterString(" |===| ", x + 42, y + 68);
        canvas.drawCenterString("  \\_/  ", x + 42, y + 80);
    } 
    else {
        // Player Mage Class
        canvas.drawCenterString("  /\\   ", x + 42, y + 20);
        canvas.drawCenterString(" /  \\  ", x + 42, y + 32);
        canvas.drawCenterString(" | o | ", x + 42, y + 44);
        canvas.drawCenterString(" \\   / ", x + 42, y + 56);
        canvas.drawCenterString("  | |  ", x + 42, y + 68);
        canvas.drawCenterString("  ( )  ", x + 42, y + 80);
    }
}

// ─── Input Press Helpers ───────────────────────────────────────
bool isPressed(int pin) {
    return digitalRead(pin) == LOW;
}

// ─── Draw Functions ────────────────────────────────────────────

void drawHeader() {
    canvas.fillRect(0, 0, 240, 14, 0x1803); // Translucent Dark Bar
    canvas.drawFastHLine(0, 14, 240, COLOR_CYBER_FG);
    
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
    canvas.setTextColor(0xFFFF);
    canvas.drawString(g_player.pClass + " Lvl " + String(g_player.level), 6, 3);
    
    // Health bar representation
    canvas.drawString("HP:" + String(g_player.hp) + "/" + String(g_player.maxHp), 95, 3);
    canvas.drawString("G:" + String(g_player.gold), 170, 3);
    
    if (g_autoplay) {
        canvas.setTextColor(COLOR_SUCCESS_FG);
        canvas.drawRightString("[AUTO]", 234, 3);
    } else {
        canvas.setTextColor(COLOR_ALERT_FG);
        canvas.drawRightString("[MANUAL]", 234, 3);
    }
}

void drawWidescreenCard() {
    int card_x = 150;
    int card_y = 18;
    int card_w = 84;
    int card_h = 112;
    
    // Draw glowing border
    canvas.drawRoundRect(card_x, card_y, card_w, card_h, 4, COLOR_ALERT_FG);
    canvas.drawRoundRect(card_x + 1, card_y + 1, card_w - 2, card_h - 2, 4, 0x4108); // Dark glow effect
    
    // Label above ASCII inside card
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
    canvas.setTextColor(COLOR_CYBER_FG);
    
    String label = g_monster_name.isEmpty() ? g_player.pClass : g_monster_name;
    if (label.length() > 10) label = label.substring(0, 9) + ".";
    canvas.drawCenterString(label, card_x + 42, card_y + 6);
    canvas.drawFastHLine(card_x + 6, card_y + 15, card_w - 12, COLOR_CYBER_FG);
    
    // Dynamic ASCII
    uint16_t art_color = g_monster_name.isEmpty() ? COLOR_CYBER_FG : COLOR_SUCCESS_FG;
    drawAsciiPortrait(g_monster_name.isEmpty() ? g_player.pClass : g_monster_name, card_x, card_y, art_color);
}

void drawMainMenu() {
    canvas.fillSprite(COLOR_CYBER_BG);
    
    canvas.setTextColor(COLOR_ALERT_FG);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(2);
    canvas.drawCenterString("SANCTUARY RPG", 120, 15);
    
    canvas.setTextSize(1);
    canvas.setTextColor(0xFFFF);
    canvas.drawCenterString("SELECT CHARACTER CLASS:", 120, 50);
    
    // Options
    const char* items[] = {"Warrior", "Mage"};
    for (int i = 0; i < 2; ++i) {
        int y = 75 + i * 20;
        if (i == g_selected_choice) {
            canvas.fillRect(20, y - 2, 200, 14, 0x1803);
            canvas.drawRect(20, y - 2, 200, 14, COLOR_CYBER_FG);
            canvas.setTextColor(COLOR_SUCCESS_FG);
            canvas.drawCenterString("> " + String(items[i]) + " <", 120, y);
        } else {
            canvas.setTextColor(0xFFFF);
            canvas.drawCenterString(items[i], 120, y);
        }
    }
    
    canvas.setTextColor(COLOR_CYBER_FG);
    canvas.drawCenterString("A/D-Pad: Select  Center: Choose", 120, 120);
    canvas.pushSprite(0, 0);
}

void drawStoryDisplay() {
    canvas.fillSprite(COLOR_CYBER_BG);
    drawHeader();
    
    // Draw Widescreen graphic card
    drawWidescreenCard();
    
    // Draw Story Narration (Wrap text cleanly)
    canvas.setTextColor(COLOR_CYBER_FG);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
    
    // Custom wrap string routine to avoid breaking words
    int text_x = 6;
    int text_y = 18;
    int max_width = 138;
    
    // We parse g_story_text into lines to draw
    String remaining = g_story_text;
    int line_h = 8;
    int current_y = text_y;
    
    while (remaining.length() > 0 && current_y < 70) {
        int char_count = canvas.textWidth(remaining) > max_width ? 26 : remaining.length();
        if (char_count < (int)remaining.length()) {
            // Backtrack to last space
            int last_space = remaining.lastIndexOf(' ', char_count);
            if (last_space > 0) char_count = last_space;
        }
        canvas.drawString(remaining.substring(0, char_count), text_x, current_y);
        remaining = remaining.substring(char_count);
        remaining.trim();
        current_y += line_h;
    }
    
    // Draw choices
    const String choices[] = { g_choice1, g_choice2, g_choice3 };
    int choice_y_start = 75;
    
    for (int i = 0; i < 3; ++i) {
        int y = choice_y_start + i * 14;
        if (i == g_selected_choice) {
            // Draw glowing outline selector
            canvas.fillRect(4, y - 2, 140, 13, 0x1803);
            canvas.drawRect(4, y - 2, 140, 13, COLOR_ALERT_FG);
            canvas.setTextColor(COLOR_SUCCESS_FG);
            canvas.drawString("> " + choices[i], 6, y);
        } else {
            canvas.setTextColor(0xFFFF);
            canvas.drawString(String(i+1) + ". " + choices[i], 6, y);
        }
    }
    
    // Autoplay countdown bar representation
    if (g_autoplay) {
        uint32_t elapsed = millis() - g_autoplay_timer;
        if (elapsed < AUTOPLAY_DELAY_MS) {
            int progress_w = (elapsed * 138) / AUTOPLAY_DELAY_MS;
            canvas.fillRect(4, 120, progress_w, 2, COLOR_SUCCESS_FG);
        }
    }
    
    canvas.pushSprite(0, 0);
}

void drawGameOver() {
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextColor(COLOR_ALERT_FG);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(2);
    canvas.drawCenterString("GAME OVER", 120, 30);
    
    canvas.setTextSize(1);
    canvas.setTextColor(0xFFFF);
    canvas.drawCenterString("You have perished in the dungeon!", 120, 65);
    
    canvas.setTextColor(COLOR_CYBER_FG);
    canvas.drawCenterString("Center/Button A: Return to Menu", 120, 95);
    canvas.pushSprite(0, 0);
}

// ─── Input Handlers ────────────────────────────────────────────

void handleMenuInput() {
    static uint32_t lastInput = 0;
    uint32_t now = millis();
    if (now - lastInput < 200) return;
    
    if (isPressed(kPinUp) || isPressed(kPinLeft)) {
        g_selected_choice = (g_selected_choice - 1 + 2) % 2;
        lastInput = now;
    }
    if (isPressed(kPinDown) || isPressed(kPinRight)) {
        g_selected_choice = (g_selected_choice + 1) % 2;
        lastInput = now;
    }
    
    if (isPressed(kPinCenter) || M5.BtnA.wasPressed()) {
        g_player.pClass = (g_selected_choice == 0) ? "Warrior" : "Mage";
        g_player.level = 1;
        g_player.hp = (g_selected_choice == 0) ? 50 : 35;
        g_player.maxHp = g_player.hp;
        g_player.gold = 10;
        g_player.xp = 0;
        g_player.xpNext = 100;
        
        g_previous_story = "The adventurer steps bravely into the eerie, dark dungeon.";
        g_selected_choice_text = "Cross the massive iron gate.";
        g_last_choice_idx = 0; // opening scene always resolves well

        advanceStory();
        lastInput = now;
    }
}

void handleStoryInput() {
    static uint32_t lastInput = 0;
    uint32_t now = millis();
    
    // Autoplay execution
    if (g_autoplay) {
        if (now - g_autoplay_timer >= AUTOPLAY_DELAY_MS) {
            // Autoplay selects the absolute best choice
            g_selected_choice = g_best_choice - 1; // 1-based index to 0-based

            // Get selected choice text
            if (g_selected_choice == 0) g_selected_choice_text = g_choice1;
            else if (g_selected_choice == 1) g_selected_choice_text = g_choice2;
            else g_selected_choice_text = g_choice3;
            g_last_choice_idx = g_selected_choice;

            advanceStory();
            g_autoplay_timer = now;
        }
        return;
    }
    
    if (now - lastInput < 200) return;
    
    if (isPressed(kPinUp) || isPressed(kPinLeft)) {
        g_selected_choice = (g_selected_choice - 1 + 3) % 3;
        lastInput = now;
    }
    if (isPressed(kPinDown) || isPressed(kPinRight)) {
        g_selected_choice = (g_selected_choice + 1) % 3;
        lastInput = now;
    }
    
    if (isPressed(kPinCenter) || M5.BtnA.wasPressed()) {
        // Apply choice
        if (g_selected_choice == 0) g_selected_choice_text = g_choice1;
        else if (g_selected_choice == 1) g_selected_choice_text = g_choice2;
        else g_selected_choice_text = g_choice3;
        g_last_choice_idx = g_selected_choice;

        advanceStory();
        lastInput = now;
    }
}

void handleGameOverInput() {
    static uint32_t lastInput = 0;
    uint32_t now = millis();
    if (now - lastInput < 200) return;
    
    if (isPressed(kPinCenter) || M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
        g_mode = GameMode::MENU;
        g_selected_choice = 0;
        lastInput = now;
    }
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    cfg.internal_imu = false;
    cfg.internal_rtc = false;
    cfg.internal_mic = false;
    cfg.internal_spk = false;
    M5.begin(cfg);
    
    M5.Display.setBaseColor(TFT_BLACK);
    M5.Display.setRotation(1); // 240x135 widescreen landscape
    M5.Display.setBrightness(150);
    
    canvas.setPsram(true);
    canvas.setColorDepth(8);
    canvas.createSprite(240, 135);
    
    pinMode(kPinUp, INPUT_PULLUP);
    pinMode(kPinDown, INPUT_PULLUP);
    pinMode(kPinLeft, INPUT_PULLUP);
    pinMode(kPinRight, INPUT_PULLUP);
    pinMode(kPinCenter, INPUT_PULLUP);
    
    Serial.begin(115200);
    randomSeed(esp_random());
}

void loop() {
    M5.update();
    
    // Tapping Button B toggles Autoplay mode on/off
    if (M5.BtnB.wasPressed()) {
        g_autoplay = !g_autoplay;
        g_autoplay_timer = millis(); // Reset timer on toggle
    }
    
    switch (g_mode) {
        case MENU:
            handleMenuInput();
            drawMainMenu();
            break;

        case STORY_DISPLAY:
            handleStoryInput();
            drawStoryDisplay();
            break;

        case GAME_OVER:
            handleGameOverInput();
            drawGameOver();
            break;
    }
    
    delay(10);
}

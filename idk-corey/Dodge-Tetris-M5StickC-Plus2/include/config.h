// config.h - M5StickC Plus2 Game System Configuration
#ifndef CONFIG_H
#define CONFIG_H

#include <M5StickCPlus2.h>

// Display Settings for M5StickC Plus2
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
#define SCREEN_ROTATION 1  // Landscape mode

// Game Constants
#define MAX_GAMES 2
#define BUTTON_DELAY 150

// Game IDs
enum GameID {
  GAME_DODGE = 0,
  GAME_TETRIS = 1
};

// Game States
enum GameState {
  STATE_MENU,
  STATE_PLAYING,
  STATE_GAME_OVER,
  STATE_GAME_WON
};

// Colors (565 RGB format for M5StickC Plus2)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_ORANGE    0xFD20
#define COLOR_PURPLE    0x780F
#define COLOR_CYAN      0x07FF

#endif

#pragma once
// config.h - Hardware and game constants for M5StickC Plus2

// ============================================================================
// Display
// ============================================================================
static const int DISPLAY_WIDTH  = 240;
static const int DISPLAY_HEIGHT = 135;

// Status bar at top of screen (shows icons for food/energy/mood)
static const int STATUS_BAR_HEIGHT = 15;

// Play area below status bar
static const int PLAY_WIDTH  = DISPLAY_WIDTH;
static const int PLAY_HEIGHT = DISPLAY_HEIGHT - STATUS_BAR_HEIGHT;
static const int PLAY_Y      = STATUS_BAR_HEIGHT;

// Sprite render scale: original 128x64 sprites drawn at 2x
static const int SPRITE_SCALE = 2;

// ============================================================================
// Game loop
// ============================================================================
static const int   FPS            = 12;
static const int   FRAME_TIME_MS  = 1000 / FPS;

// ============================================================================
// Camera / panning
// ============================================================================
static const int   PAN_SPEED      = 2;  // pixels per frame (at 1x scale)

// ============================================================================
// Transition defaults
// ============================================================================
// 'fade', 'wipe', or 'iris'
#define TRANSITION_TYPE      "fade"
static const float TRANSITION_DURATION = 0.4f; // seconds per half-transition

// ============================================================================
// Button timing thresholds (milliseconds)
// ============================================================================
static const uint32_t BTN_DEBOUNCE_MS    = 50;
static const uint32_t BTN_SHORT_MAX_MS   = 350;  // up to 350 ms = short press
static const uint32_t BTN_MEDIUM_MIN_MS  = 350;  // 350–600 ms = medium (select)
static const uint32_t BTN_MEDIUM_MAX_MS  = 600;
static const uint32_t BTN_LONG_MIN_MS    = 600;  // 600+ ms = long press
static const uint32_t BTN_DOUBLE_WINDOW_MS = 400; // max gap between two presses

// ============================================================================
// Colors (RGB565)
// ============================================================================
static const uint16_t COLOR_BLACK      = 0x0000;
static const uint16_t COLOR_WHITE      = 0xFFFF;
static const uint16_t COLOR_CREAM      = 0xFFE0; // cat fill color
static const uint16_t COLOR_DARK_GRAY  = 0x4208;
static const uint16_t COLOR_MED_GRAY   = 0x8410;
static const uint16_t COLOR_LIGHT_GRAY = 0xC618;
static const uint16_t COLOR_RED        = 0xF800;
static const uint16_t COLOR_GREEN      = 0x07E0;
static const uint16_t COLOR_BLUE       = 0x001F;
static const uint16_t COLOR_YELLOW     = 0xFFE0;
static const uint16_t COLOR_ORANGE     = 0xFD20;
static const uint16_t COLOR_CYAN       = 0x07FF;
static const uint16_t COLOR_MAGENTA    = 0xF81F;
static const uint16_t COLOR_NAVY       = 0x000F;
static const uint16_t COLOR_SKY_DAY    = 0x867F; // light blue sky
static const uint16_t COLOR_SKY_NIGHT  = 0x000F; // deep navy

// UI colors
static const uint16_t COLOR_UI_BG      = 0x1082; // dark panel background
static const uint16_t COLOR_UI_BORDER  = 0x4208;
static const uint16_t COLOR_UI_SELECT  = 0x02DF; // highlighted item
static const uint16_t COLOR_UI_TEXT    = 0xFFFF;
static const uint16_t COLOR_UI_DIM     = 0x8410;

// Stat bar colors
static const uint16_t COLOR_BAR_HIGH   = 0x07E0; // green
static const uint16_t COLOR_BAR_MED    = 0xFFE0; // yellow
static const uint16_t COLOR_BAR_LOW    = 0xF800; // red

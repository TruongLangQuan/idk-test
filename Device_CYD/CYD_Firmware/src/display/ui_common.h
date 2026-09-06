#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// Colors
#define COLOR_BG 0x0000
#define COLOR_TEXT 0xFFFF
#define COLOR_ACCENT 0x07FF
#define COLOR_HIGHLIGHT 0x3200

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

extern int ui_rotation;

void ui_init();
void ui_clear();
void ui_draw_status_bar();

#endif // UI_COMMON_H
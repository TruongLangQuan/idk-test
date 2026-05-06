#include "screen3.h"
#include "ui_common.h"
#include "../media/screensaver.h"
#include "../media/ascii_art.h"
#include "../media/gif_player.h"

static int current_mode = 0;
#define MAX_MODES 4

void screen3_init() {
    ui_clear();
    current_mode = 0;
    screen3_draw();
}

void screen3_draw() {
    ui_clear();
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.drawString("gif playing/ascii(2d or 3d) playing/screensaver", 22, 112);
    
    if (current_mode == 0) {
        gif_player_init();
    } else if (current_mode == 1) {
        ascii_art_init(0); // 2D
    } else if (current_mode == 2) {
        ascii_art_init(1); // 3D
    } else if (current_mode == 3) {
        screensaver_init();
    }
}

void screen3_update() {
    if (current_mode == 0) {
        gif_player_update();
    } else if (current_mode == 1 || current_mode == 2) {
        ascii_art_update();
    } else if (current_mode == 3) {
        screensaver_update();
    }
    
    // Draw mode dots
    for(int i=0; i<MAX_MODES; i++) {
        if(i == current_mode) tft.fillCircle(160 - 15 + i*10, 230, 3, COLOR_TEXT);
        else tft.drawCircle(160 - 15 + i*10, 230, 3, COLOR_TEXT);
    }
}

void screen3_next_mode() {
    current_mode = (current_mode + 1) % MAX_MODES;
    screen3_draw();
}
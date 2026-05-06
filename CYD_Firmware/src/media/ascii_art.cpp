#include "ascii_art.h"
#include "../display/ui_common.h"
#include <math.h>

static int art_mode = 0;
static float theta = 0;

void ascii_art_init(int mode) {
    art_mode = mode;
    ui_clear();
}

void ascii_art_update() {
    if (art_mode == 0) {
        // 2D Ascii
        tft.setTextColor(COLOR_ACCENT, COLOR_BG);
        tft.setCursor(20, 100);
        tft.setTextSize(2);
        tft.print("2D ASCII Art");
    } else {
        // 3D Donut/Cube mock
        tft.fillScreen(COLOR_BG);
        theta += 0.1;
        int x = 160 + sin(theta) * 50;
        int y = 120 + cos(theta) * 50;
        tft.drawRect(x, y, 20, 20, COLOR_TEXT);
        delay(50); // Small delay just for visualization
    }
}
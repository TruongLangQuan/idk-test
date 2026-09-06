#include "screensaver.h"
#include "../display/ui_common.h"

#define MAX_COLS 32
static int column_y[MAX_COLS];
static uint16_t matrix_colors[] = {0x07E0, 0x05E0, 0x03E0, 0x02E0}; // Different greens

void screensaver_init() {
    ui_clear();
    for(int i=0; i<MAX_COLS; i++) {
        column_y[i] = random(-240, 0);
    }
}

void screensaver_update() {
    // Dim the screen slightly for trail effect (simulated)
    // Real fading is slow on TFT, so we just draw new chars
    
    for(int i=0; i<MAX_COLS; i++) {
        int x = i * 10;
        
        // Draw head
        tft.setTextColor(0xFFFF, COLOR_BG); // White head
        tft.setCursor(x, column_y[i]);
        tft.print((char)random(33, 126));
        
        // Draw tail
        if (column_y[i] >= 10) {
            tft.setTextColor(matrix_colors[random(0, 4)], COLOR_BG);
            tft.setCursor(x, column_y[i] - 10);
            tft.print((char)random(33, 126));
        }
        
        column_y[i] += 10;
        if (column_y[i] > 240) {
            column_y[i] = 0;
            // Clear column
            tft.fillRect(x, 0, 10, 240, COLOR_BG);
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(50));
}
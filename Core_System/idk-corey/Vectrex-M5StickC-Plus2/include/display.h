#ifndef DISPLAY_H
#define DISPLAY_H

#include <M5StickCPlus2.h>
#include <stdint.h>

// Display dimensions
#define LCD_WIDTH  240
#define LCD_HEIGHT 135

// Rotation state (0, 1, 2, 3 for 4 directions)
extern uint8_t screenRotation;  // 0=portrait, 1=landscape, 2=portrait180, 3=landscape180

// Colors (RGB565 format)
#define VECTOR_COLOR     0x07E0  // Pure green (authentic Vectrex)
#define BACKGROUND_COLOR 0x0000  // Black
#define DIM_VECTOR       0x0320  // Dimmed green (for persistence)

// Framebuffer (240×135 pixels, RGB565 = 2 bytes/pixel = 64,800 bytes)
// Allocated in PSRAM for better memory management
extern uint16_t *framebuffer;

// Dirty rectangle tracking for partial screen updates
extern int dirty_minX, dirty_maxX, dirty_minY, dirty_maxY;
extern bool dirty_flag;

// Initialize display system
void displayInit();

// Clear framebuffer to black
void displayClear();

// Draw pixel into framebuffer (with bounds checking)
void displayDrawPixel(int x, int y, uint16_t color);

// Present framebuffer to LCD (call once per frame)
void displayPresent();

// Toggle screen rotation (landscape <-> portrait)
void displayToggleRotation();

// Coordinate conversion (Vectrex oscilloscope → LCD)
// Vectrex coordinates: 0 to 33000 (X), 0 to 41000 (Y)
int vectrexToLcdX(int vecX);
int vectrexToLcdY(int vecY);

#endif

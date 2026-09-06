#include "display.h"
#include <esp_heap_caps.h>

// Allocate framebuffer in PSRAM (64,800 bytes)
uint16_t *framebuffer = nullptr;

// Dirty rectangle tracking for partial updates
int dirty_minX = LCD_WIDTH;
int dirty_maxX = 0;
int dirty_minY = LCD_HEIGHT;
int dirty_maxY = 0;
bool dirty_flag = false;

// Rotation state (0, 1, 2, 3 for 4 directions)
uint8_t screenRotation = 3;  // Default to landscape 3 (inverted)

void displayInit() {
    #ifdef use_lib_log_serial
    Serial.printf("\n=== Memory Info ===\n");
    Serial.printf("Total heap: %d bytes\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.printf("Framebuffer size: %d bytes\n", LCD_WIDTH * LCD_HEIGHT * 2);
    #endif
    
    // Allocate framebuffer in PSRAM
    framebuffer = (uint16_t*)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * 2, MALLOC_CAP_SPIRAM);
    if (!framebuffer) {
        #ifdef use_lib_log_serial
        Serial.println("WARNING: Failed to allocate in PSRAM, trying RAM...");
        #endif
        framebuffer = (uint16_t*)malloc(LCD_WIDTH * LCD_HEIGHT * 2);
        if (!framebuffer) {
            #ifdef use_lib_log_serial
            Serial.println("ERROR: Failed to allocate framebuffer!");
            #endif
            return;
        }
    }
    
    #ifdef use_lib_log_serial
    Serial.printf("SUCCESS: Framebuffer allocated at 0x%08X\n", (uint32_t)framebuffer);
    Serial.printf("Free PSRAM after allocation: %d bytes\n", ESP.getFreePsram());
    #endif
    
    displayClear();
}

void displayClear() {
    if (framebuffer) {
        memset(framebuffer, 0, LCD_WIDTH * LCD_HEIGHT * 2);
    }
}

void displayDrawPixel(int x, int y, uint16_t color) {
    if (framebuffer && x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT) {
        framebuffer[y * LCD_WIDTH + x] = color;
    }
}

void displayPresent() {
    if (framebuffer) {
        // Always push as 240×135 - LCD rotation setting handles the rest
        M5.Lcd.pushImage(0, 0, LCD_WIDTH, LCD_HEIGHT, framebuffer);
    }
}

void displayToggleRotation() {
    // Toggle between landscape (1) and landscape-180 (3) only
    screenRotation = (screenRotation == 1) ? 3 : 1;
    M5.Lcd.setRotation(screenRotation);
    M5.Lcd.fillScreen(BLACK);  // Clear LCD screen completely
    displayClear();  // Clear framebuffer
    #ifdef use_lib_log_serial
    const char* rotNames[] = {"portrait", "landscape", "portrait180", "landscape180"};
    Serial.printf("Screen rotation: %s (%d)\n", rotNames[screenRotation], screenRotation);
    #endif
}

// Vectrex oscilloscope coordinates (from FastDac.h):
// _ALG_MAX_X = 33000
// _ALG_MAX_Y = 41000
int vectrexToLcdX(int vecX) {
    // Always map to framebuffer width (240) - LCD rotation handles orientation
    return (vecX * LCD_WIDTH) / 33000;
}

int vectrexToLcdY(int vecY) {
    // Always map to framebuffer height (135) - LCD rotation handles orientation
    return (vecY * LCD_HEIGHT) / 41000;
}

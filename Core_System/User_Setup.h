// User_Setup.h for TFT_eSPI
// TenStar TS-ESP32-S3 + 1.14" TFT (ST7789) 240x135 Landscape
// idk-ts-desktop Firmware - High-Contrast Monochrome Theme

#ifndef USER_SETUP_H
#define USER_SETUP_H

#define USER_SETUP_LOADED

// ============================================
// HARDWARE CONFIGURATION
// ============================================

// Use HSPI port (FSPI is used by native PSRAM on ESP32-S3)
#define USE_HSPI_PORT

// ST7789 Driver
#define ST7789_DRIVER

// Display Resolution - Landscape orientation
#define TFT_WIDTH  240
#define TFT_HEIGHT 135

// Color Order (BGR for ST7789)
#define TFT_RGB_ORDER TFT_BGR

// CGRAM Offset for ST7789 1.14" display
#define CGRAM_OFFSET

// Inversion ON - required for ST7789 to show black background
#define TFT_INVERSION_ON

// ============================================
// FONT CONFIGURATION
// ============================================

// GLCD Font (monospace ASCII - retro terminal look)
#define LOAD_GLCD
#define LOAD_FONT2

// ============================================
// SPI CONFIGURATION
// ============================================

#define SPI_FREQUENCY 27000000
#define SPI_READ_FREQUENCY 20000000

// ============================================
// TFT PIN MAPPING
// ============================================
// | Function | TS-ESP32-S3 Pin |
// |---------|---------------|
// | TFT_CS  | GPIO 6       |
// | TFT_DC  | GPIO 7       |
// | TFT_RST | GPIO 8       |
// | TFT_BL  | GPIO 9       |

#define TFT_MISO -1          // Not used
#define TFT_MOSI 35          // Shared (but we use manual SPI for SD)
#define TFT_SCLK 36           // Shared (but we use manual SPI for SD)
#define TFT_CS   6            // GPIO 6 - TFT Chip Select
#define TFT_DC   7            // GPIO 7 - Data/Command
#define TFT_RST  8            // GPIO 8 - Reset
#define TFT_BL   9            // GPIO 9 - Backlight (PWM)

// Backlight polarity - HIGH turns on backlight
#define TFT_BACKLIGHT_ON 1

// Touch controller not used
#define TOUCH_CS -1

// ============================================
// OPTIMIZATIONS
// ============================================

// Read windowed data for faster rendering
#define READ_WINDOWED_CD

// Touch not used - save memory
#define TOUCH_DISABLED

#endif
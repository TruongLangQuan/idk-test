#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WiFi Configuration
// ============================================
#define WIFI_SSID_AP      "CYD_WiFi"
#define WIFI_PASSWORD_AP  "12345678"

// Station mode (optional)
#define WIFI_SSID_STA     ""          // Leave empty to disable STA mode
#define WIFI_PASSWORD_STA ""

// ============================================
// Display Configuration
// ============================================
#define DISPLAY_WIDTH     320
#define DISPLAY_HEIGHT    240
#define DISPLAY_DRIVER    "ILI9341"

// Text color (16-bit RGB565)
#define COLOR_WHITE       0xFFFF
#define COLOR_BLACK       0x0000
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_GRAY        0x8410

// ============================================
// Feature Flags
// ============================================
#define ENABLE_WIFI       1
#define ENABLE_WEB_UI     1
#define ENABLE_TOUCH      0
#define ENABLE_BUTTONS    1
#define ENABLE_ADC        0
#define ENABLE_SERIAL_LOG 1

// ============================================
// Update Intervals (milliseconds)
// ============================================
#define DISPLAY_UPDATE_INTERVAL  1000
#define SENSOR_READ_INTERVAL     5000
#define WIFI_CHECK_INTERVAL      10000

// ============================================
// Web Server Configuration
// ============================================
#define WEB_SERVER_PORT   80
#define WEB_SERVER_TIMEOUT 30000

// ============================================
// Button Configuration
// ============================================
#define BUTTON_DEBOUNCE   50        // Debounce time in ms
#define LONG_PRESS_TIME   1000      // Time for long press in ms

// ============================================
// Screen Configuration
// ============================================
#define NUM_SCREENS       3
#define SCREEN_INDICATOR_X 280
#define SCREEN_INDICATOR_Y 230

// ============================================
// Serial Monitor
// ============================================
#define SERIAL_BAUD       115200
#define SERIAL_TX_PIN     1
#define SERIAL_RX_PIN     3

// ============================================
// Macros for Easy Access
// ============================================
#define DEBUG_ENABLED     ENABLE_SERIAL_LOG
#define DEBUG_PRINT(x)    if(DEBUG_ENABLED) Serial.println(x)
#define DEBUG_PRINTF(...) if(DEBUG_ENABLED) Serial.printf(__VA_ARGS__)

#endif // CONFIG_H

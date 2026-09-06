#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "soc/soc_caps.h"
#include <stdint.h>

#define USB_VID 0x303a
#define USB_PID 0x1001

static const uint8_t TX = 43;
static const uint8_t RX = 44;
static const uint8_t TXD2 = 1;
static const uint8_t RXD2 = 2;
static const uint8_t SDA = 9;
static const uint8_t SCL = 10;
static const uint8_t SS = -1;
static const uint8_t MOSI = -1;
static const uint8_t MISO = -1;
static const uint8_t SCK = -1;

#define HAS_BTN 0
#define BTN_ALIAS "\"Ok\""
#define BTN_PIN 5
#define BTN_ACT LOW

// Display setup
#define HAS_SCREEN 1
#define ROTATION 1
#define MINBRIGHT 160

#define USER_SETUP_LOADED 1
#define ST7789_DRIVER 1
#define INIT_SEQUENCE_3 1
#define TFT_IPS 1
#define TFT_RGB_ORDER 1 
#define TFT_INVERSION_ON 1
#define TFT_WIDTH 172
#define TFT_HEIGHT 320
#define CGRAM_OFFSET 1
#define TFT_X_OFFSET 34
#define TFT_Y_OFFSET 0

#define TFT_BL 46
#define TFT_RST 39
#define TFT_DC 41
#define TFT_MOSI 45
#define TFT_MISO 18
#define TFT_SCLK 40
#define TFT_CS 42
#define TOUCH_CS -1
#define SMOOTH_FONT 1
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 16000000

#define SDCARD_CS -1
#define SDCARD_SCK -1
#define SDCARD_MISO -1
#define SDCARD_MOSI -1
// Custom SDMMC freq for waveshare stability
#define BOARD_MAX_SDMMC_FREQ 20000000


#define GROVE_SDA 9
#define GROVE_SCL 10

#define SPI_SCK_PIN -1
#define SPI_MOSI_PIN -1
#define SPI_MISO_PIN -1
#define SPI_SS_PIN -1

// 5-Way Tactile Switch
#define BTN_UP 0
#define BTN_DOWN 2
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_CENTER 5

#define HAS_5_BUTTONS


#endif /* Pins_Arduino_h */

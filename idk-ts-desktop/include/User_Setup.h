#pragma once

#ifndef USER_SETUP_LOADED
#define USER_SETUP_LOADED 1
#endif

#define ST7789_DRIVER

// ESP32-S3: keep the TFT away from the flash/FSPI peripheral.
#define USE_HSPI_PORT

// 1.14 inch ST7789, landscape output after tft.setRotation(1).
#define TFT_WIDTH 135
#define TFT_HEIGHT 240

#define TFT_RGB_ORDER TFT_BGR
#define TFT_INVERSION_ON
#define CGRAM_OFFSET

// Tenstar TS-ESP32-S3 onboard 1.14 TFT pinout.
#define TFT_MISO -1
#define TFT_MOSI 35
#define TFT_SCLK 36
#define TFT_CS 7
#define TFT_DC 39
#define TFT_RST 40
#define TFT_BL 45
#define TFT_BACKLIGHT_ON HIGH

#define LOAD_GLCD
#define LOAD_FONT2

#define SPI_FREQUENCY 27000000
#define SPI_READ_FREQUENCY 16000000
#define SPI_TOUCH_FREQUENCY 2500000

#define SUPPORT_TRANSACTIONS
#define TOUCH_CS -1

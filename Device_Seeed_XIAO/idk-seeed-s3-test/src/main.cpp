#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <LovyanGFX.hpp>

// --- Pin Definitions ---
#define SPI_SCK    7  // D8
#define SPI_MOSI   9  // D10
#define SPI_MISO   8  // D9

#define LCD_CS     2  // D1
#define LCD_DC     3  // D2
#define LCD_RST    -1 
#define LCD_BL     -1 

#define SD_CS      4  // D3

#define JOY_UP     1  // D0
#define JOY_DOWN   5  // D4
#define JOY_LEFT   6  // D5
#define JOY_RIGHT  43 // D6
#define JOY_CENTER 44 // D7

// --- LovyanGFX Configuration ---
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;
      cfg.use_lock   = true; 
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = SPI_SCK;
      cfg.pin_mosi = SPI_MOSI;
      cfg.pin_miso = SPI_MISO;
      cfg.pin_dc   = LCD_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs   = LCD_CS;
      cfg.pin_rst  = LCD_RST;
      cfg.pin_busy = -1;
      cfg.panel_width      = 172;
      cfg.panel_height     = 320;
      cfg.offset_x         = 34; 
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = false;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = true; 
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX lcd;
String sd_test_result = "Not Tested";
String sd_capacity = "";

void testSDCard() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI, 4000000)) {
    sd_test_result = "SD Init Failed!";
    return;
  }

  // Get Card Size
  float cardSizeMB = SD.cardSize() / (1024.0 * 1024.0);
  float usedSpaceMB = SD.usedBytes() / (1024.0 * 1024.0);
  
  if (cardSizeMB > 1024) {
    float cardSizeGB = cardSizeMB / 1024.0;
    float usedSpaceGB = usedSpaceMB / 1024.0;
    sd_capacity = String(usedSpaceGB, 2) + "GB / " + String(cardSizeGB, 2) + "GB";
  } else {
    sd_capacity = String(usedSpaceMB, 1) + "MB / " + String(cardSizeMB, 1) + "MB";
  }

  // Clean up any old test file
  if (SD.exists("/test.txt")) {
    SD.remove("/test.txt");
  }

  // Write test
  File file = SD.open("/test.txt", FILE_WRITE);
  if (!file) {
    sd_test_result = "SD Write Error!";
    return;
  }
  file.println("Hello Xiao ESP32S3!");
  file.close();

  // Read test
  file = SD.open("/test.txt", FILE_READ);
  if (!file) {
    sd_test_result = "SD Read Error!";
    return;
  }
  String content = file.readStringUntil('\n');
  file.close();

  if (content.indexOf("Hello Xiao ESP32S3!") >= 0) {
    sd_test_result = "Test: PASS!";
  } else {
    sd_test_result = "Content Mismatch!";
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(JOY_UP, INPUT_PULLUP);
  pinMode(JOY_DOWN, INPUT_PULLUP);
  pinMode(JOY_LEFT, INPUT_PULLUP);
  pinMode(JOY_RIGHT, INPUT_PULLUP);
  pinMode(JOY_CENTER, INPUT_PULLUP);

  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(2);
  
  lcd.setCursor(10, 10);
  lcd.println("Testing SD Card...");
  
  testSDCard();
}

void loop() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setCursor(10, 10);
  lcd.println("--- PCB TESTER ---");

  lcd.setCursor(10, 40);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.println("SD " + sd_capacity);

  lcd.setCursor(10, 60);
  if (sd_test_result.indexOf("PASS") >= 0) {
    lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  } else {
    lcd.setTextColor(TFT_RED, TFT_BLACK);
  }
  lcd.println(sd_test_result);
  
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  bool up = digitalRead(JOY_UP) == LOW;
  bool down = digitalRead(JOY_DOWN) == LOW;
  bool left = digitalRead(JOY_LEFT) == LOW;
  bool right = digitalRead(JOY_RIGHT) == LOW;
  bool center = digitalRead(JOY_CENTER) == LOW;

  lcd.setCursor(10, 90);
  lcd.printf("UP: %s\n", up ? "PRESSED" : "---");
  lcd.printf("DOWN: %s\n", down ? "PRESSED" : "---");
  lcd.printf("LEFT: %s\n", left ? "PRESSED" : "---");
  lcd.printf("RIGHT: %s\n", right ? "PRESSED" : "---");
  lcd.printf("CENTER: %s\n", center ? "PRESSED" : "---");

  delay(100);
}

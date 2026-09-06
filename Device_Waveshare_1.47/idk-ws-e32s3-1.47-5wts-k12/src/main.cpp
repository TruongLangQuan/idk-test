#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <SD_MMC.h>
#include <FS.h>
#include <Wire.h>
#include <Arduino.h>

// ============================================================================
// 1. LovyanGFX Driver Configuration for Waveshare ESP32-S3-LCD-1.47B (ST7789)
// ============================================================================
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;

public:
  LGFX(void) {
    {
      auto bus_cfg = _bus_instance.config();
      bus_cfg.spi_host   = SPI2_HOST; // FSPI/SPI2 on ESP32-S3
      bus_cfg.spi_mode   = 0;
      bus_cfg.freq_write = 40000000;
      bus_cfg.freq_read  = 16000000;
      bus_cfg.pin_sclk   = 40; // GPIO 40
      bus_cfg.pin_mosi   = 45; // GPIO 45
      bus_cfg.pin_miso   = -1;
      bus_cfg.pin_dc     = 41; // GPIO 41
      _bus_instance.config(bus_cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto panel_cfg = _panel_instance.config();
      panel_cfg.pin_cs           = 42; // GPIO 42
      panel_cfg.pin_rst          = 39; // GPIO 39
      panel_cfg.pin_busy         = -1;
      panel_cfg.panel_width      = 172;
      panel_cfg.panel_height     = 320;
      panel_cfg.offset_x         = 34; // Centering offset
      panel_cfg.offset_y         = 0;
      panel_cfg.offset_rotation  = 0;
      panel_cfg.dummy_read_bits  = 8;
      panel_cfg.readable         = false;
      panel_cfg.invert           = true;  // ST7789 Inversion ON
      panel_cfg.rgb_order        = false; // BGR
      _panel_instance.config(panel_cfg);
    }

    {
      auto light_cfg = _light_instance.config();
      light_cfg.pin_bl = 46; // Backlight = GPIO 46
      light_cfg.invert = false;
      light_cfg.freq   = 12000;
      light_cfg.pwm_channel = 7;
      _light_instance.config(light_cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};

namespace {

LGFX lcd;
LGFX_Sprite canvas(&lcd);

// ============================================================================
// 2. Hardware Pin Mappings
// ============================================================================

// LCD & RGB Pins
constexpr int kPinLCD_BL    = 46; // Backlight = GPIO 46
constexpr int kPinRGB_LED   = 38; // RGB LED = GPIO 38

// CardKB I2C Bus Pins
constexpr int kPinCARDKB_SDA = 9;  // I2C SDA (Header GP9)
constexpr int kPinCARDKB_SCL = 10; // I2C SCL (Header GP10)

// 5-Way Tactile Switch 1 (K1)
constexpr int kPinK1_UP     = 0;
constexpr int kPinK1_DOWN   = 2;
constexpr int kPinK1_LEFT   = 3;
constexpr int kPinK1_RIGHT  = 4;
constexpr int kPinK1_PRESS  = 5;

// 5-Way Tactile Switch 2 (K2)
constexpr int kPinK2_UP     = 6;
constexpr int kPinK2_DOWN   = 7;
constexpr int kPinK2_LEFT   = 8;
constexpr int kPinK2_RIGHT  = 11;
constexpr int kPinK2_PRESS  = 43; // TX Header Pin

// Mini Slide Switch Pin
constexpr int kPinSLIDE_SW  = 44; // RX Header Pin

// SD MMC Bus Pins
constexpr int kPinSD_CLK    = 14;
constexpr int kPinSD_CMD    = 15;
constexpr int kPinSD_D0     = 16;
constexpr int kPinSD_D1     = 18;
constexpr int kPinSD_D2     = 17;
constexpr int kPinSD_D3     = 21;

// ============================================================================
// 3. State & Variables
// ============================================================================
struct SwitchState {
  bool up;
  bool down;
  bool left;
  bool right;
  bool press;
};

SwitchState g_k1 = {false, false, false, false, false};
SwitchState g_k2 = {false, false, false, false, false};
bool g_slide_sw_on = false;

// Dynamic CardKB I2C Auto-Scanner State
bool g_cardkb_present = false;
uint8_t g_detected_i2c_addr = 0x00;
String g_i2c_scan_status = "Scanning I2C...";
char g_last_cardkb_char = 0;
String g_cardkb_buffer = "";
uint32_t g_cardkb_key_count = 0;

// SD Card State
bool g_sd_mounted = false;
uint64_t g_sd_total_mb = 0;
uint64_t g_sd_free_mb = 0;
float g_sd_write_speed_kbs = 0.0f;
float g_sd_read_speed_kbs = 0.0f;
String g_sd_status_msg = "SD Not Tested";
bool g_sd_testing = false;

// Battery State
float g_bat_voltage = 3.85f;
int g_bat_percentage = 78;

// Color Palette (RGB565)
constexpr uint16_t COLOR_BG        = 0x10A2; // Dark Navy #101420
constexpr uint16_t COLOR_CARD      = 0x1CE7; // Slate Blue #1C2438
constexpr uint16_t COLOR_TEXT_MAIN = 0xFFFF; // Pure White
constexpr uint16_t COLOR_TEXT_MUTED= 0x8C71; // Cool Gray
constexpr uint16_t COLOR_CYAN      = 0x07FF; // Vibrant Cyan #00F0FF
constexpr uint16_t COLOR_AMBER     = 0xFD60; // Warm Gold #FFB000
constexpr uint16_t COLOR_GREEN     = 0x2E66; // Emerald #20E070
constexpr uint16_t COLOR_RED       = 0xF886; // Coral Red #FF3050
constexpr uint16_t COLOR_ACTIVE    = 0x07E0; // Neon Lime #00FF66

// ============================================================================
// 4. Hardware Routines
// ============================================================================

void initGPIO() {
  pinMode(kPinLCD_BL, OUTPUT);
  digitalWrite(kPinLCD_BL, HIGH);

  pinMode(kPinRGB_LED, OUTPUT);
  digitalWrite(kPinRGB_LED, LOW);

  pinMode(kPinK1_UP, INPUT_PULLUP);
  pinMode(kPinK1_DOWN, INPUT_PULLUP);
  pinMode(kPinK1_LEFT, INPUT_PULLUP);
  pinMode(kPinK1_RIGHT, INPUT_PULLUP);
  pinMode(kPinK1_PRESS, INPUT_PULLUP);

  pinMode(kPinK2_UP, INPUT_PULLUP);
  pinMode(kPinK2_DOWN, INPUT_PULLUP);
  pinMode(kPinK2_LEFT, INPUT_PULLUP);
  pinMode(kPinK2_RIGHT, INPUT_PULLUP);
  pinMode(kPinK2_PRESS, INPUT_PULLUP);

  pinMode(kPinSLIDE_SW, INPUT_PULLUP);
}

void initCardKB() {
  pinMode(kPinCARDKB_SDA, INPUT_PULLUP);
  pinMode(kPinCARDKB_SCL, INPUT_PULLUP);

  Wire.begin(kPinCARDKB_SDA, kPinCARDKB_SCL, 50000); // 50kHz for rock-solid timing
  Wire.setTimeOut(50);
}

void scanI2CBus() {
  uint8_t found_addr = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      found_addr = addr;
      break;
    }
  }

  if (found_addr != 0) {
    g_detected_i2c_addr = found_addr;
    g_cardkb_present = true;
    char buf[24];
    snprintf(buf, sizeof(buf), "Dev at 0x%02X", found_addr);
    g_i2c_scan_status = String(buf);
  } else {
    g_detected_i2c_addr = 0;
    g_cardkb_present = false;
    g_i2c_scan_status = "No I2C Device";
  }
}

void pollCardKB() {
  uint32_t now = millis();
  static uint32_t last_scan_ms = 0;

  // Run full I2C address scan every 800ms
  if (!g_cardkb_present || (now - last_scan_ms > 800)) {
    last_scan_ms = now;
    scanI2CBus();
  }

  if (g_cardkb_present && g_detected_i2c_addr != 0) {
    Wire.requestFrom(g_detected_i2c_addr, (uint8_t)1);
    if (Wire.available()) {
      char c = Wire.read();
      if (c != 0) {
        g_last_cardkb_char = c;
        g_cardkb_key_count++;

        if (c == 0x08) { // Backspace
          if (g_cardkb_buffer.length() > 0) {
            g_cardkb_buffer.remove(g_cardkb_buffer.length() - 1);
          }
        } else if (c == 0x0D || c == 0x0A) { // Enter / Clear
          g_cardkb_buffer = "";
        } else if (c >= 32 && c <= 126) {
          if (g_cardkb_buffer.length() < 18) {
            g_cardkb_buffer += c;
          }
        }
      }
    }
  }
}

void pollInputs() {
  g_k1.up    = (digitalRead(kPinK1_UP) == LOW);
  g_k1.down  = (digitalRead(kPinK1_DOWN) == LOW);
  g_k1.left  = (digitalRead(kPinK1_LEFT) == LOW);
  g_k1.right = (digitalRead(kPinK1_RIGHT) == LOW);
  g_k1.press = (digitalRead(kPinK1_PRESS) == LOW);

  g_k2.up    = (digitalRead(kPinK2_UP) == LOW);
  g_k2.down  = (digitalRead(kPinK2_DOWN) == LOW);
  g_k2.left  = (digitalRead(kPinK2_LEFT) == LOW);
  g_k2.right = (digitalRead(kPinK2_RIGHT) == LOW);
  g_k2.press = (digitalRead(kPinK2_PRESS) == LOW);

  g_slide_sw_on = (digitalRead(kPinSLIDE_SW) == LOW);

  pollCardKB();
}

void pollBattery() {
  int raw_adc = analogRead(1);
  if (raw_adc > 0) {
    g_bat_voltage = (raw_adc / 4095.0f) * 3.3f * 2.0f;
  } else {
    g_bat_voltage = 3.7f + 0.3f * sin(millis() / 4000.0f);
  }

  if (g_bat_voltage > 4.2f) g_bat_voltage = 4.2f;
  if (g_bat_voltage < 3.2f) g_bat_voltage = 3.2f;

  g_bat_percentage = (int)(((g_bat_voltage - 3.2f) / 1.0f) * 100.0f);
  if (g_bat_percentage > 100) g_bat_percentage = 100;
  if (g_bat_percentage < 0) g_bat_percentage = 0;
}

bool mountSDCard() {
  SD_MMC.end();
  vTaskDelay(pdMS_TO_TICKS(10));
  if (!SD_MMC.setPins(kPinSD_CLK, kPinSD_CMD, kPinSD_D0, kPinSD_D1, kPinSD_D2, kPinSD_D3)) {
    g_sd_status_msg = "Pin Config Fail";
    g_sd_mounted = false;
    return false;
  }

  if (!SD_MMC.begin("/sdcard", false, false, 20000000)) {
    if (!SD_MMC.begin("/sdcard", true, false, 20000000)) {
      g_sd_status_msg = "No SD Card";
      g_sd_mounted = false;
      return false;
    }
  }

  g_sd_mounted = true;
  g_sd_total_mb = SD_MMC.totalBytes() / (1024 * 1024);
  g_sd_free_mb = (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024);
  g_sd_status_msg = "SD Card Ready";
  return true;
}

void runSDCardTest() {
  if (!g_sd_mounted && !mountSDCard()) {
    return;
  }

  g_sd_testing = true;
  g_sd_status_msg = "Testing SD...";

  constexpr size_t test_buf_size = 32768; // 32KB
  uint8_t* test_buffer = (uint8_t*)malloc(test_buf_size);

  if (!test_buffer) {
    g_sd_status_msg = "RAM Alloc Fail";
    g_sd_testing = false;
    return;
  }

  for (size_t i = 0; i < test_buf_size; i++) {
    test_buffer[i] = (uint8_t)(i & 0xFF);
  }

  // Write Test
  fs::File test_file = SD_MMC.open("/idk_test.bin", FILE_WRITE);
  if (!test_file) {
    g_sd_status_msg = "Write Err";
    free(test_buffer);
    g_sd_testing = false;
    return;
  }

  uint32_t start_w = millis();
  size_t bytes_written = test_file.write(test_buffer, test_buf_size);
  test_file.flush();
  test_file.close();
  uint32_t elapsed_w = millis() - start_w;

  if (elapsed_w > 0) {
    g_sd_write_speed_kbs = ((float)bytes_written / 1024.0f) / (elapsed_w / 1000.0f);
  }

  // Read Test
  test_file = SD_MMC.open("/idk_test.bin", FILE_READ);
  if (!test_file) {
    g_sd_status_msg = "Read Err";
    free(test_buffer);
    g_sd_testing = false;
    return;
  }

  uint32_t start_r = millis();
  size_t bytes_read = test_file.read(test_buffer, test_buf_size);
  test_file.close();
  uint32_t elapsed_r = millis() - start_r;

  if (elapsed_r > 0) {
    g_sd_read_speed_kbs = ((float)bytes_read / 1024.0f) / (elapsed_r / 1000.0f);
  }

  SD_MMC.remove("/idk_test.bin");
  free(test_buffer);

  g_sd_status_msg = "Test Passed!";
  g_sd_testing = false;
}

// ============================================================================
// 5. UI Engine
// ============================================================================

void drawDPad(int center_x, int center_y, const char* title, const SwitchState& state, uint16_t primary_color) {
  constexpr int box_w = 26;
  constexpr int box_h = 18;
  constexpr int spacing = 20;

  canvas.setTextColor(COLOR_TEXT_MAIN, COLOR_CARD);
  canvas.setTextDatum(top_center);
  canvas.drawString(title, center_x, center_y - 36);

  // Up
  uint16_t c_up = state.up ? COLOR_ACTIVE : COLOR_CARD;
  canvas.fillRoundRect(center_x - box_w/2, center_y - spacing - box_h/2, box_w, box_h, 4, c_up);
  canvas.drawRoundRect(center_x - box_w/2, center_y - spacing - box_h/2, box_w, box_h, 4, primary_color);
  canvas.setTextColor(state.up ? COLOR_BG : COLOR_TEXT_MUTED, c_up);
  canvas.setTextDatum(middle_center);
  canvas.drawString("^", center_x, center_y - spacing);

  // Down
  uint16_t c_down = state.down ? COLOR_ACTIVE : COLOR_CARD;
  canvas.fillRoundRect(center_x - box_w/2, center_y + spacing - box_h/2, box_w, box_h, 4, c_down);
  canvas.drawRoundRect(center_x - box_w/2, center_y + spacing - box_h/2, box_w, box_h, 4, primary_color);
  canvas.setTextColor(state.down ? COLOR_BG : COLOR_TEXT_MUTED, c_down);
  canvas.setTextDatum(middle_center);
  canvas.drawString("v", center_x, center_y + spacing);

  // Left
  uint16_t c_left = state.left ? COLOR_ACTIVE : COLOR_CARD;
  canvas.fillRoundRect(center_x - spacing - box_w/2, center_y - box_h/2, box_w, box_h, 4, c_left);
  canvas.drawRoundRect(center_x - spacing - box_w/2, center_y - box_h/2, box_w, box_h, 4, primary_color);
  canvas.setTextColor(state.left ? COLOR_BG : COLOR_TEXT_MUTED, c_left);
  canvas.setTextDatum(middle_center);
  canvas.drawString("<", center_x - spacing, center_y);

  // Right
  uint16_t c_right = state.right ? COLOR_ACTIVE : COLOR_CARD;
  canvas.fillRoundRect(center_x + spacing - box_w/2, center_y - box_h/2, box_w, box_h, 4, c_right);
  canvas.drawRoundRect(center_x + spacing - box_w/2, center_y - box_h/2, box_w, box_h, 4, primary_color);
  canvas.setTextColor(state.right ? COLOR_BG : COLOR_TEXT_MUTED, c_right);
  canvas.setTextDatum(middle_center);
  canvas.drawString(">", center_x + spacing, center_y);

  // Center / Press
  uint16_t c_press = state.press ? COLOR_ACTIVE : COLOR_CARD;
  canvas.fillRoundRect(center_x - box_w/2, center_y - box_h/2, box_w, box_h, 4, c_press);
  canvas.drawRoundRect(center_x - box_w/2, center_y - box_h/2, box_w, box_h, 4, primary_color);
  canvas.setTextColor(state.press ? COLOR_BG : COLOR_TEXT_MUTED, c_press);
  canvas.setTextDatum(middle_center);
  canvas.drawString("OK", center_x, center_y);
}

void renderUI() {
  canvas.fillScreen(COLOR_BG);

  // Header Bar
  canvas.fillRect(0, 0, 320, 24, COLOR_CARD);
  canvas.drawFastHLine(0, 24, 320, COLOR_CYAN);

  canvas.setTextDatum(middle_left);
  canvas.setTextColor(COLOR_CYAN, COLOR_CARD);
  canvas.drawString("IDK WS-1.47-5WTS-K12", 8, 12);

  // Slide Switch Status
  canvas.setTextDatum(middle_right);
  if (g_slide_sw_on) {
    canvas.setTextColor(COLOR_GREEN, COLOR_CARD);
    canvas.drawString("SW: ON", 230, 12);
  } else {
    canvas.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    canvas.drawString("SW: OFF", 230, 12);
  }

  // Battery Status
  char bat_str[16];
  snprintf(bat_str, sizeof(bat_str), "%.2fV %d%%", g_bat_voltage, g_bat_percentage);
  canvas.setTextColor(COLOR_AMBER, COLOR_CARD);
  canvas.drawString(bat_str, 312, 12);

  // Dual D-Pads
  drawDPad(50,  95, "5WTS K1", g_k1, COLOR_CYAN);
  drawDPad(150, 95, "5WTS K2", g_k2, COLOR_AMBER);

  // Vertical Divider
  canvas.drawFastVLine(205, 30, 135, COLOR_CARD);

  // SD Card Card
  canvas.fillRoundRect(212, 30, 102, 134, 6, COLOR_CARD);
  canvas.drawRoundRect(212, 30, 102, 134, 6, g_sd_mounted ? COLOR_GREEN : COLOR_RED);

  canvas.setTextDatum(top_center);
  canvas.setTextColor(COLOR_TEXT_MAIN, COLOR_CARD);
  canvas.drawString("SD BENCHMARK", 263, 34);

  canvas.setTextDatum(top_center);
  canvas.setTextColor(g_sd_mounted ? COLOR_GREEN : COLOR_RED, COLOR_CARD);
  canvas.drawString(g_sd_status_msg.c_str(), 263, 48);

  if (g_sd_mounted) {
    canvas.setTextDatum(top_left);
    canvas.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);

    char sz_str[24];
    snprintf(sz_str, sizeof(sz_str), "Free:%lluMB", g_sd_free_mb);
    canvas.drawString(sz_str, 218, 64);

    snprintf(sz_str, sizeof(sz_str), "Tot :%lluMB", g_sd_total_mb);
    canvas.drawString(sz_str, 218, 78);

    canvas.setTextColor(COLOR_CYAN, COLOR_CARD);
    snprintf(sz_str, sizeof(sz_str), "W:%.1fKB/s", g_sd_write_speed_kbs);
    canvas.drawString(sz_str, 218, 96);

    canvas.setTextColor(COLOR_AMBER, COLOR_CARD);
    snprintf(sz_str, sizeof(sz_str), "R:%.1fKB/s", g_sd_read_speed_kbs);
    canvas.drawString(sz_str, 218, 110);
  }

  // CardKB Live Typed Input Bar (Bottom Overlay with Realtime I2C Scan Status)
  canvas.fillRoundRect(8, 140, 190, 26, 4, COLOR_CARD);
  canvas.drawRoundRect(8, 140, 190, 26, 4, g_cardkb_present ? COLOR_CYAN : COLOR_RED);
  canvas.setTextDatum(middle_left);

  if (g_cardkb_present) {
    canvas.setTextColor(COLOR_CYAN, COLOR_CARD);
    String txt = "KB: " + g_cardkb_buffer + "_";
    canvas.drawString(txt.c_str(), 14, 153);
  } else {
    canvas.setTextColor(COLOR_RED, COLOR_CARD);
    String status_txt = "I2C: " + g_i2c_scan_status;
    canvas.drawString(status_txt.c_str(), 14, 153);
  }

  // Footer Tip
  canvas.setTextDatum(bottom_center);
  canvas.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  canvas.drawString("Press K1 PRESS: Test", 263, 160);

  // Push Canvas to Display
  canvas.pushSprite(0, 0);
}

} // namespace

// ============================================================================
// 6. Setup & Loop
// ============================================================================

void setup() {
  Serial.begin(115200);

  // Backlight Pin 46 HIGH
  pinMode(kPinLCD_BL, OUTPUT);
  digitalWrite(kPinLCD_BL, HIGH);

  // Initialize Display
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);

  canvas.createSprite(320, 172);

  // Startup Splash Screen
  canvas.fillScreen(COLOR_BG);
  canvas.setTextDatum(middle_center);
  canvas.setTextColor(COLOR_CYAN, COLOR_BG);
  canvas.drawString("IDK WS-1.47-5WTS-K12", 160, 70);
  canvas.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  canvas.drawString("SCANNING I2C BUS...", 160, 100);
  canvas.pushSprite(0, 0);

  delay(150);

  initGPIO();
  initCardKB();
  mountSDCard();

  Serial.println("[SYSTEM] Setup completed successfully!");
}

void loop() {
  pollInputs();
  pollBattery();

  static bool prev_k1_press = false;
  if (g_k1.press && !prev_k1_press && !g_sd_testing) {
    runSDCardTest();
  }
  prev_k1_press = g_k1.press;

  renderUI();
  vTaskDelay(pdMS_TO_TICKS(30));
}

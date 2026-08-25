#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <SD_MMC.h>
#include <FS.h>

// 1. Custom LovyanGFX Device Class for 1.47 inch LCD (ST7789, 172x320)
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:
  LGFX(void) {
    {
      auto bus_cfg = _bus_instance.config();
      bus_cfg.spi_host = SPI2_HOST; // FSPI/SPI2 on ESP32-S3
      bus_cfg.spi_mode = 0;
      bus_cfg.freq_write = 40000000; // 40MHz write
      bus_cfg.freq_read  = 16000000; // 16MHz read
      bus_cfg.pin_sclk = 40;
      bus_cfg.pin_mosi = 45;
      bus_cfg.pin_miso = -1;
      bus_cfg.pin_dc   = 41;
      _bus_instance.config(bus_cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto panel_cfg = _panel_instance.config();
      panel_cfg.pin_cs           = 42;
      panel_cfg.pin_rst          = 39;
      panel_cfg.pin_busy         = -1;
      panel_cfg.panel_width      = 172;
      panel_cfg.panel_height     = 320;
      panel_cfg.offset_x         = 34; // Offset to center the 172-wide panel in 240-wide CGRAM frame
      panel_cfg.offset_y         = 0;
      panel_cfg.offset_rotation  = 0;
      panel_cfg.dummy_read_bits  = 8;
      panel_cfg.readable         = false;
      panel_cfg.invert           = true;  // Color inversion on for ST7789
      panel_cfg.rgb_order        = false; // BGR order
      _panel_instance.config(panel_cfg);
      setPanel(&_panel_instance);
    }
  }
};

namespace {

// LCD Device instance & Double-buffered Sprite Canvas
LGFX lcd;
LGFX_Sprite canvas(&lcd);

// External GPIO Buttons Configuration
constexpr int kPinSW_UP     = 1;
constexpr int kPinSW_DOWN   = 2;
constexpr int kPinSW_LEFT   = 3;
constexpr int kPinSW_RIGHT  = 4;
constexpr int kPinSW_CENTER = 5;
constexpr int kPinBTN_A     = 6;
constexpr int kPinBTN_B     = 7;

struct Button {
  const char* name;
  int pin;
  bool pressed;
  int x;
  int y;
  int w;
  int h;
};

// Layout for Visual Indicators on 320x172 Widescreen
Button g_buttons[] = {
  // 5-Way Switch (Right Column D-Pad)
  {"UP",     kPinSW_UP,     false, 225, 35,  35, 22},
  {"DOWN",   kPinSW_DOWN,   false, 225, 95,  35, 22},
  {"LEFT",   kPinSW_LEFT,   false, 180, 65,  35, 22},
  {"RIGHT",  kPinSW_RIGHT,  false, 270, 65,  35, 22},
  {"CENTER", kPinSW_CENTER, false, 225, 65,  35, 22},
  
  // Independent Push Buttons (Left Column Indicators)
  {"BTN A",  kPinBTN_A,     false, 12,  125, 65, 22},
  {"BTN B",  kPinBTN_B,     false, 87,  125, 65, 22}
};

// Built-in SDMMC State Variables
bool g_sd_mounted = false;
String g_sd_type = "None";
uint64_t g_sd_total_bytes = 0;
uint64_t g_sd_free_bytes = 0;
String g_sd_test_result = "IDLE";

// Polling debouncer
uint32_t g_last_poll_ms = 0;
constexpr uint32_t kPollIntervalMs = 20;

// Initialize SD Card via SDMMC (4-bit mode, fallback to 1-bit)
bool initSDCard() {
  Serial.println("[SDMMC] Setting pins...");
  if (!SD_MMC.setPins(14, 15, 16, 18, 17, 21)) {
    Serial.println("[SDMMC] Pin routing configuration failed.");
    g_sd_mounted = false;
    g_sd_type = "PinErr";
    return false;
  }

  Serial.println("[SDMMC] Initializing SD Card...");
  // Try 4-bit mode (pins: CLK, CMD, D0, D1, D2, D3)
  if (!SD_MMC.begin("/sdcard", false, false, 20000000)) {
    Serial.println("[SDMMC] 4-bit mount failed. Retrying in 1-bit mode...");
    // Try 1-bit mode (pins: CLK, CMD, D0 only)
    if (!SD_MMC.begin("/sdcard", true, false, 20000000)) {
      Serial.println("[SDMMC] 1-bit mount failed.");
      g_sd_mounted = false;
      g_sd_type = "Fail";
      return false;
    }
  }

  g_sd_mounted = true;
  sdcard_type_t cardType = SD_MMC.cardType();
  if (cardType == CARD_MMC) {
    g_sd_type = "MMC";
  } else if (cardType == CARD_SD) {
    g_sd_type = "SDSC";
  } else if (cardType == CARD_SDHC) {
    g_sd_type = "SDHC";
  } else {
    g_sd_type = "Unknown";
  }

  g_sd_total_bytes = SD_MMC.totalBytes();
  g_sd_free_bytes = SD_MMC.totalBytes() - SD_MMC.usedBytes();

  Serial.printf("[SDMMC] Card Type: %s\n", g_sd_type.c_str());
  Serial.printf("[SDMMC] Card Size: %llu MB\n", g_sd_total_bytes / (1024 * 1024));
  Serial.printf("[SDMMC] Free Space: %llu MB\n", g_sd_free_bytes / (1024 * 1024));
  return true;
}

// Perform active Read/Write verification on SDMMC card
void verifySDCard() {
  if (!g_sd_mounted) {
    g_sd_test_result = "NO SD";
    Serial.println("[SDMMC Test] Cannot run test, card not mounted.");
    return;
  }

  g_sd_test_result = "RUNNING";
  Serial.println("[SDMMC Test] Starting write test...");

  File testFile = SD_MMC.open("/sd_test.txt", FILE_WRITE);
  if (!testFile) {
    g_sd_test_result = "WR_FAIL";
    Serial.println("[SDMMC Test] Failed to open file for writing.");
    return;
  }

  String testMessage = "ESP32S3-LCD-1.47 SDMMC Test Successful! Millis: " + String(millis());
  if (testFile.println(testMessage)) {
    Serial.println("[SDMMC Test] Write successful.");
  } else {
    g_sd_test_result = "WR_FAIL";
    Serial.println("[SDMMC Test] Write payload failed.");
    testFile.close();
    return;
  }
  testFile.close();

  Serial.println("[SDMMC Test] Starting read validation...");
  testFile = SD_MMC.open("/sd_test.txt", FILE_READ);
  if (!testFile) {
    g_sd_test_result = "RD_FAIL";
    Serial.println("[SDMMC Test] Failed to open file for reading.");
    return;
  }

  String readLine = testFile.readStringUntil('\n');
  readLine.trim();
  testFile.close();

  Serial.printf("[SDMMC Test] Read string: '%s'\n", readLine.c_str());
  if (readLine.startsWith("ESP32S3-LCD-1.47 SDMMC Test Successful!")) {
    g_sd_test_result = "PASS";
    Serial.println("[SDMMC Test] Validation Passed!");
  } else {
    g_sd_test_result = "VERIFY_ERR";
    Serial.println("[SDMMC Test] Validation Failed - content mismatch.");
  }
}

// Render interface to Sprite and push to LCD (double-buffered, flicker-free)
void drawUI() {
  canvas.fillScreen(TFT_BLACK);

  // Top header bar
  canvas.fillRect(0, 0, 320, 18, 0x1A6D); // Dark slate header
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextDatum(top_left);
  canvas.drawString(" ESP32-S3-LCD-1.47 CO-OP TEST", 4, 3);

  // Left Column - SD Card MMC Status Box
  canvas.drawRect(5, 24, 160, 92, TFT_DARKGREY);
  canvas.setTextColor(TFT_CYAN);
  canvas.drawString("BUILT-IN SDMMC INFO", 10, 28);

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("Mount:", 10, 44);
  if (g_sd_mounted) {
    canvas.setTextColor(TFT_GREEN);
    canvas.drawString("OK", 65, 44);
  } else {
    canvas.setTextColor(TFT_RED);
    canvas.drawString("FAIL", 65, 44);
  }

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("Type:", 10, 60);
  canvas.setTextColor(TFT_YELLOW);
  canvas.drawString(g_sd_type, 65, 60);

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("Size:", 10, 76);
  if (g_sd_mounted) {
    canvas.setTextColor(TFT_LIGHTGRAY);
    canvas.printf("%lluMB", g_sd_total_bytes / (1024 * 1024));
  } else {
    canvas.drawString("-", 65, 76);
  }

  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("Test:", 10, 92);
  if (g_sd_test_result == "PASS") {
    canvas.setTextColor(TFT_GREEN);
  } else if (g_sd_test_result == "IDLE") {
    canvas.setTextColor(TFT_LIGHTGRAY);
  } else {
    canvas.setTextColor(TFT_RED);
  }
  canvas.drawString(g_sd_test_result, 65, 92);

  // Right Column - 5-Way & Push Button D-Pad Layout
  canvas.drawRect(172, 24, 143, 143, TFT_DARKGREY);
  canvas.setTextColor(TFT_CYAN);
  canvas.drawString("INPUT MONITORS", 178, 28);

  // Draw D-pad and Push Buttons
  for (const auto& btn : g_buttons) {
    uint16_t boxColor = btn.pressed ? TFT_GREEN : TFT_BLACK;
    uint16_t borderColor = btn.pressed ? TFT_GREEN : TFT_LIGHTGRAY;
    uint16_t textColor = btn.pressed ? TFT_BLACK : TFT_WHITE;

    canvas.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 4, boxColor);
    canvas.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 4, borderColor);

    canvas.setTextColor(textColor);
    canvas.setTextDatum(middle_center);
    if (strcmp(btn.name, "CENTER") == 0) {
      canvas.drawString("CTR", btn.x + (btn.w / 2), btn.y + (btn.h / 2));
    } else {
      canvas.drawString(btn.name, btn.x + (btn.w / 2), btn.y + (btn.h / 2));
    }
  }

  // Help Footer Text (Left Bottom)
  canvas.setTextColor(TFT_DARKGREY);
  canvas.setTextDatum(top_left);
  canvas.drawString("Press CTR button to verify SD", 6, 118);

  // Push Canvas to Display
  canvas.pushSprite(0, 0);
}

} // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n==================================");
  Serial.println("  ESP32-S3-LCD-1.47 MULTI-TEST    ");
  Serial.println("==================================");

  // Initialize LCD
  lcd.init();
  lcd.setRotation(1); // Set display landscape

  // Turn on Backlight (pin 48)
  pinMode(48, OUTPUT);
  digitalWrite(48, HIGH);

  // Initialize Sprite Canvas
  canvas.createSprite(320, 172);

  // Register Buttons as INPUT_PULLUP
  for (const auto& btn : g_buttons) {
    pinMode(btn.pin, INPUT_PULLUP);
    Serial.printf("[Input] Pin %d registered for %s\n", btn.pin, btn.name);
  }

  // Initialize SD Card MMC interface
  initSDCard();

  // Initial draw
  drawUI();
}

void loop() {
  uint32_t now = millis();

  if (now - g_last_poll_ms >= kPollIntervalMs) {
    g_last_poll_ms = now;
    bool state_changed = false;

    // Poll D-pad switches & Push buttons
    for (auto& btn : g_buttons) {
      bool current_pressed = (digitalRead(btn.pin) == LOW);
      if (current_pressed != btn.pressed) {
        btn.pressed = current_pressed;
        state_changed = true;

        Serial.printf("[Input] Event: %s was %s\n", btn.name, current_pressed ? "PRESSED" : "RELEASED");

        // Run active write/read test when Center button is clicked
        if (strcmp(btn.name, "CENTER") == 0 && current_pressed) {
          verifySDCard();
        }
      }
    }

    // Double-buffered push if inputs changed
    if (state_changed) {
      drawUI();
    }
  }
}

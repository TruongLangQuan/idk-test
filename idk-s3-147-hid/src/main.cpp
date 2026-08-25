#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <SD_MMC.h>
#include <FS.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBMSC.h"
#include "sdmmc_cmd.h"
#include <vector>

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

// C++ Hack subclass to expose protected sdmmc_card_t* for raw sector access
class BridgeSDMMCFS : public fs::SDMMCFS {
public:
  sdmmc_card_t* card() { return _card; }
};

namespace {

// LCD Device instance & Double-buffered Sprite Canvas
LGFX lcd;
LGFX_Sprite canvas(&lcd);

// USB Composite Instances
USBHIDKeyboard Keyboard;
USBMSC msc;

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

// Layout for Visual Indicators on 320x172 Widescreen (Not drawn, but kept in code for poll triggers)
Button g_buttons[] = {
  {"UP",     kPinSW_UP,     false, 225, 35,  35, 22},
  {"DOWN",   kPinSW_DOWN,   false, 225, 95,  35, 22},
  {"LEFT",   kPinSW_LEFT,   false, 180, 65,  35, 22},
  {"RIGHT",  kPinSW_RIGHT,  false, 270, 65,  35, 22},
  {"CENTER", kPinSW_CENTER, false, 225, 65,  35, 22},
  {"BTN A",  kPinBTN_A,     false, 12,  125, 65, 22},
  {"BTN B",  kPinBTN_B,     false, 87,  125, 65, 22}
};

// Embedded Ducky Script Fallback
const String kDefaultScript = 
  "CTRL-ALT t\n"
  "DELAY 1000\n"
  "STRING sudo pacman -Syu --needed --noconfirm && yay -Syu --needed --noconfirm && npm update all && flatpak update --noninteractive \n"
  "ENTER\n"
  "DELAY 1000\n"
  "STRING 15031169\n"
  "ENTER\n";

// Script State Variables
bool g_sd_mounted = false;
String g_payload_source = "INTERNAL";
std::vector<String> g_script_lines;
bool g_injecting = false;
int g_current_line_idx = 0;
String g_status_msg = "INIT USB...";
String g_last_sent_action = "CONNECTING TO PC...";

// Premium Cyberpunk Color Palette Tokens
constexpr uint32_t kColorBg         = 0x060713; // Obsidian Space Black
constexpr uint32_t kColorHeader     = 0x1E1035; // Royal Dark Purple
constexpr uint32_t kColorBorder     = 0x4D0099; // Cyber Neon Purple
constexpr uint32_t kColorActive     = 0x00F5FF; // Electric Turquoise Cyan
constexpr uint32_t kColorSuccess    = 0x39FF14; // Acid Neon Green
constexpr uint32_t kColorWarn       = 0xFF007F; // Hot Pink
constexpr uint32_t kColorText       = 0xFFFFFF; // Crisp Ice White
constexpr uint32_t kColorMuted      = 0x7E7E9E; // Sleek Muted Slate

// Debouncer
uint32_t g_last_poll_ms = 0;
constexpr uint32_t kPollIntervalMs = 15;

// Helper to access underlying SDMMC card structure safely
sdmmc_card_t* getSDMMCCard() {
  return ((BridgeSDMMCFS*)&SD_MMC)->card();
}

// Parse ducky script text line-by-line
void parseScript(const String& content) {
  g_script_lines.clear();
  int start = 0;
  int len = content.length();
  while (start < len) {
    int end = content.indexOf('\n', start);
    if (end == -1) {
      end = len;
    }
    String line = content.substring(start, end);
    line.trim();
    if (line.length() > 0) {
      g_script_lines.push_back(line);
    }
    start = end + 1;
  }
}

// Initialize SDMMC Card to read live custom payload
void initSDCard() {
  Serial.println("[SDMMC] Setting pins...");
  if (!SD_MMC.setPins(14, 15, 16, 18, 17, 21)) {
    Serial.println("[SDMMC] Pin routing configuration failed.");
    g_sd_mounted = false;
    return;
  }

  Serial.println("[SDMMC] Mounting SD Card...");
  if (!SD_MMC.begin("/sdcard", false, false, 20000000)) {
    if (!SD_MMC.begin("/sdcard", true, false, 20000000)) {
      Serial.println("[SDMMC] Mount failed.");
      g_sd_mounted = false;
      return;
    }
  }

  g_sd_mounted = true;
  Serial.println("[SDMMC] Mounted SD card successfully.");
  
  // Try to load /auto-update.txt
  if (SD_MMC.exists("/auto-update.txt")) {
    File f = SD_MMC.open("/auto-update.txt", FILE_READ);
    if (f) {
      String file_content = f.readString();
      f.close();
      parseScript(file_content);
      g_payload_source = "SD CARD (LOADED)";
      Serial.println("[Payload] Loaded custom Ducky Script from SD Card!");
      return;
    }
  }
  
  g_payload_source = "INTERNAL DEFAULT";
  Serial.println("[Payload] /auto-update.txt not found on SD. Using internal default.");
}

// Render widescreen interface to Sprite and push to LCD (flicker-free)
void drawUI() {
  canvas.fillScreen(kColorBg);

  // Top header bar
  canvas.fillRect(0, 0, 320, 18, kColorHeader);
  canvas.drawFastHLine(0, 18, 320, kColorBorder);
  
  canvas.setTextColor(kColorActive);
  canvas.setTextDatum(top_left);
  canvas.drawString(" DUCKY AUTO-INJECTOR ENGINE", 4, 3);

  // USB OTG Status Indicator
  canvas.setTextDatum(top_right);
  canvas.setTextColor(kColorSuccess);
  
  if (g_sd_mounted) {
    canvas.drawString("USB: DUAL KEY+MSC ", 316, 3);
  } else {
    canvas.drawString("USB: KEYBOARD ONLY ", 316, 3);
  }

  // Full-width Payload Dashboard Container
  canvas.drawRect(5, 24, 310, 124, kColorBorder);
  canvas.fillRect(6, 25, 308, 16, kColorHeader);
  
  canvas.setTextDatum(top_center);
  canvas.setTextColor(kColorActive);
  canvas.drawString("PAYLOAD STATUS DASHBOARD", 160, 28);

  canvas.setTextDatum(top_left);
  canvas.setTextColor(kColorText);
  canvas.drawString("Source:", 15, 48);
  canvas.setTextColor(kColorWarn);
  canvas.drawString(g_payload_source, 75, 48);

  canvas.setTextColor(kColorText);
  canvas.drawString("Status:", 15, 66);
  if (g_injecting) {
    canvas.setTextColor(kColorSuccess);
    canvas.drawString("INJECTING COMMANDS...", 75, 66);
  } else {
    if (g_status_msg == "COMPLETE") {
      canvas.setTextColor(kColorSuccess);
      canvas.drawString("FINISHED SUCCESSFUL", 75, 66);
    } else {
      canvas.setTextColor(kColorActive);
      canvas.drawString(g_status_msg, 75, 66);
    }
  }

  canvas.setTextColor(kColorText);
  canvas.drawString("Cmd:", 15, 84);
  canvas.setTextColor(kColorActive);
  
  // Show active command text with plenty of space on widescreen
  String display_cmd = g_last_sent_action;
  if (display_cmd.length() > 32) display_cmd = display_cmd.substring(0, 30) + "..";
  canvas.drawString(display_cmd, 55, 84);

  // Large Premium Progress Bar
  canvas.drawRect(15, 104, 290, 12, kColorBorder);
  if (g_script_lines.size() > 0) {
    int progress_w = 288 * g_current_line_idx / g_script_lines.size();
    if (g_injecting) {
      canvas.fillRect(16, 105, progress_w, 10, kColorSuccess);
    } else if (g_current_line_idx == (int)g_script_lines.size()) {
      canvas.fillRect(16, 105, 288, 10, kColorSuccess);
    }
  }

  // Help Footer Text
  canvas.setTextColor(kColorMuted);
  canvas.setTextDatum(top_left);
  
  if (g_sd_mounted) {
    canvas.drawString("SD card mounted on PC as 'S3_MSC_PAD' drive", 15, 128);
  } else {
    canvas.drawString("Press CENTER or BTN A to re-inject payload", 15, 128);
  }

  // Push Canvas to Display
  canvas.pushSprite(0, 0);
}

// Perform Ducky Script keystroke injection step-by-step
void runDuckyScript() {
  if (g_script_lines.empty()) return;
  g_injecting = true;
  g_status_msg = "RUNNING";
  g_current_line_idx = 0;
  
  Serial.println("[Ducky] Auto-Injection started!");
  
  for (size_t i = 0; i < g_script_lines.size(); i++) {
    g_current_line_idx = i;
    String line = g_script_lines[i];
    g_last_sent_action = line;
    
    // Refresh GUI for each step
    drawUI();
    Serial.println("[Ducky Step] Executing: " + line);
    
    if (line.startsWith("CTRL-ALT ")) {
      String keyStr = line.substring(9);
      keyStr.trim();
      if (keyStr.length() > 0) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(keyStr[0]);
        delay(120);
        Keyboard.releaseAll();
      }
    } 
    else if (line.startsWith("DELAY ")) {
      int ms = line.substring(6).toInt();
      delay(ms);
    } 
    else if (line.startsWith("STRING ")) {
      String text = line.substring(7);
      Keyboard.print(text.c_str());
    } 
    else if (line == "ENTER") {
      Keyboard.write(KEY_RETURN);
    }
    
    delay(50); // Safe brief pause between commands
  }
  
  g_current_line_idx = g_script_lines.size();
  g_injecting = false;
  g_status_msg = "COMPLETE";
  g_last_sent_action = "PAYLOAD DONE!";
  drawUI();
  Serial.println("[Ducky] Auto-Injection completed successfully.");
}

// Low-level sector read/write callbacks for high-speed USB Mass Storage Class
int32_t usbWriteCallback(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  sdmmc_card_t* card = getSDMMCCard();
  if (!card) return -1;

  uint32_t secSize = card->csd.sector_size;
  if (secSize == 0) return -1;

  uint32_t numSectors = bufsize / secSize;
  esp_err_t err = sdmmc_write_sectors(card, buffer, lba, numSectors);
  if (err != ESP_OK) {
    return -1;
  }
  return bufsize;
}

int32_t usbReadCallback(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  sdmmc_card_t* card = getSDMMCCard();
  if (!card) return -1;

  uint32_t secSize = card->csd.sector_size;
  if (secSize == 0) return -1;

  uint32_t numSectors = bufsize / secSize;
  esp_err_t err = sdmmc_read_sectors(card, buffer, lba, numSectors);
  if (err != ESP_OK) {
    return -1;
  }
  return bufsize;
}

bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject) {
  return true;
}

} // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n==================================");
  Serial.println(" ESP32-S3 KEYBOARD EMULATOR DUCKY ");
  Serial.println("==================================");

  // Initialize LCD
  lcd.init();
  lcd.setRotation(1); // Landscape

  // Turn on Backlight (pin 48)
  pinMode(48, OUTPUT);
  digitalWrite(48, HIGH);

  // Initialize Sprite Canvas
  canvas.createSprite(320, 172);

  // Register Buttons as INPUT_PULLUP
  for (const auto& btn : g_buttons) {
    pinMode(btn.pin, INPUT_PULLUP);
  }

  // Pre-parse embedded script fallback
  parseScript(kDefaultScript);

  // Attempt SDMMC card loading to override payload dynamically
  initSDCard();

  // 2. Setup USB Composite interfaces
  // Initialize USB Mass Storage (MSC) if SD card is successfully mounted
  if (g_sd_mounted) {
    sdmmc_card_t* card = getSDMMCCard();
    if (card) {
      uint32_t secSize = card->csd.sector_size;
      uint32_t numSectors = card->csd.capacity;

      msc.vendorID("ESP32");
      msc.productID("S3_MSC_PAD");
      msc.productRevision("1.0");

      msc.onRead(usbReadCallback);
      msc.onWrite(usbWriteCallback);
      msc.onStartStop(usbStartStopCallback);
      
      msc.mediaPresent(true);
      msc.begin(numSectors, secSize);
      Serial.println("[USB MSC] Exposing SDMMC card via USB Mass Storage Class!");
    }
  }

  // Initialize USB Keyboard
  Keyboard.begin();
  
  // Start Composite USB Device
  USB.begin();

  // Draw initial state showing USB connecting
  g_status_msg = "USB ENUM...";
  g_last_sent_action = "WAITING FOR PC...";
  drawUI();

  // Safe delay for Host OS USB keyboard driver enumeration (2.5 seconds)
  delay(2500);

  // AUTO-RUN script immediately on boot!
  runDuckyScript();
}

void loop() {
  uint32_t now = millis();

  if (now - g_last_poll_ms >= kPollIntervalMs) {
    g_last_poll_ms = now;
    bool state_changed = false;

    // Scan hardware button states
    bool up_pressed     = (digitalRead(kPinSW_UP) == LOW);
    bool down_pressed   = (digitalRead(kPinSW_DOWN) == LOW);
    bool left_pressed   = (digitalRead(kPinSW_LEFT) == LOW);
    bool right_pressed  = (digitalRead(kPinSW_RIGHT) == LOW);
    bool center_pressed = (digitalRead(kPinSW_CENTER) == LOW);
    bool btnA_pressed   = (digitalRead(kPinBTN_A) == LOW);
    bool btnB_pressed   = (digitalRead(kPinBTN_B) == LOW);

    // Update screen visual button layout
    for (auto& btn : g_buttons) {
      bool cur = false;
      if (strcmp(btn.name, "UP") == 0) cur = up_pressed;
      else if (strcmp(btn.name, "DOWN") == 0) cur = down_pressed;
      else if (strcmp(btn.name, "LEFT") == 0) cur = left_pressed;
      else if (strcmp(btn.name, "RIGHT") == 0) cur = right_pressed;
      else if (strcmp(btn.name, "CENTER") == 0) cur = center_pressed;
      else if (strcmp(btn.name, "BTN A") == 0) cur = btnA_pressed;
      else if (strcmp(btn.name, "BTN B") == 0) cur = btnB_pressed;

      if (cur != btn.pressed) {
        btn.pressed = cur;
        state_changed = true;

        // Allows manual RE-TRIGGER on CENTER or BTN A press anytime
        if (cur && !g_injecting) {
          if (strcmp(btn.name, "CENTER") == 0 || strcmp(btn.name, "BTN A") == 0) {
            runDuckyScript();
            state_changed = true;
          }
        }
      }
    }

    if (state_changed) {
      drawUI();
    }
  }
}

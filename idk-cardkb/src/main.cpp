#include <M5Unified.h>
#include <FS.h>
#include <SPI.h>
#include <Wire.h>

namespace {

// Cyberpunk Palette (RGB565)
constexpr uint16_t kBg = 0x0810;       // Cyber Space Dark Violet (R=1, G=4, B=16)
constexpr uint16_t kGrid = 0x1030;     // Subtle cybernetic grid lines
constexpr uint16_t kPink = 0xF81F;     // Neon Pink (borders/tags)
constexpr uint16_t kCyan = 0x07FF;     // Glowing Neon Cyan (active track, highlights)
constexpr uint16_t kYellow = 0xFFE0;   // Neon Yellow (gauges, metrics)
constexpr uint16_t kGreen = 0x07E0;    // Terminal Green (online/success)
constexpr uint16_t kRed = 0xF800;      // Warning Red (offline/error)
constexpr uint16_t kWhite = 0xFFFF;    // Pure White
constexpr uint16_t kDim = 0x7BEF;      // Muted slate gray (inactive elements)

// CardKB I2C Config
constexpr uint8_t kCardKBAddr = 0x5F;
constexpr int kPinSDA = 9;  // Grove Port SDA on M5StickS3
constexpr int kPinSCL = 10; // Grove Port SCL on M5StickS3

// UI Canvas and state
M5Canvas g_canvas(&M5.Display);
bool g_cardkb_online = false;
uint32_t g_last_scan_ms = 0;
constexpr uint32_t kScanIntervalMs = 1000; // scan I2C every 1s

// Terminal scroll buffer
constexpr int kTerminalRows = 6;
constexpr int kTerminalCols = 22;
String g_terminal_lines[kTerminalRows];
int g_term_line_count = 0;
String g_current_line = "";

// Diagnostic data
uint8_t g_last_raw_key = 0;
String g_last_key_name = "NONE";
uint32_t g_key_count = 0;

// Scan I2C bus for CardKB address
bool scanCardKB() {
  Wire.beginTransmission(kCardKBAddr);
  return (Wire.endTransmission() == 0);
}

// Add a line to the scrollable terminal buffer
void pushTerminalLine(const String& line) {
  if (g_term_line_count < kTerminalRows) {
    g_terminal_lines[g_term_line_count] = line;
    g_term_line_count++;
  } else {
    // Scroll up
    for (int i = 1; i < kTerminalRows; ++i) {
      g_terminal_lines[i - 1] = g_terminal_lines[i];
    }
    g_terminal_lines[kTerminalRows - 1] = line;
  }
}

// Process key input from CardKB
void handleKeyInput(uint8_t key) {
  g_last_raw_key = key;
  g_key_count++;

  // Identify special keys
  if (key == 0x08) { // Backspace
    g_last_key_name = "BACKSPACE";
    if (g_current_line.length() > 0) {
      g_current_line.remove(g_current_line.length() - 1);
    }
  } else if (key == 0x09) { // Tab
    g_last_key_name = "TAB";
    g_current_line += "  ";
  } else if (key == 0x0D || key == 0x0A) { // Enter / Newline
    g_last_key_name = "ENTER";
    pushTerminalLine(g_current_line);
    g_current_line = "";
  } else if (key == 0x1B) { // Escape
    g_last_key_name = "ESCAPE";
    g_current_line = "";
    pushTerminalLine("// SYSTEM CLEAR //");
  } else if (key == 0xB5) {
    g_last_key_name = "ARROW UP";
  } else if (key == 0xB6) {
    g_last_key_name = "ARROW DOWN";
  } else if (key == 0xB4) {
    g_last_key_name = "ARROW LEFT";
  } else if (key == 0xB7) {
    g_last_key_name = "ARROW RIGHT";
  } else if (key >= 0x20 && key <= 0x7E) { // Printables
    g_last_key_name = String("'") + (char)key + String("'");
    g_current_line += (char)key;
    if (g_current_line.length() >= kTerminalCols) {
      pushTerminalLine(g_current_line);
      g_current_line = "";
    }
  } else {
    char buf[16];
    snprintf(buf, sizeof(buf), "RAW: 0x%02X", key);
    g_last_key_name = String(buf);
  }
}

// Poll key data from CardKB over I2C
void pollKeyboard() {
  if (!g_cardkb_online) return;

  // Request 1 byte from CardKB
  Wire.requestFrom(kCardKBAddr, (uint8_t)1);
  while (Wire.available()) {
    uint8_t key = Wire.read();
    if (key != 0) {
      handleKeyInput(key);
    }
  }
}

// Draw base cyberpunk grid background
void drawGridAndHeader() {
  g_canvas.fillScreen(kBg);
  
  // Cybernetic grids
  for (int y = 20; y < 120; y += 22) {
    g_canvas.drawFastHLine(0, y, 240, kGrid);
  }
  for (int x = 20; x < 240; x += 30) {
    g_canvas.drawFastVLine(x, 18, 100, kGrid);
  }
  
  // Neon bounding lines
  g_canvas.drawFastHLine(0, 17, 240, kPink);
  g_canvas.drawFastHLine(0, 118, 240, kPink);
  
  // Title bar
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kCyan, kBg);
  g_canvas.drawString("// DECK-KEYBOARD S3 v1.0", 6, 3);
  
  // Battery gauge
  int bat = M5.Power.getBatteryLevel();
  g_canvas.setTextDatum(top_right);
  g_canvas.setTextColor(kYellow, kBg);
  g_canvas.printf("BAT: %d%%", bat);
}

// Render UI Components
void drawUI() {
  drawGridAndHeader();

  // LEFT PANEL: System diagnostics
  g_canvas.drawRoundRect(4, 22, 104, 92, 4, kPink);
  g_canvas.setTextDatum(top_center);
  g_canvas.setTextColor(kPink, kBg);
  g_canvas.drawString("SYS DIAGS", 56, 26);

  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kWhite);
  g_canvas.drawString("CardKB:", 10, 40);
  if (g_cardkb_online) {
    g_canvas.setTextColor(kGreen);
    g_canvas.drawString("ONLINE", 60, 40);
  } else {
    g_canvas.setTextColor(kRed);
    g_canvas.drawString("OFFLINE", 60, 40);
  }

  g_canvas.setTextColor(kWhite);
  g_canvas.drawString("Addr: 0x5F", 10, 52);

  g_canvas.setTextColor(kWhite);
  g_canvas.drawString("Key: ", 10, 68);
  g_canvas.setTextColor(kYellow);
  g_canvas.drawString(g_last_key_name.substring(0, 9), 42, 68);

  g_canvas.setTextColor(kWhite);
  g_canvas.printf("Hex: 0x%02X", g_last_raw_key);
  g_canvas.drawString(" ", 86, 80);

  g_canvas.setTextColor(kDim);
  g_canvas.printf("Count: %u", g_key_count);

  // RIGHT PANEL: Terminal emulator output
  g_canvas.drawRoundRect(112, 22, 124, 92, 4, kCyan);
  g_canvas.setTextDatum(top_center);
  g_canvas.setTextColor(kCyan, kBg);
  g_canvas.drawString("TERMINAL CONSOLE", 174, 26);

  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kGreen);
  
  // Render scroll buffer lines
  int y_pos = 38;
  for (int i = 0; i < g_term_line_count; ++i) {
    g_canvas.drawString(g_terminal_lines[i], 118, y_pos);
    y_pos += 11;
  }

  // Draw current line and blinking cursor
  if (y_pos < 110) {
    g_canvas.drawString(g_current_line, 118, y_pos);
    
    // Blinking green block cursor
    if ((millis() / 400) % 2 == 0) {
      int cursor_x = 118 + g_canvas.textWidth(g_current_line);
      g_canvas.fillRect(cursor_x, y_pos, 6, 9, kGreen);
    }
  }

  // Bottom action hints
  g_canvas.setTextDatum(top_left);
  g_canvas.setTextColor(kCyan);
  g_canvas.drawString("PLUG CARDKB TO PORT.A (I2C 9/10)", 6, 122);
}

} // namespace

void setup() {
  auto cfg = M5.config();
  cfg.fallback_board = m5::board_t::board_M5StickS3;
  M5.begin(cfg);

  M5.Display.setRotation(1); // landscape
  M5.Display.setBrightness(180);

  // Boost Grove Port VCC/5V Output
  M5.Power.setExtOutput(true);

  // Initialize Wire (I2C) on Port.A pins (SDA=9, SCL=10)
  Wire.begin(kPinSDA, kPinSCL, 100000UL); // 100kHz standard mode

  // Initialize Canvas
  g_canvas.createSprite(240, 135);

  // Quick initial I2C check
  g_cardkb_online = scanCardKB();
  g_last_scan_ms = millis();
}

void loop() {
  M5.update();
  uint32_t now = millis();

  // Periodically check if keyboard is online
  if (now - g_last_scan_ms >= kScanIntervalMs) {
    g_cardkb_online = scanCardKB();
    g_last_scan_ms = now;
  }

  // Read incoming characters from I2C keyboard
  pollKeyboard();

  // Mock input using on-board button A
  if (M5.BtnA.wasPressed()) {
    handleKeyInput('M');
    handleKeyInput('5');
    handleKeyInput('S');
    handleKeyInput('3');
    handleKeyInput('!');
  }

  // Mock input using on-board button B
  if (M5.BtnB.wasPressed()) {
    handleKeyInput(0x0D); // Enter key simulation
  }

  // Render and push frame
  drawUI();
  g_canvas.pushSprite(0, 0);

  delay(16);
}

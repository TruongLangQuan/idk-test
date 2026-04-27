#include <M5Unified.h>
#include <SD.h>
#include <SPIFFS.h>
#include <vector>
#include <algorithm>

namespace {

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

struct Button {
  int pin;
  bool pressed;
  uint32_t lastRepeatMs;
};

static Button g_buttons[] = {
    {kPinUp, false, 0},
    {kPinDown, false, 0},
    {kPinLeft, false, 0},
    {kPinRight, false, 0},
    {kPinCenter, false, 0},
};

static constexpr uint32_t kRepeatMs = 200;

// ─── State ─────────────────────────────────────────────────────
enum class ScreenState {
  ROM_LIST,
  ROM_INFO,
  EMULATING,
};

struct RomEntry {
  String filename;
  String displayName;
  uint32_t fileSize;
  bool isLoaded;
};

static std::vector<RomEntry> g_roms;
static ScreenState g_screen = ScreenState::ROM_LIST;
static int g_rom_index = 0;
static int g_rom_page = 0;

// ─── Emulation placeholders (to be implemented) ────────────────
static uint8_t *g_rom_data = nullptr;
static uint32_t g_rom_size = 0;

// ─── Helpers ───────────────────────────────────────────────────

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void scanRoms() {
  g_roms.clear();
  File root = SD.open("/roms");
  if (!root || !root.isDirectory()) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 60);
    M5.Display.print("No /roms folder");
    delay(3000);
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    String nameLower = name;
    nameLower.toLowerCase();
    
    if (nameLower.endsWith(".nes") || nameLower.endsWith(".gb") || 
        nameLower.endsWith(".gba") || nameLower.endsWith(".smc")) {
      String displayName = name.substring(name.lastIndexOf('/') + 1);
      g_roms.push_back({name, displayName, file.size(), false});
    }
    file = root.openNextFile();
  }
  root.close();

  // Sort alphabetically
  std::sort(g_roms.begin(), g_roms.end(), [](const RomEntry& a, const RomEntry& b) {
    return a.displayName < b.displayName;
  });
}

void drawRomList() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);

  M5.Display.setCursor(4, 4);
  M5.Display.printf("idk-gamerom [%d/%d]", g_rom_index + 1, (int)g_roms.size());

  const int itemsPerPage = 7;
  g_rom_page = g_rom_index / itemsPerPage;
  int pageStart = g_rom_page * itemsPerPage;
  int pageEnd = std::min(pageStart + itemsPerPage, (int)g_roms.size());

  for (int i = pageStart; i < pageEnd; ++i) {
    int displayRow = (i - pageStart);
    int y = 20 + displayRow * 15;
    bool isSelected = (i == g_rom_index);

    if (isSelected) {
      M5.Display.fillRect(0, y - 2, 240, 14, 0x0410);  // dark cyan highlight
      M5.Display.setTextColor(TFT_YELLOW, 0x0410);
    } else {
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    }

    String displayText = g_roms[i].displayName;
    if (displayText.length() > 28) {
      displayText = displayText.substring(0, 25) + "...";
    }
    M5.Display.setCursor(4, y);
    M5.Display.print(displayText);
  }

  // Footer
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 125);
  M5.Display.print("UP/DOWN: browse  CENTER: start");
}

void drawRomInfo() {
  if (g_rom_index >= g_roms.size()) return;

  const RomEntry& rom = g_roms[g_rom_index];

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);

  M5.Display.setCursor(4, 4);
  M5.Display.print("ROM Info");

  M5.Display.setCursor(4, 20);
  M5.Display.printf("Name: ");
  M5.Display.println(rom.displayName);

  String format = "UNKNOWN";
  String nameLower = rom.filename;
  nameLower.toLowerCase();
  if (nameLower.endsWith(".nes")) format = "NES";
  else if (nameLower.endsWith(".gb")) format = "GameBoy";
  else if (nameLower.endsWith(".gba")) format = "GameBoy Advance";
  else if (nameLower.endsWith(".smc")) format = "SNES";

  M5.Display.printf("Size: %u KB\n", rom.fileSize / 1024);
  M5.Display.printf("Format: %s\n", format.c_str());
  M5.Display.printf("Status: %s\n", rom.isLoaded ? "IN SPIFFS" : "ON SD CARD");

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(4, 125);
  M5.Display.print("CENTER: load  PWR: back");
}

void handleInput() {
  const uint32_t now = millis();

  auto handleDir = [&](Button& btn, int action) {
    const bool down = readPressed(btn.pin);
    if (!down) {
      btn.pressed = false;
      return;
    }

    bool trigger = false;
    if (!btn.pressed) {
      btn.pressed = true;
      btn.lastRepeatMs = now;
      trigger = true;
    } else if (now - btn.lastRepeatMs > kRepeatMs) {
      btn.lastRepeatMs = now;
      trigger = true;
    }

    if (trigger) {
      if (action == 0 && g_rom_index > 0) g_rom_index--;           // UP
      if (action == 1 && g_rom_index < (int)g_roms.size() - 1) g_rom_index++;  // DOWN
      if (action == 2 && g_rom_index > 6) g_rom_index -= 7;         // LEFT (page up)
      if (action == 3 && g_rom_index + 7 < (int)g_roms.size()) g_rom_index += 7;  // RIGHT (page down)
    }
  };

  handleDir(g_buttons[0], 0);  // UP
  handleDir(g_buttons[1], 1);  // DOWN
  handleDir(g_buttons[2], 2);  // LEFT
  handleDir(g_buttons[3], 3);  // RIGHT

  // CENTER button
  if (readPressed(kPinCenter)) {
    static uint32_t lastPress = 0;
    if (now - lastPress > 300) {
      if (g_screen == ScreenState::ROM_LIST) {
        g_screen = ScreenState::ROM_INFO;
      } else if (g_screen == ScreenState::ROM_INFO) {
        // Copy to SPIFFS logic
        g_screen = ScreenState::EMULATING;
        
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.setCursor(20, 40);
        M5.Display.print("Copying to SPIFFS...");
        
        // Ensure SPIFFS is ready
        if (!SPIFFS.begin(true)) {
          M5.Display.setCursor(20, 60);
          M5.Display.setTextColor(TFT_RED);
          M5.Display.print("SPIFFS format failed");
          delay(2000);
          g_screen = ScreenState::ROM_INFO;
          return;
        }
        
        RomEntry& rom = g_roms[g_rom_index];
        File source = SD.open(rom.filename, FILE_READ);
        if (source) {
          String destName = String("/") + rom.displayName;
          File dest = SPIFFS.open(destName, FILE_WRITE);
          if (dest) {
            uint8_t buf[2048];
            size_t copied = 0;
            while (source.available()) {
              size_t len = source.read(buf, sizeof(buf));
              dest.write(buf, len);
              copied += len;
              
              // Draw progress bar
              if (copied % 16384 == 0 || copied == rom.fileSize) {
                int w = (int)((float)copied / rom.fileSize * 200.0f);
                M5.Display.drawRect(20, 60, 200, 10, TFT_WHITE);
                M5.Display.fillRect(20, 60, w, 10, TFT_GREEN);
              }
            }
            dest.close();
            rom.isLoaded = true;
          }
          source.close();
        }
      }
      lastPress = now;
    }
  }

  // PWR button to go back
  if (M5.BtnPWR.wasPressed()) {
    if (g_screen == ScreenState::ROM_INFO) {
      g_screen = ScreenState::ROM_LIST;
    }
  }
}

void drawScreen() {
  switch (g_screen) {
    case ScreenState::ROM_LIST:
      drawRomList();
      break;
    case ScreenState::ROM_INFO:
      drawRomInfo();
      break;
    case ScreenState::EMULATING:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_CYAN);
      M5.Display.setTextSize(2);
      M5.Display.setCursor(60, 40);
      M5.Display.print("EMULATOR");
      
      M5.Display.setTextSize(1);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(40, 70);
      M5.Display.printf("Running: %s", g_roms[g_rom_index].displayName.c_str());
      
      M5.Display.setTextColor(TFT_RED);
      M5.Display.setCursor(10, 100);
      M5.Display.print("NOTE: Real emulation requires");
      M5.Display.setCursor(10, 115);
      M5.Display.print("additional C/C++ libraries!");
      break;
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  Serial.begin(115200);
  delay(200);

  Serial.println("idk-gamerom starting...");

  // Initialize 5-way pins
  for (auto& btn : g_buttons) {
    pinMode(btn.pin, INPUT_PULLUP);
  }

  // Initialize SD
  if (!SD.begin()) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(10, 60);
    M5.Display.print("SD init failed!");
    delay(3000);
    return;
  }

  // Scan for ROMs
  scanRoms();

  if (g_roms.empty()) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(10, 50);
    M5.Display.print("No .nes files found");
    M5.Display.setCursor(10, 70);
    M5.Display.print("Place ROMs in /roms/");
    delay(3000);
  }

  drawScreen();
}

void loop() {
  M5.update();
  handleInput();
  drawScreen();
  delay(5);
}

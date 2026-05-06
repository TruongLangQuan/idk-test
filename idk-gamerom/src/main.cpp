#include <M5Unified.h>
#include <SD.h>
#include <LittleFS.h>
#include <SPI.h>
#include <vector>
#include <algorithm>

#define ENABLE_SOUND 0
#define ENABLE_LCD 1
#include "peanut_gb.h"

namespace {

// ─── 5-way tactile switch GPIO mapping ──────────────────────────
static constexpr int kPinUp = 32;
static constexpr int kPinDown = 33;
static constexpr int kPinLeft = 25;
static constexpr int kPinRight = 26;
static constexpr int kPinCenter = 0;

M5Canvas canvas(&M5.Display);

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

// ─── Emulation Data ───────────────────────────────────────────────────
static uint8_t *g_rom_data = nullptr;
static uint8_t *g_ram_data = nullptr;
static struct gb_s gb_context;

uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr) {
    return g_rom_data[addr];
}
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr) {
    return g_ram_data[addr];
}
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val) {
    g_ram_data[addr] = val;
}
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val) {
    Serial.printf("GB Error %d: %d\n", gb_err, val);
}
void lcd_draw_line(struct gb_s *gb, const uint8_t *pixels, const uint_fast8_t line) {
    // 160x144. Crop to 160x135 for M5StickC screen (clip 4 top, 5 bottom).
    if (line < 4 || line >= 139) return;
    uint16_t color_line[160];
    const uint16_t palette[4] = { TFT_WHITE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLACK }; 
    for(int x=0; x<160; x++) {
       color_line[x] = palette[pixels[x] & 3];
    }
    canvas.pushImage(40, line - 4, 160, 1, color_line);
}

// ─── Helpers ───────────────────────────────────────────────────

void scanDirectory(File dir) {
  if (!dir) return;
  File file = dir.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String name = file.name();
      if (!name.startsWith("/")) name = String("/") + name;
      
      String nameLower = name;
      nameLower.toLowerCase();
      
      if (nameLower.endsWith(".gb")) { // PeanutGB only supports GB (DMG)
        String displayName = name.substring(name.lastIndexOf('/') + 1);
        bool exists = false;
        for (const auto& r : g_roms) {
          if (r.filename == name) { exists = true; break; }
        }
        if (!exists) {
          g_roms.push_back({name, displayName, file.size(), false});
        }
      }
    }
    file = dir.openNextFile();
  }
}

void scanRoms() {
  g_roms.clear();
  File romsDir = SD.open("/roms");
  if (romsDir && romsDir.isDirectory()) {
    scanDirectory(romsDir);
    romsDir.close();
  }
  File rootDir = SD.open("/");
  if (rootDir && rootDir.isDirectory()) {
    scanDirectory(rootDir);
    rootDir.close();
  }
  std::sort(g_roms.begin(), g_roms.end(), [](const RomEntry& a, const RomEntry& b) {
    return a.displayName < b.displayName;
  });
}

void drawRomList() {
  canvas.fillSprite(TFT_BLACK);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextSize(1);
  canvas.setCursor(4, 4);
  canvas.printf("GameBoy ROMs [%d/%d]", g_rom_index + 1, (int)g_roms.size());

  const int itemsPerPage = 7;
  g_rom_page = g_rom_index / itemsPerPage;
  int pageStart = g_rom_page * itemsPerPage;
  int pageEnd = std::min(pageStart + itemsPerPage, (int)g_roms.size());

  for (int i = pageStart; i < pageEnd; ++i) {
    int displayRow = (i - pageStart);
    int y = 20 + displayRow * 15;
    if (i == g_rom_index) {
      canvas.fillRect(0, y - 2, 240, 14, 0x0410);
      canvas.setTextColor(TFT_YELLOW, 0x0410);
    } else {
      canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    String displayText = g_roms[i].displayName;
    if (displayText.length() > 28) displayText = displayText.substring(0, 25) + "...";
    canvas.setCursor(4, y);
    canvas.print(displayText);
  }
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setCursor(4, 125);
  canvas.print("PWR/B: browse  A: start");
}

void drawRomInfo() {
  if (g_rom_index >= g_roms.size()) return;
  const RomEntry& rom = g_roms[g_rom_index];
  canvas.fillSprite(TFT_BLACK);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setTextSize(1);
  canvas.setCursor(4, 4);
  canvas.print("ROM Info");
  canvas.setCursor(4, 20);
  canvas.printf("Name: ");
  canvas.println(rom.displayName);
  canvas.printf("Size: %u KB\n", rom.fileSize / 1024);
  canvas.printf("Format: GameBoy (DMG)\n");
  canvas.printf("Status: %s\n", rom.isLoaded ? "IN LittleFS" : "ON SD CARD");
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setCursor(4, 125);
  canvas.print("A: LOAD & PLAY  Hold PWR: back");
}

void launchEmulator(const String& destName) {
  File f = LittleFS.open(destName, FILE_READ);
  if (!f) return;

  size_t sz = f.size();
  if (g_rom_data) heap_caps_free(g_rom_data);
  if (g_ram_data) heap_caps_free(g_ram_data);
  
  g_rom_data = (uint8_t*)heap_caps_malloc(sz, MALLOC_CAP_SPIRAM);
  g_ram_data = (uint8_t*)heap_caps_malloc(32768, MALLOC_CAP_SPIRAM); // 32KB RAM max for basic carts
  
  if (!g_rom_data || !g_ram_data) {
      canvas.fillSprite(TFT_BLACK);
      canvas.setTextColor(TFT_RED);
      canvas.setCursor(10, 60);
      canvas.printf("Out of PSRAM!");
      canvas.setCursor(10, 75);
      canvas.printf("Need: %u KB", sz / 1024);
      canvas.pushSprite(0, 0);
      
      if (g_rom_data) { heap_caps_free(g_rom_data); g_rom_data = nullptr; }
      if (g_ram_data) { heap_caps_free(g_ram_data); g_ram_data = nullptr; }
      f.close();
      delay(4000);
      g_screen = ScreenState::ROM_INFO;
      return;
  }
  
  f.read(g_rom_data, sz);
  f.close();
  
  enum gb_init_error_e ret = gb_init(&gb_context, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error, nullptr);
  if (ret == GB_INIT_NO_ERROR) {
      gb_init_lcd(&gb_context, &lcd_draw_line);
      canvas.fillSprite(TFT_BLACK);
      g_screen = ScreenState::EMULATING;
  } else {
      canvas.fillSprite(TFT_BLACK);
      canvas.setTextColor(TFT_RED);
      canvas.setCursor(10, 60);
      canvas.printf("GB Init Error: %d", ret);
      canvas.pushSprite(0, 0);
      delay(3000);
      g_screen = ScreenState::ROM_INFO;
  }
}

void drawScreen() {
  switch (g_screen) {
    case ScreenState::ROM_LIST: drawRomList(); canvas.pushSprite(0, 0); break;
    case ScreenState::ROM_INFO: drawRomInfo(); canvas.pushSprite(0, 0); break;
    case ScreenState::EMULATING: {
      // Input mapping
      gb_context.direct.joypad = 0xFF;
      if (digitalRead(kPinUp) == LOW) gb_context.direct.joypad_bits.up = 0;
      if (digitalRead(kPinDown) == LOW) gb_context.direct.joypad_bits.down = 0;
      if (digitalRead(kPinLeft) == LOW) gb_context.direct.joypad_bits.left = 0;
      if (digitalRead(kPinRight) == LOW) gb_context.direct.joypad_bits.right = 0;
      
      if (M5.BtnA.isPressed()) gb_context.direct.joypad_bits.a = 0;
      if (M5.BtnB.isPressed()) gb_context.direct.joypad_bits.b = 0;
      if (M5.BtnPWR.isPressed()) gb_context.direct.joypad_bits.start = 0;
      if (digitalRead(kPinCenter) == LOW) gb_context.direct.joypad_bits.select = 0;
      
      gb_run_frame(&gb_context);
      canvas.pushSprite(0, 0);
      break;
    }
  }
}

void handleInput() {
  M5.update();

  bool up = M5.BtnPWR.wasPressed();
  bool down = M5.BtnB.wasPressed();
  bool select = M5.BtnA.wasPressed();
  bool back = M5.BtnPWR.pressedFor(500);

  if (g_screen == ScreenState::EMULATING) {
     if (back) {
         g_screen = ScreenState::ROM_LIST;
         delay(500);
     }
     return; // Emulator handles its own inputs per frame
  }

  if (back) {
    if (g_screen == ScreenState::ROM_INFO) {
      g_screen = ScreenState::ROM_LIST;
      delay(200);
    }
    return;
  }

  if (up && g_screen == ScreenState::ROM_LIST && g_rom_index > 0) g_rom_index--;
  if (down && g_screen == ScreenState::ROM_LIST && g_rom_index < (int)g_roms.size() - 1) g_rom_index++;

  if (select) {
    if (g_screen == ScreenState::ROM_LIST && !g_roms.empty()) {
      g_screen = ScreenState::ROM_INFO;
    } else if (g_screen == ScreenState::ROM_INFO) {
      RomEntry& rom = g_roms[g_rom_index];
      
      if (!LittleFS.begin(true)) {
        canvas.fillSprite(TFT_BLACK);
        canvas.setCursor(20, 60);
        canvas.setTextColor(TFT_RED);
        canvas.print("LittleFS format failed");
        canvas.pushSprite(0, 0);
        delay(2000);
        return;
      }

      String destName = String("/") + rom.displayName;
      
      if (rom.isLoaded || LittleFS.exists(destName)) {
        rom.isLoaded = true;
        launchEmulator(destName);
        return;
      }

      canvas.fillSprite(TFT_BLACK);
      canvas.setTextColor(TFT_YELLOW);
      canvas.setCursor(20, 40);
      canvas.print("Copying ROM to RAM/Flash...");
      canvas.pushSprite(0, 0);
      
      File source = SD.open(rom.filename, FILE_READ);
      if (source) {
        File dest = LittleFS.open(destName, FILE_WRITE);
        if (dest) {
          uint8_t buf[2048];
          size_t copied = 0;
          while (source.available()) {
            size_t len = source.read(buf, sizeof(buf));
            dest.write(buf, len);
            copied += len;
            if (copied % 16384 == 0 || copied == rom.fileSize) {
              float pct = (float)copied / rom.fileSize;
              canvas.fillSprite(TFT_BLACK);
              canvas.setTextColor(TFT_YELLOW);
              canvas.setCursor(20, 40);
              canvas.print("Copying ROM...");
              canvas.drawRect(20, 60, 200, 10, TFT_WHITE);
              canvas.fillRect(20, 60, (int)(pct * 200.0f), 10, TFT_GREEN);
              canvas.setTextColor(TFT_WHITE);
              canvas.setCursor(105, 75);
              canvas.printf("%d%%", (int)(pct * 100.0f));
              canvas.pushSprite(0, 0);
            }
          }
          dest.close();
          rom.isLoaded = true;
          delay(500);
          launchEmulator(destName);
        }
        source.close();
      }
    }
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  cfg.internal_mic = false;
  cfg.internal_spk = false;
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);

  Serial.begin(115200);
  delay(200);

  canvas.setPsram(true);
  canvas.setColorDepth(8);
  canvas.createSprite(240, 135);

  SPI.begin(0, 36, 26, 14);
  if (!SD.begin(14, SPI, 15000000)) {
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextColor(TFT_RED);
    canvas.setCursor(10, 60);
    canvas.print("SD init failed!");
    canvas.pushSprite(0, 0);
    delay(2000);
  } else {
    scanRoms();
  }

  if (g_roms.empty()) {
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextColor(TFT_YELLOW);
    canvas.setCursor(10, 50);
    canvas.print("No .gb files found!");
    canvas.setCursor(10, 70);
    canvas.print("Place .gb ROMs in root or /roms");
    canvas.pushSprite(0, 0);
    delay(3000);
  }
}

void loop() {
  handleInput();
  drawScreen();
  delay(1);
}

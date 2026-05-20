#include <M5Unified.h>
#include <SD.h>
#include <LittleFS.h>
#include <SPI.h>
#include <vector>
#include <algorithm>

#define ENABLE_SOUND 0
#define ENABLE_LCD 1
#include "peanut_gb.h"
#include "idk_nes.h"
#include "idk_snes.h"

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

enum class EmulatorKind {
  NONE,
  GAMEBOY,
  NES,
  SNES,
};

struct RomEntry {
  String filename;
  String displayName;
  uint32_t fileSize;
  uint16_t maxKb;
  bool playable;
  const char* console;
  const char* note;
};

struct SnesHeader {
  bool valid;
  String title;
  String mapper;
  uint32_t romKb;
  uint32_t sramKb;
  uint16_t checksum;
};

static std::vector<RomEntry> g_roms;
static ScreenState g_screen = ScreenState::ROM_LIST;
static int g_rom_index = 0;
static int g_rom_page = 0;
static EmulatorKind g_emulator = EmulatorKind::NONE;

// ─── Emulation Data ───────────────────────────────────────────────────
static uint8_t *g_rom_data = nullptr;
static uint8_t *g_ram_data = nullptr;
static struct gb_s gb_context;

struct RomTypeInfo {
  const char* ext;
  const char* console;
  uint16_t maxKb;
  bool playable;
  const char* note;
};

static constexpr RomTypeInfo kRomTypes[] = {
    {".gb", "Game Boy", 1536, true, "Playable: PeanutGB"},
    {".gbc", "Game Boy Color", 1536, true, "Best effort: DMG-compatible"},
    {".nes", "Nintendo NES/Famicom", 4096, true, "Playable: Nofrendo core"},
    {".sfc", "Super Nintendo", 4096, true, "Playable: Snes9x core"},
    {".smc", "Super Nintendo", 4096, true, "Playable: Snes9x core"},
    {".md", "Sega Genesis/Mega Drive", 4096, false, "No Genesis core in repo"},
    {".gen", "Sega Genesis/Mega Drive", 4096, false, "No Genesis core in repo"},
    {".gba", "Game Boy Advance", 8192, false, "No GBA core in repo"},
};

bool isSnesRom(const RomEntry& rom) {
  String lower = rom.filename;
  lower.toLowerCase();
  return lower.endsWith(".sfc") || lower.endsWith(".smc");
}

bool isNesRom(const RomEntry& rom) {
  String lower = rom.filename;
  lower.toLowerCase();
  return lower.endsWith(".nes");
}

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
      String name = file.path();
      if (name.isEmpty()) name = String("/") + file.name();
      if (!name.startsWith("/")) name = String("/") + name;
      
      String nameLower = name;
      nameLower.toLowerCase();
      
      for (const auto& type : kRomTypes) {
        if (nameLower.endsWith(type.ext)) {
          String displayName = name.substring(name.lastIndexOf('/') + 1);
          bool exists = false;
          for (const auto& r : g_roms) {
            if (r.filename == name) { exists = true; break; }
          }
          if (!exists) {
            uint16_t maxKb = type.maxKb;
            if (String(type.ext) == ".nes") {
              maxKb = static_cast<uint16_t>(std::min<uint32_t>(idk_nes_max_rom_kb(), 65535));
            } else if (String(type.ext) == ".sfc" || String(type.ext) == ".smc") {
              maxKb = static_cast<uint16_t>(std::min<uint32_t>(idk_snes_max_rom_kb(), 65535));
            }
            g_roms.push_back({name, displayName, file.size(), maxKb, type.playable, type.console, type.note});
          }
          break;
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
  canvas.printf("Console ROMs [%d/%d]", g_rom_index + 1, (int)g_roms.size());

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

bool readSnesHeaderAt(File& f, uint32_t offset, const char* mapper, SnesHeader& out) {
  if (f.size() < offset + 0x20) return false;
  if (!f.seek(offset)) return false;
  uint8_t buf[32];
  if (f.read(buf, sizeof(buf)) != sizeof(buf)) return false;

  String title = "";
  int printable = 0;
  for (int i = 0; i < 21; ++i) {
    char c = static_cast<char>(buf[i]);
    if (c >= 32 && c <= 126) {
      title += c;
      printable++;
    } else {
      title += ' ';
    }
  }
  title.trim();
  if (printable < 8 || title.length() == 0) return false;

  const uint8_t romExp = buf[0x17];
  const uint8_t ramExp = buf[0x18];
  const uint16_t complement = static_cast<uint16_t>(buf[0x1C]) | (static_cast<uint16_t>(buf[0x1D]) << 8);
  const uint16_t checksum = static_cast<uint16_t>(buf[0x1E]) | (static_cast<uint16_t>(buf[0x1F]) << 8);
  const bool checksumLooksValid = static_cast<uint16_t>(checksum + complement) == 0xFFFF;

  if (!checksumLooksValid && romExp > 0x0D) return false;

  out.valid = true;
  out.title = title;
  out.mapper = mapper;
  out.romKb = (romExp <= 0x0D) ? (1UL << romExp) : 0;
  out.sramKb = (ramExp == 0) ? 0 : (1UL << ramExp);
  out.checksum = checksum;
  return true;
}

SnesHeader readSnesHeader(const String& path) {
  SnesHeader out = {false, "", "", 0, 0, 0};
  File f = SD.open(path, FILE_READ);
  if (!f) return out;

  const uint32_t copierOffset = (f.size() % 1024 == 512) ? 512 : 0;
  SnesHeader lo = out;
  SnesHeader hi = out;
  const bool hasLo = readSnesHeaderAt(f, copierOffset + 0x7FC0, "LoROM", lo);
  const bool hasHi = readSnesHeaderAt(f, copierOffset + 0xFFC0, "HiROM", hi);
  f.close();

  if (hasLo && !hasHi) return lo;
  if (hasHi && !hasLo) return hi;
  if (hasLo && hasHi) return hi.romKb >= lo.romKb ? hi : lo;
  return out;
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
  const uint32_t sizeKb = (rom.fileSize + 1023) / 1024;
  canvas.printf("Size: %u KB\n", sizeKb);
  canvas.printf("Max: %u KB\n", rom.maxKb);
  canvas.printf("Format: %s\n", rom.console);
  canvas.printf("Status: %s\n", rom.playable ? "PLAYABLE" : "BROWSER ONLY");
  canvas.printf("%s\n", rom.note);
  if (isNesRom(rom)) {
    canvas.printf("Safe NES cap: %u KB\n", rom.maxKb);
    canvas.printf("Exit: PWR + CENTER\n");
  }
  if (isSnesRom(rom)) {
    SnesHeader header = readSnesHeader(rom.filename);
    if (header.valid) {
      canvas.printf("Title: %s\n", header.title.c_str());
      canvas.printf("%s ROM:%uK SRAM:%uK\n", header.mapper.c_str(), header.romKb, header.sramKb);
      canvas.printf("CRC: %04X\n", header.checksum);
    } else {
      canvas.printf("SNES header: unknown\n");
    }
  }
  canvas.setTextColor(TFT_DARKGREY, TFT_BLACK);
  canvas.setCursor(4, 125);
  canvas.print("A: play/info  Hold PWR: back");
}

void showMessage(const char* title, const String& msg, uint16_t color = TFT_YELLOW) {
  canvas.fillSprite(TFT_BLACK);
  canvas.setTextColor(color, TFT_BLACK);
  canvas.setCursor(8, 24);
  canvas.print(title);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setCursor(8, 48);
  canvas.print(msg);
  canvas.pushSprite(0, 0);
}

void restoreGamepadPins() {
  pinMode(kPinUp, INPUT_PULLUP);
  pinMode(kPinDown, INPUT_PULLUP);
  pinMode(kPinLeft, INPUT_PULLUP);
  pinMode(kPinRight, INPUT_PULLUP);
  pinMode(kPinCenter, INPUT_PULLUP);
  delay(5);
}

void launchEmulator(const RomEntry& rom) {
  if (!rom.playable) {
    showMessage("Unsupported", String(rom.console) + "\n" + rom.note, TFT_RED);
    delay(2500);
    g_screen = ScreenState::ROM_INFO;
    return;
  }

  const uint32_t sizeKb = (rom.fileSize + 1023) / 1024;
  if (sizeKb > rom.maxKb) {
    showMessage("ROM too large", String(sizeKb) + " KB > max " + String(rom.maxKb) + " KB", TFT_RED);
    delay(3000);
    g_screen = ScreenState::ROM_INFO;
    return;
  }

  if (isSnesRom(rom)) {
    String error;
    showMessage("Loading SNES", rom.displayName, TFT_YELLOW);
    if (idk_snes_begin(rom.filename, error)) {
      canvas.fillSprite(TFT_BLACK);
      g_emulator = EmulatorKind::SNES;
      g_screen = ScreenState::EMULATING;
    } else {
      showMessage("SNES failed", error, TFT_RED);
      delay(3500);
      g_screen = ScreenState::ROM_INFO;
    }
    return;
  }

  if (isNesRom(rom)) {
    String error;
    showMessage("Loading NES", rom.displayName, TFT_YELLOW);
    if (idk_nes_begin(rom.filename, error)) {
      g_emulator = EmulatorKind::NES;
      g_screen = ScreenState::EMULATING;
    } else {
      showMessage("NES failed", error, TFT_RED);
      delay(3500);
      g_screen = ScreenState::ROM_INFO;
    }
    return;
  }

  File f = SD.open(rom.filename, FILE_READ);
  if (!f) return;

  size_t sz = f.size();
  if (g_rom_data) heap_caps_free(g_rom_data);
  if (g_ram_data) heap_caps_free(g_ram_data);
  
  g_rom_data = (uint8_t*)heap_caps_malloc(sz, MALLOC_CAP_SPIRAM);
  g_ram_data = (uint8_t*)heap_caps_malloc(32768, MALLOC_CAP_SPIRAM); // 32KB RAM max for basic carts
  
  if (!g_rom_data || !g_ram_data) {
      showMessage("Out of PSRAM", String("Need: ") + String((sz + 1023) / 1024) + " KB", TFT_RED);
      
      if (g_rom_data) { heap_caps_free(g_rom_data); g_rom_data = nullptr; }
      if (g_ram_data) { heap_caps_free(g_ram_data); g_ram_data = nullptr; }
      f.close();
      delay(4000);
      g_screen = ScreenState::ROM_INFO;
      return;
  }
  
  f.read(g_rom_data, sz);
  f.close();
  restoreGamepadPins();
  
  enum gb_init_error_e ret = gb_init(&gb_context, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error, nullptr);
  if (ret == GB_INIT_NO_ERROR) {
      gb_init_lcd(&gb_context, &lcd_draw_line);
      canvas.fillSprite(TFT_BLACK);
      g_emulator = EmulatorKind::GAMEBOY;
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
      if (g_emulator == EmulatorKind::NES) {
        if (!idk_nes_active()) {
          g_emulator = EmulatorKind::NONE;
          g_screen = ScreenState::ROM_LIST;
          break;
        }
        break;
      }
      if (g_emulator == EmulatorKind::SNES) {
        idk_snes_run_frame(canvas);
        break;
      }

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
         if (g_emulator == EmulatorKind::NES) idk_nes_end();
         if (g_emulator == EmulatorKind::SNES) idk_snes_end();
         g_emulator = EmulatorKind::NONE;
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
      
      launchEmulator(rom);
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
    canvas.print("No console ROMs found!");
    canvas.setCursor(10, 70);
    canvas.print("Use /roms: gb gbc nes sfc md gba");
    canvas.pushSprite(0, 0);
    delay(3000);
  }
}

void loop() {
  handleInput();
  drawScreen();
  delay(1);
}

import os

code = """
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <SD_MMC.h>
#include <FS.h>
#include <vector>
#include <string>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ctype.h>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;
public:
  LGFX(void) {
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_host   = SPI2_HOST;
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
    bus_cfg.spi_mode   = 0;
    bus_cfg.freq_write = 80000000;
    bus_cfg.freq_read  = 16000000;
    bus_cfg.pin_sclk   = 40;
    bus_cfg.pin_mosi   = 45;
    bus_cfg.pin_miso   = -1;
    bus_cfg.pin_dc     = 41;
    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);
    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs           = 42;
    panel_cfg.pin_rst          = 39;
    panel_cfg.pin_busy         = -1;
    panel_cfg.panel_width      = 172;
    panel_cfg.panel_height     = 320;
    panel_cfg.offset_x         = 34;
    panel_cfg.offset_y         = 0;
    panel_cfg.offset_rotation  = 0;
    panel_cfg.dummy_read_bits  = 8;
    panel_cfg.readable         = false;
    panel_cfg.invert           = true;
    panel_cfg.rgb_order        = false;
    _panel_instance.config(panel_cfg);
    auto light_cfg = _light_instance.config();
    light_cfg.pin_bl = 46;
    light_cfg.invert = false;
    light_cfg.freq   = 12000;
    light_cfg.pwm_channel = 7;
    _light_instance.config(light_cfg);
    _panel_instance.setLight(&_light_instance);
    setPanel(&_panel_instance);
  }
};

LGFX M5Display;

constexpr int kPinSD_CLK    = 14;
constexpr int kPinSD_CMD    = 15;
constexpr int kPinSD_D0     = 16;
constexpr int kPinSD_D1     = 18;
constexpr int kPinSD_D2     = 17;
constexpr int kPinSD_D3     = 21;

static constexpr uint8_t IR_PIN = 9;
static constexpr uint8_t IR_PIN_EXT = 9;
static constexpr uint8_t IR_REPEATS = 2;

enum class TxMode {
  TX_INTERNAL,
  TX_EXTERNAL,
};
static TxMode g_tx_mode = TxMode::TX_INTERNAL;

struct IrCommand {
  String name;
  bool is_raw;
  // Parsed
  String protocol;
  uint32_t address;
  uint32_t command;
  // Raw
  uint32_t frequency;
  std::vector<uint16_t> raw_data;
};

static std::vector<IrCommand> g_loaded_commands;
static String g_loaded_filename = "";

enum class ScreenMode {
  DIR_LIST,
  CMD_LIST,
};

static ScreenMode g_mode = ScreenMode::DIR_LIST;
static String g_status = "Ready";

struct DirEntry {
  String name;
  bool isDir;
};
static std::vector<DirEntry> g_dir_entries;
static String g_current_dir = "/";
static int g_dir_cursor = 0;
static int g_cmd_cursor = 0;
static int g_dir_scroll = 0;
static int g_cmd_scroll = 0;

static String joinPath(const String &base, const String &name) {
  if (name.startsWith("/")) return name;
  if (base.length() == 0 || base == "/") return String("/") + name;
  if (base.endsWith("/")) return base + name;
  return base + "/" + name;
}

static String parentPath(const String &path) {
  if (path.length() == 0 || path == "/") return "/";
  int slash = path.lastIndexOf('/');
  if (slash <= 0) return "/";
  return path.substring(0, slash);
}

bool loadDirectory(const String &dirPath) {
  g_dir_entries.clear();
  g_dir_cursor = 0;
  g_dir_scroll = 0;
  
  if (dirPath != "/") {
    g_dir_entries.push_back({"..", true});
  }

  File root = SD_MMC.open(dirPath.c_str());
  if (!root) {
    g_status = "Open dir failed";
    return false;
  }
  if (!root.isDirectory()) {
    root.close();
    return false;
  }

  File f = root.openNextFile();
  while (f && g_dir_entries.size() < 500) {
    String name = f.name();
    int slash = name.lastIndexOf('/');
    if (slash >= 0) {
      name = name.substring(slash + 1);
    }
    if (name.length() > 0) {
      bool isDir = f.isDirectory();
      if (isDir || name.endsWith(".ir") || name.endsWith(".IR")) {
        g_dir_entries.push_back({name, isDir});
      }
    }
    f.close();
    f = root.openNextFile();
  }
  root.close();

  // Sort: dirs first, then files
  std::sort(g_dir_entries.begin(), g_dir_entries.end(), [](const DirEntry& a, const DirEntry& b) {
    if (a.name == "..") return true;
    if (b.name == "..") return false;
    if (a.isDir != b.isDir) return a.isDir;
    return a.name.compareTo(b.name) < 0;
  });

  return true;
}

uint32_t parseHexBytes(const String& str) {
  uint32_t val = 0;
  int shift = 0;
  int len = str.length();
  for (int i = 0; i < len; i += 3) { // Expecting "XX XX XX..."
    String byteStr = str.substring(i, i + 2);
    byteStr.trim();
    if (byteStr.length() > 0) {
      uint32_t b = strtoul(byteStr.c_str(), nullptr, 16);
      val |= (b & 0xFF) << shift;
      shift += 8;
    }
  }
  return val;
}

bool loadIrFile(const String &path) {
  File f = SD_MMC.open(path.c_str(), "r");
  if (!f) return false;
  
  g_loaded_commands.clear();
  g_loaded_filename = path;
  
  IrCommand current_cmd;
  bool in_command = false;

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.isEmpty() || line.startsWith("#")) {
      if (in_command && current_cmd.name.length() > 0) {
        g_loaded_commands.push_back(current_cmd);
      }
      in_command = false;
      continue;
    }
    
    int colon = line.indexOf(':');
    if (colon < 0) continue;
    
    String key = line.substring(0, colon);
    String value = line.substring(colon + 1);
    key.trim();
    value.trim();
    
    if (key == "name") {
      if (in_command && current_cmd.name.length() > 0) {
        g_loaded_commands.push_back(current_cmd);
      }
      current_cmd = IrCommand();
      current_cmd.name = value;
      in_command = true;
    } else if (key == "type") {
      current_cmd.is_raw = (value == "raw");
    } else if (key == "protocol") {
      current_cmd.protocol = value;
    } else if (key == "address") {
      current_cmd.address = parseHexBytes(value);
    } else if (key == "command") {
      current_cmd.command = parseHexBytes(value);
    } else if (key == "frequency") {
      current_cmd.frequency = value.toInt();
    } else if (key == "data") {
      int start = 0;
      while (start < value.length()) {
        int space = value.indexOf(' ', start);
        if (space < 0) space = value.length();
        String numStr = value.substring(start, space);
        if (numStr.length() > 0) {
          current_cmd.raw_data.push_back(numStr.toInt());
        }
        start = space + 1;
      }
    }
  }
  
  if (in_command && current_cmd.name.length() > 0) {
    g_loaded_commands.push_back(current_cmd);
  }
  
  f.close();
  return true;
}

static uint16_t reverseBits16(uint16_t value, uint8_t nbits) {
  uint16_t out = 0;
  for (uint8_t i = 0; i < nbits; ++i) {
    out <<= 1;
    out |= (value & 1);
    value >>= 1;
  }
  return out;
}

void sendCommand(const IrCommand& cmd) {
  uint8_t tx_pin = (g_tx_mode == TxMode::TX_EXTERNAL) ? IR_PIN_EXT : IR_PIN;
  IRsend sender(tx_pin);
  sender.begin();

  if (cmd.is_raw) {
    g_status = "Sending raw: " + cmd.name;
    sender.sendRaw(cmd.raw_data.data(), cmd.raw_data.size(), cmd.frequency / 1000);
  } else {
    g_status = "Sending: " + cmd.name;
    decode_type_t protocol = strToDecodeType(cmd.protocol.c_str());
    if (protocol == decode_type_t::UNKNOWN) {
      g_status = "Unknown protocol: " + cmd.protocol;
      return;
    }

    uint64_t data = cmd.command;
    uint16_t bits = 0;

    switch (protocol) {
      case NEC:
      case NEC_LIKE:
        bits = 32;
        data = reverseBits16(cmd.address, 16) | ((uint64_t)reverseBits16(cmd.command, 16) << 16);
        sender.send(protocol, data, bits, IR_REPEATS);
        break;
      case SAMSUNG:
        bits = 32;
        data = reverseBits16(cmd.address, 16) | ((uint64_t)reverseBits16(cmd.command, 16) << 16);
        sender.send(protocol, data, bits, IR_REPEATS);
        break;
      case SONY:
        bits = 12;
        data = (cmd.command & 0x7F) | ((cmd.address & 0x1F) << 7);
        sender.send(protocol, data, bits, IR_REPEATS);
        break;
      case RC5:
        bits = 12;
        data = (cmd.command & 0x3F) | ((cmd.address & 0x1F) << 6);
        sender.send(protocol, data, bits, IR_REPEATS);
        break;
      default:
        bits = 32;
        sender.send(protocol, data, bits, IR_REPEATS);
        break;
    }
  }

  digitalWrite(tx_pin, LOW);
  g_status = "Sent: " + cmd.name;
}

void drawUI() {
  M5Display.fillScreen(TFT_BLACK);
  M5Display.setTextSize(1);
  M5Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Display.setCursor(2, 2);
  M5Display.println("idk-ir SD");
  M5Display.printf("TX:%s pin:%d\n", g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT",
                    g_tx_mode == TxMode::TX_EXTERNAL ? IR_PIN_EXT : IR_PIN);

  if (g_mode == ScreenMode::DIR_LIST) {
    M5Display.println("Next/Prev hold:fast");
    M5Display.printf("[%s]\n", g_current_dir.c_str());

    const size_t visible = 8;
    const size_t total = g_dir_entries.size() + 1; // +1 for TX toggle

    if (g_dir_cursor >= total) g_dir_cursor = total > 0 ? total - 1 : 0;
    if (g_dir_cursor < g_dir_scroll) g_dir_scroll = g_dir_cursor;
    if (g_dir_cursor >= g_dir_scroll + visible) g_dir_scroll = g_dir_cursor - visible + 1;
    
    size_t end = g_dir_scroll + visible;
    if (end > total) end = total;
    
    for (size_t i = g_dir_scroll; i < end; ++i) {
      const bool selected = (i == g_dir_cursor);
      M5Display.setTextColor(selected ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      
      if (i == 0) {
        M5Display.printf("%c [TX:%s]\n", selected ? '>' : ' ',
                          g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT");
      } else {
        const auto& entry = g_dir_entries[i - 1];
        String dname = entry.name;
        if (dname.length() > 22) dname = dname.substring(0, 22) + "..";
        if (entry.isDir) {
          M5Display.printf("%c [%s]\n", selected ? '>' : ' ', dname.c_str());
        } else {
          M5Display.printf("%c %s\n", selected ? '>' : ' ', dname.c_str());
        }
      }
    }
  } else {
    String fname = g_loaded_filename;
    int slash = fname.lastIndexOf('/');
    if (slash >= 0) fname = fname.substring(slash + 1);
    
    M5Display.printf("File: %s\n", fname.c_str());
    if (g_loaded_commands.empty()) {
      M5Display.println("No commands in file");
    } else {
      M5Display.println("Next:down Prev:up");
      M5Display.println("Send:press Back:hold left");
      
      const size_t visible = 6;
      const size_t total = g_loaded_commands.size();
      if (g_cmd_cursor < g_cmd_scroll) g_cmd_scroll = g_cmd_cursor;
      if (g_cmd_cursor >= g_cmd_scroll + visible) g_cmd_scroll = g_cmd_cursor - visible + 1;
      
      size_t end = g_cmd_scroll + visible;
      if (end > total) end = total;
      
      for (size_t i = g_cmd_scroll; i < end; ++i) {
        const auto& cmd = g_loaded_commands[i];
        M5Display.setTextColor(i == g_cmd_cursor ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        String cname = cmd.name;
        if (cname.length() > 24) cname = cname.substring(0, 24) + "..";
        M5Display.printf("%c %s\n", i == g_cmd_cursor ? '>' : ' ', cname.c_str());
      }
    }
  }

  M5Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Display.fillRect(0, 122, 240, 13, TFT_BLACK);
  M5Display.setCursor(2, 124);
  M5Display.print(g_status);
}

void setup() {
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  M5Display.init();
  M5Display.setRotation(1);
  M5Display.setBrightness(180);

  SD_MMC.setPins(kPinSD_CLK, kPinSD_CMD, kPinSD_D0, kPinSD_D1, kPinSD_D2, kPinSD_D3);
  if (!SD_MMC.begin("/sdcard", true, false, 40000000)) {
    g_status = "SD Mount Failed";
  } else {
    g_status = "SD Mounted";
    loadDirectory("/");
  }

  drawUI();
}

void loop() {
  static bool btn_press_last = false;
  static bool btn_right_last = false;
  static bool btn_left_last = false;
  static bool btn_up_last = false;
  static bool btn_down_last = false;
  
  bool btn_press_raw = (digitalRead(5) == LOW);
  bool btn_right_raw = (digitalRead(4) == LOW);
  bool btn_left_raw = (digitalRead(3) == LOW);
  bool btn_up_raw = (digitalRead(0) == LOW);
  bool btn_down_raw = (digitalRead(2) == LOW);
  
  bool btn_press = btn_press_raw;
  bool btn_right = btn_right_raw || btn_down_raw;
  bool btn_left = btn_left_raw || btn_up_raw;
  
  bool select = btn_press && !btn_press_last;
  bool right_pressed = btn_right && !btn_right_last;
  bool left_pressed = btn_left && !btn_left_last;
  
  btn_press_last = btn_press;
  btn_right_last = btn_right;
  btn_left_last = btn_left;

  if (g_mode == ScreenMode::DIR_LIST) {
    const size_t total_items = g_dir_entries.size() + 1;
    static bool nextWasPressed = false;
    static uint32_t nextPressTime = 0;
    static uint32_t lastNextRepeat = 0;
    static bool prevWasPressed = false;
    static uint32_t prevPressTime = 0;
    static uint32_t lastPrevRepeat = 0;

    if (right_pressed) {
      nextWasPressed = true;
      nextPressTime = millis();
      g_dir_cursor = (g_dir_cursor + 1) % total_items;
      drawUI();
    } else if (btn_right && nextWasPressed) {
      if (millis() - nextPressTime > 220 && millis() - lastNextRepeat > 80) {
        g_dir_cursor = (g_dir_cursor + 1) % total_items;
        lastNextRepeat = millis();
        drawUI();
      }
    } else if (left_pressed && !btn_press) {
      prevWasPressed = true;
      prevPressTime = millis();
      g_dir_cursor = (g_dir_cursor + total_items - 1) % total_items;
      drawUI();
    } else if (btn_left && !btn_press && prevWasPressed) {
      if (millis() - prevPressTime > 220 && millis() - lastPrevRepeat > 80) {
        g_dir_cursor = (g_dir_cursor + total_items - 1) % total_items;
        lastPrevRepeat = millis();
        drawUI();
      }
    } else if (select) {
      if (g_dir_cursor == 0) {
        g_tx_mode = (g_tx_mode == TxMode::TX_INTERNAL) ? TxMode::TX_EXTERNAL : TxMode::TX_INTERNAL;
        g_status = (g_tx_mode == TxMode::TX_EXTERNAL) ? "TX -> EXT pin 9" : "TX -> INT pin 9";
        drawUI();
      } else {
        const auto& entry = g_dir_entries[g_dir_cursor - 1];
        if (entry.isDir) {
          if (entry.name == "..") {
            g_current_dir = parentPath(g_current_dir);
          } else {
            g_current_dir = joinPath(g_current_dir, entry.name);
          }
          if (loadDirectory(g_current_dir)) {
            g_status = "Loaded dir";
          }
          drawUI();
        } else {
          String path = joinPath(g_current_dir, entry.name);
          g_status = "Loading " + entry.name;
          drawUI();
          
          if (loadIrFile(path)) {
            g_mode = ScreenMode::CMD_LIST;
            g_cmd_cursor = 0;
            g_cmd_scroll = 0;
            g_status = "Loaded " + String(g_loaded_commands.size()) + " cmds";
          } else {
            g_status = "Failed to load " + entry.name;
          }
          drawUI();
        }
      }
      delay(120);
    }
    if (!btn_right) nextWasPressed = false;
    if (!btn_left || btn_press) prevWasPressed = false;
  } else {
    static bool nextWasPressed = false;
    static uint32_t nextPressTime = 0;
    static uint32_t lastNextRepeat = 0;
    static bool prevWasPressed = false;
    static uint32_t prevPressTime = 0;
    static uint32_t lastPrevRepeat = 0;

    size_t cmd_count = g_loaded_commands.size();

    if (right_pressed && cmd_count > 0) {
      nextWasPressed = true;
      nextPressTime = millis();
      g_cmd_cursor = (g_cmd_cursor + 1) % cmd_count;
      drawUI();
    } else if (btn_right && nextWasPressed && cmd_count > 0) {
      if (millis() - nextPressTime > 220 && millis() - lastNextRepeat > 80) {
        g_cmd_cursor = (g_cmd_cursor + 1) % cmd_count;
        lastNextRepeat = millis();
        drawUI();
      }
    } else if (left_pressed && !btn_press && cmd_count > 0) {
      prevWasPressed = true;
      prevPressTime = millis();
      g_cmd_cursor = (g_cmd_cursor + cmd_count - 1) % cmd_count;
      drawUI();
    } else if (btn_left && !btn_press && prevWasPressed && cmd_count > 0) {
      if (millis() - prevPressTime > 220 && millis() - lastPrevRepeat > 80) {
        g_cmd_cursor = (g_cmd_cursor + cmd_count - 1) % cmd_count;
        lastPrevRepeat = millis();
        drawUI();
      }
    } else if (select) {
      if (cmd_count > 0) {
        sendCommand(g_loaded_commands[g_cmd_cursor]);
      }
      drawUI();
      delay(120);
    }

    if (btn_left && (millis() - prevPressTime > 700)) {
      g_mode = ScreenMode::DIR_LIST;
      g_status = "Back to files";
      drawUI();
      delay(180);
    }
    if (!btn_right) nextWasPressed = false;
    if (!btn_left || btn_press) prevWasPressed = false;
  }

  delay(10);
}
"""
with open("idk-ir-ws-1.47/src/main.cpp", "w") as f:
    f.write(code)

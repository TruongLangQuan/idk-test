#include <M5Unified.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ctype.h>
#include "generated/ir_assets.h"

static constexpr uint8_t IR_PIN = 19;
static constexpr uint8_t IR_PIN_EXT = 26;
static constexpr uint8_t IR_REPEATS = 2;

enum class TxMode {
  TX_INTERNAL,
  TX_EXTERNAL,
};

static TxMode g_tx_mode = TxMode::TX_INTERNAL;

enum class ScreenMode {
  FILE_LIST,
  CMD_LIST,
};

static ScreenMode g_mode = ScreenMode::FILE_LIST;
static size_t g_file_idx = 0;
static size_t g_file_cursor = 0;  // 0 = TX toggle, 1 = Search, 2.. = filtered file items
static size_t g_cmd_idx = 0;
static size_t g_file_scroll = 0;
static size_t g_cmd_scroll = 0;
static String g_status = "Ready";
static String g_search_query = "";
static size_t g_filtered_count = 0;
static size_t g_filtered_indices[IR_FILE_COUNT > 0 ? IR_FILE_COUNT : 1];

static uint16_t reverseBits16(uint16_t value, uint8_t nbits) {
  uint16_t out = 0;
  for (uint8_t i = 0; i < nbits; ++i) {
    out <<= 1;
    out |= (value & 1);
    value >>= 1;
  }
  return out;
}

bool containsIgnoreCase(const char* text, const String& query) {
  if (!text) return false;
  if (query.isEmpty()) return true;
  const int qlen = query.length();
  if (qlen <= 0) return true;
  const int tlen = strlen(text);
  if (tlen < qlen) return false;

  for (int i = 0; i <= tlen - qlen; ++i) {
    bool ok = true;
    for (int j = 0; j < qlen; ++j) {
      const char a = static_cast<char>(tolower(static_cast<unsigned char>(text[i + j])));
      const char b = static_cast<char>(tolower(static_cast<unsigned char>(query[j])));
      if (a != b) {
        ok = false;
        break;
      }
    }
    if (ok) return true;
  }
  return false;
}

void applyFileFilter() {
  g_filtered_count = 0;
  for (size_t i = 0; i < IR_FILE_COUNT; ++i) {
    if (containsIgnoreCase(IR_FILES[i].name, g_search_query)) {
      g_filtered_indices[g_filtered_count++] = i;
    }
  }
  if (g_filtered_count == 0) {
    g_file_idx = 0;
    if (g_file_cursor > 1) g_file_cursor = 1;
  } else {
    if (g_file_idx >= g_filtered_count) g_file_idx = 0;
    if (g_file_cursor > g_filtered_count + 1) g_file_cursor = g_filtered_count + 1;
  }
  g_file_scroll = 0;
}

bool inputSearchQuery(String& out) {
  const char* row1 = "1234567890";
  const char* row2 = "abcdefghijklmnopqrstuvwxyz";
  const char* row3 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char* row4 = "!@#$%^&*()-_=+[]{};:,.?/\\";
  const char* top[] = {"OK", "DEL", "SPACE", "CANCEL"};
  constexpr int topCount = 4;

  int row = 0;
  int col = 0;
  String value = out;

  auto rowLen = [&](int r) -> int {
    if (r == 0) return topCount;
    if (r == 1) return static_cast<int>(strlen(row1));
    if (r == 2) return static_cast<int>(strlen(row2));
    if (r == 3) return static_cast<int>(strlen(row3));
    return static_cast<int>(strlen(row4));
  };

  auto draw = [&]() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, 2);
    M5.Display.println("Search Remote");
    M5.Display.setCursor(2, 14);
    M5.Display.print("Len: ");
    M5.Display.print(value.length());

    M5.Display.drawRect(2, 24, 236, 12, TFT_DARKGREY);
    M5.Display.setCursor(4, 26);
    M5.Display.print(value);

    int y = 42;
    for (int i = 0; i < topCount; ++i) {
      M5.Display.setTextColor((row == 0 && col == i) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(4 + i * 58, y);
      M5.Display.print(top[i]);
    }

    auto drawRow = [&](int rr, const char* text, int yy) {
      int n = strlen(text);
      for (int i = 0; i < n && i < 28; ++i) {
        M5.Display.setTextColor((row == rr && col == i) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(4 + (i % 14) * 16, yy + (i / 14) * 10);
        M5.Display.print(text[i]);
      }
    };
    drawRow(1, row1, 56);
    drawRow(2, row2, 68);
    drawRow(3, row3, 88);
    drawRow(4, row4, 108);
  };

  draw();
  while (true) {
    M5.update();
    if (M5.BtnPWR.wasPressed()) {
      row = (row + 1) % 5;
      int mx = rowLen(row);
      if (col >= mx) col = mx - 1;
      if (col < 0) col = 0;
      draw();
    }
    if (M5.BtnB.wasPressed()) {
      int mx = rowLen(row);
      col = (col + 1) % mx;
      draw();
    }
    if (M5.BtnA.wasPressed()) {
      if (row == 0) {
        if (col == 0) { out = value; return true; }
        if (col == 1) { if (!value.isEmpty()) value.remove(value.length() - 1); }
        if (col == 2) { if (value.length() < 63) value += ' '; }
        if (col == 3) { return false; }
      } else {
        char ch = 0;
        if (row == 1) ch = row1[col];
        if (row == 2) ch = row2[col];
        if (row == 3) ch = row3[col];
        if (row == 4) ch = row4[col];
        if (ch && value.length() < 63) value += ch;
      }
      draw();
    }
    delay(8);
  }
}

void sendCommand(const IrCommand& cmd) {
  const uint8_t tx_pin = (g_tx_mode == TxMode::TX_EXTERNAL) ? IR_PIN_EXT : IR_PIN;
  IRsend sender(tx_pin);
  pinMode(tx_pin, OUTPUT);
  digitalWrite(tx_pin, LOW);
  sender.begin();

  String protocol = cmd.protocol;
  protocol.toUpperCase();
  String type = cmd.type;
  type.toUpperCase();

  if (type == "RAW" && cmd.raw_data && cmd.raw_len > 0) {
    uint16_t* tmp = static_cast<uint16_t*>(malloc(sizeof(uint16_t) * cmd.raw_len));
    if (!tmp) {
      g_status = "RAW alloc failed";
      return;
    }
    for (size_t i = 0; i < cmd.raw_len; ++i) tmp[i] = cmd.raw_data[i];
    const uint32_t freq = cmd.frequency > 0 ? cmd.frequency : 38000;
    sender.sendRaw(tmp, cmd.raw_len, freq);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) sender.sendRaw(tmp, cmd.raw_len, freq);
    free(tmp);
    digitalWrite(tx_pin, LOW);
    g_status = "Sent RAW " + String(cmd.name) + (g_tx_mode == TxMode::TX_EXTERNAL ? " [EXT]" : " [INT]");
    return;
  }

  if (protocol.indexOf("NECEXT") >= 0) {
    uint16_t addr16 = static_cast<uint16_t>(cmd.address & 0xFFFFu);
    uint16_t cmd16 = static_cast<uint16_t>(cmd.command & 0xFFFFu);
    uint16_t lsbAddr = reverseBits16((addr16 >> 8) | (addr16 << 8), 16);
    uint16_t lsbCmd = reverseBits16((cmd16 >> 8) | (cmd16 << 8), 16);
    uint32_t data = (static_cast<uint32_t>(lsbAddr) << 16) | lsbCmd;
    sender.sendNEC(data, 32);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) sender.sendNEC(data, 32);
    digitalWrite(tx_pin, LOW);
    g_status = "Sent NECext " + String(cmd.name);
    return;
  }

  if (protocol.indexOf("NEC") >= 0) {
    uint16_t addr8 = static_cast<uint8_t>(cmd.address & 0xFFu);
    uint16_t cmd8 = static_cast<uint8_t>(cmd.command & 0xFFu);
    uint64_t data = sender.encodeNEC(addr8, cmd8);
    sender.sendNEC(data, 32);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) sender.sendNEC(data, 32);
    digitalWrite(tx_pin, LOW);
    g_status = "Sent NEC " + String(cmd.name);
    return;
  }

  if (protocol.indexOf("SIRC15") >= 0) {
    uint32_t data = ((cmd.address & 0xFFu) << 7) | (cmd.command & 0x7Fu);
    sender.sendSony(data, 15, 2);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) sender.sendSony(data, 15, 2);
    digitalWrite(tx_pin, LOW);
    g_status = "Sent SIRC15 " + String(cmd.name);
    return;
  }

  if (protocol.indexOf("SIRC") >= 0 || protocol.indexOf("SONY") >= 0) {
    uint32_t data = ((cmd.address & 0x1Fu) << 7) | (cmd.command & 0x7Fu);
    sender.sendSony(data, 12, 2);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) sender.sendSony(data, 12, 2);
    digitalWrite(tx_pin, LOW);
    g_status = "Sent SIRC " + String(cmd.name);
    return;
  }

  if (protocol.length() > 0) {
    decode_type_t proto = strToDecodeType(protocol.c_str());
    if (proto != decode_type_t::UNKNOWN) {
      uint16_t bits = IRsend::defaultBits(proto);
      if (bits == 0) bits = 32;
      uint64_t value = ((uint64_t)(cmd.address & 0xFFFFFFFFu) << 32) | (cmd.command & 0xFFFFFFFFu);
      if (bits <= 32) value = ((uint64_t)(cmd.address & 0xFFFFu) << 16) | (cmd.command & 0xFFFFu);
      bool ok = sender.send(proto, value, bits);
      for (uint8_t i = 0; i < IR_REPEATS; ++i) ok = sender.send(proto, value, bits) || ok;
      digitalWrite(tx_pin, LOW);
      g_status = ok ? ("Sent " + protocol + " " + String(cmd.name)) : ("Send fail " + protocol);
      return;
    }
  }

  digitalWrite(tx_pin, LOW);
  g_status = "Unsupported protocol";
}

void drawUI() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.println("idk-ir");
  M5.Display.printf("TX:%s pin:%d\n", g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT",
                    g_tx_mode == TxMode::TX_EXTERNAL ? IR_PIN_EXT : IR_PIN);

  if (IR_FILE_COUNT == 0) {
    M5.Display.println("No IR file compiled");
    return;
  }

  if (g_mode == ScreenMode::FILE_LIST) {
    M5.Display.println("Next/Prev hold:fast");
    M5.Display.printf("Q:%s\n", g_search_query.isEmpty() ? "(all)" : g_search_query.c_str());
    M5.Display.printf("Found:%u\n", static_cast<unsigned>(g_filtered_count));

    const size_t total_items = g_filtered_count + 2;  // TX + Search + file items
    const size_t visible = 8;
    if (g_file_cursor >= total_items) g_file_cursor = total_items - 1;
    if (g_file_cursor < g_file_scroll) g_file_scroll = g_file_cursor;
    if (g_file_cursor >= g_file_scroll + visible) g_file_scroll = g_file_cursor - visible + 1;
    size_t end = g_file_scroll + visible;
    if (end > total_items) end = total_items;
    for (size_t i = g_file_scroll; i < end; ++i) {
      const bool selected = (i == g_file_cursor);
      M5.Display.setTextColor(selected ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      if (i == 0) {
        M5.Display.printf("%c [TX:%s]\n", selected ? '>' : ' ',
                          g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT");
      } else if (i == 1) {
        M5.Display.printf("%c [Search]\n", selected ? '>' : ' ');
      } else {
        const auto& f = IR_FILES[g_filtered_indices[i - 2]];
        M5.Display.printf("%c %s\n", selected ? '>' : ' ', f.name);
      }
    }
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  } else {
    const auto& file = IR_FILES[g_filtered_indices[g_file_idx]];
    M5.Display.printf("%s\n", file.name);
    if (file.command_count == 0) {
      M5.Display.println("No command");
    } else {
      M5.Display.println("Next:down Prev:up M5:send");
      const size_t visible = 7;
      if (g_cmd_idx < g_cmd_scroll) g_cmd_scroll = g_cmd_idx;
      if (g_cmd_idx >= g_cmd_scroll + visible) g_cmd_scroll = g_cmd_idx - visible + 1;
      size_t end = g_cmd_scroll + visible;
      if (end > file.command_count) end = file.command_count;
      for (size_t i = g_cmd_scroll; i < end; ++i) {
        const auto& cmd = file.commands[i];
        M5.Display.setTextColor(i == g_cmd_idx ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.printf("%c %s\n", i == g_cmd_idx ? '>' : ' ', cmd.name);
      }
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    }
  }

  M5.Display.fillRect(0, 122, 240, 13, TFT_BLACK);
  M5.Display.setCursor(2, 124);
  M5.Display.print(g_status);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  applyFileFilter();
  g_status = "Use [TX] to switch output";
  drawUI();
}

void loop() {
  M5.update();
  if (IR_FILE_COUNT == 0) {
    delay(100);
    return;
  }

  const bool select = M5.BtnA.wasPressed();
  if (g_mode == ScreenMode::FILE_LIST) {
    const size_t total_items = g_filtered_count + 2;
    static bool nextWasPressed = false;
    static uint32_t nextPressTime = 0;
    static uint32_t lastNextRepeat = 0;
    static bool prevWasPressed = false;
    static uint32_t prevPressTime = 0;
    static uint32_t lastPrevRepeat = 0;

    if (M5.BtnB.wasPressed()) {
      nextWasPressed = true;
      nextPressTime = millis();
      g_file_cursor = (g_file_cursor + 1) % total_items;
      drawUI();
    } else if (M5.BtnB.isPressed() && nextWasPressed) {
      if (millis() - nextPressTime > 220 && millis() - lastNextRepeat > 80) {
        g_file_cursor = (g_file_cursor + 1) % total_items;
        lastNextRepeat = millis();
        drawUI();
      }
    } else if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
      prevWasPressed = true;
      prevPressTime = millis();
      g_file_cursor = (g_file_cursor + total_items - 1) % total_items;
      drawUI();
    } else if (M5.BtnPWR.isPressed() && !M5.BtnA.isPressed() && prevWasPressed) {
      if (millis() - prevPressTime > 220 && millis() - lastPrevRepeat > 80) {
        g_file_cursor = (g_file_cursor + total_items - 1) % total_items;
        lastPrevRepeat = millis();
        drawUI();
      }
    } else if (select) {
      if (g_file_cursor == 0) {
        g_tx_mode = (g_tx_mode == TxMode::TX_INTERNAL) ? TxMode::TX_EXTERNAL : TxMode::TX_INTERNAL;
        g_status = (g_tx_mode == TxMode::TX_EXTERNAL) ? "TX -> EXT pin 26" : "TX -> INT pin 19";
      } else if (g_file_cursor == 1) {
        String query = g_search_query;
        if (inputSearchQuery(query)) {
          g_search_query = query;
          applyFileFilter();
          g_status = "Search updated";
        } else {
          g_status = "Search cancelled";
        }
      } else {
        g_file_idx = g_file_cursor - 2;
        g_mode = ScreenMode::CMD_LIST;
        g_cmd_idx = 0;
        g_cmd_scroll = 0;
        g_status = "Select command";
      }
      drawUI();
      delay(120);
    }
    if (!M5.BtnB.isPressed()) nextWasPressed = false;
    if (!M5.BtnPWR.isPressed() || M5.BtnA.isPressed()) prevWasPressed = false;
  } else {
    const auto& file = IR_FILES[g_filtered_indices[g_file_idx]];
    static bool nextWasPressed = false;
    static uint32_t nextPressTime = 0;
    static uint32_t lastNextRepeat = 0;
    static bool prevWasPressed = false;
    static uint32_t prevPressTime = 0;
    static uint32_t lastPrevRepeat = 0;

    if (M5.BtnB.wasPressed() && file.command_count > 0) {
      nextWasPressed = true;
      nextPressTime = millis();
      g_cmd_idx = (g_cmd_idx + 1) % file.command_count;
      drawUI();
    } else if (M5.BtnB.isPressed() && nextWasPressed && file.command_count > 0) {
      if (millis() - nextPressTime > 220 && millis() - lastNextRepeat > 80) {
        g_cmd_idx = (g_cmd_idx + 1) % file.command_count;
        lastNextRepeat = millis();
        drawUI();
      }
    } else if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed() && file.command_count > 0) {
      prevWasPressed = true;
      prevPressTime = millis();
      g_cmd_idx = (g_cmd_idx + file.command_count - 1) % file.command_count;
      drawUI();
    } else if (M5.BtnPWR.isPressed() && !M5.BtnA.isPressed() && prevWasPressed && file.command_count > 0) {
      if (millis() - prevPressTime > 220 && millis() - lastPrevRepeat > 80) {
        g_cmd_idx = (g_cmd_idx + file.command_count - 1) % file.command_count;
        lastPrevRepeat = millis();
        drawUI();
      }
    } else if (select) {
      if (file.command_count > 0) {
        sendCommand(file.commands[g_cmd_idx]);
      }
      drawUI();
      delay(120);
    }

    if (M5.BtnPWR.pressedFor(700)) {
      g_mode = ScreenMode::FILE_LIST;
      g_file_cursor = g_file_idx + 2;
      g_status = "Back to files";
      drawUI();
      delay(180);
    }
    if (!M5.BtnB.isPressed()) nextWasPressed = false;
    if (!M5.BtnPWR.isPressed() || M5.BtnA.isPressed()) prevWasPressed = false;
  }

  delay(10);
}

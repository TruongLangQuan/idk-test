#include <M5Unified.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include "generated/ir_assets.h"

static constexpr uint8_t IR_PIN = 19;
static constexpr uint8_t IR_REPEATS = 2;
static IRsend g_irsend(IR_PIN);

enum class ScreenMode {
  FILE_LIST,
  CMD_LIST,
};

static ScreenMode g_mode = ScreenMode::FILE_LIST;
static size_t g_file_idx = 0;
static size_t g_cmd_idx = 0;
static size_t g_file_scroll = 0;
static size_t g_cmd_scroll = 0;
static String g_status = "Ready";

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
    g_irsend.sendRaw(tmp, cmd.raw_len, cmd.frequency);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) g_irsend.sendRaw(tmp, cmd.raw_len, cmd.frequency);
    free(tmp);
    g_status = "Sent RAW " + String(cmd.name);
    return;
  }

  if (protocol.indexOf("NECEXT") >= 0) {
    uint16_t addr16 = static_cast<uint16_t>(cmd.address & 0xFFFFu);
    uint16_t cmd16 = static_cast<uint16_t>(cmd.command & 0xFFFFu);
    uint16_t lsbAddr = reverseBits16((addr16 >> 8) | (addr16 << 8), 16);
    uint16_t lsbCmd = reverseBits16((cmd16 >> 8) | (cmd16 << 8), 16);
    uint32_t data = (static_cast<uint32_t>(lsbAddr) << 16) | lsbCmd;
    g_irsend.sendNEC(data, 32);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) g_irsend.sendNEC(data, 32);
    g_status = "Sent NECext " + String(cmd.name);
    return;
  }

  if (protocol.indexOf("NEC") >= 0) {
    uint16_t addr8 = static_cast<uint8_t>(cmd.address & 0xFFu);
    uint16_t cmd8 = static_cast<uint8_t>(cmd.command & 0xFFu);
    uint64_t data = g_irsend.encodeNEC(addr8, cmd8);
    g_irsend.sendNEC(data, 32);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) g_irsend.sendNEC(data, 32);
    g_status = "Sent NEC " + String(cmd.name);
    return;
  }

  if (protocol.indexOf("SIRC15") >= 0) {
    uint32_t data = ((cmd.address & 0xFFu) << 7) | (cmd.command & 0x7Fu);
    g_irsend.sendSony(data, 15, 2);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) g_irsend.sendSony(data, 15, 2);
    g_status = "Sent SIRC15 " + String(cmd.name);
    return;
  }

  if (protocol.indexOf("SIRC") >= 0 || protocol.indexOf("SONY") >= 0) {
    uint32_t data = ((cmd.address & 0x1Fu) << 7) | (cmd.command & 0x7Fu);
    g_irsend.sendSony(data, 12, 2);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) g_irsend.sendSony(data, 12, 2);
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
      bool ok = g_irsend.send(proto, value, bits);
      for (uint8_t i = 0; i < IR_REPEATS; ++i) ok = g_irsend.send(proto, value, bits) || ok;
      g_status = ok ? ("Sent " + protocol + " " + String(cmd.name)) : ("Send fail " + protocol);
      return;
    }
  }

  g_status = "Unsupported protocol";
}

void drawUI() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.println("idk-ir");

  if (IR_FILE_COUNT == 0) {
    M5.Display.println("No IR file compiled");
    return;
  }

  if (g_mode == ScreenMode::FILE_LIST) {
    M5.Display.println("Next:down Prev:up M5:open");
    const size_t visible = 8;
    if (g_file_idx < g_file_scroll) g_file_scroll = g_file_idx;
    if (g_file_idx >= g_file_scroll + visible) g_file_scroll = g_file_idx - visible + 1;
    size_t end = g_file_scroll + visible;
    if (end > IR_FILE_COUNT) end = IR_FILE_COUNT;
    for (size_t i = g_file_scroll; i < end; ++i) {
      const auto& f = IR_FILES[i];
      M5.Display.setTextColor(i == g_file_idx ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      M5.Display.printf("%c %s\n", i == g_file_idx ? '>' : ' ', f.name);
    }
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  } else {
    const auto& file = IR_FILES[g_file_idx];
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
  g_irsend.begin();
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
    if (M5.BtnB.wasPressed()) {
      g_file_idx = (g_file_idx + 1) % IR_FILE_COUNT;
      drawUI();
    } else if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
      g_file_idx = (g_file_idx + IR_FILE_COUNT - 1) % IR_FILE_COUNT;
      drawUI();
    } else if (select) {
      g_mode = ScreenMode::CMD_LIST;
      g_cmd_idx = 0;
      g_cmd_scroll = 0;
      g_status = "Select command";
      drawUI();
      delay(120);
    }
  } else {
    const auto& file = IR_FILES[g_file_idx];
    if (M5.BtnB.wasPressed() && file.command_count > 0) {
      g_cmd_idx = (g_cmd_idx + 1) % file.command_count;
      drawUI();
    } else if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed() && file.command_count > 0) {
      g_cmd_idx = (g_cmd_idx + file.command_count - 1) % file.command_count;
      drawUI();
    } else if (select) {
      if (file.command_count > 0) {
        sendCommand(file.commands[g_cmd_idx]);
      }
      drawUI();
      delay(120);
    }

    if (M5.BtnPWR.pressedFor(700)) {
      g_mode = ScreenMode::FILE_LIST;
      g_status = "Back to files";
      drawUI();
      delay(180);
    }
  }

  delay(10);
}

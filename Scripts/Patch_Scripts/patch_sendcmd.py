import re

with open("idk-ir-ws-1.47/src/main.cpp", "r") as f:
    code = f.read()

new_send_command = """void sendCommand(const IrCommand& cmd) {
  const uint8_t tx_pin = (g_tx_mode == TxMode::TX_EXTERNAL) ? IR_PIN_EXT : IR_PIN;
  IRsend sender(tx_pin);
  pinMode(tx_pin, OUTPUT);
  digitalWrite(tx_pin, LOW);
  sender.begin();

  if (cmd.is_raw) {
    if (cmd.raw_data.empty()) {
      g_status = "No raw data";
      return;
    }
    const uint32_t freq = cmd.frequency > 0 ? cmd.frequency : 38000;
    sender.sendRaw(cmd.raw_data.data(), cmd.raw_data.size(), freq);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) sender.sendRaw(cmd.raw_data.data(), cmd.raw_data.size(), freq);
    digitalWrite(tx_pin, LOW);
    g_status = "Sent RAW " + String(cmd.name);
    return;
  }

  String protocol = cmd.protocol;
  protocol.toUpperCase();

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

  if (protocol.indexOf("SAMSUNG") >= 0) {
    uint16_t addr8 = static_cast<uint8_t>(cmd.address & 0xFFu);
    uint16_t cmd8 = static_cast<uint8_t>(cmd.command & 0xFFu);
    uint64_t data = sender.encodeSAMSUNG(addr8, cmd8);
    sender.sendSAMSUNG(data, 32);
    for (uint8_t i = 0; i < IR_REPEATS; ++i) sender.sendSAMSUNG(data, 32);
    digitalWrite(tx_pin, LOW);
    g_status = "Sent Samsung " + String(cmd.name);
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
"""

start_str = "void sendCommand(const IrCommand& cmd) {"
end_str = "void drawUI() {"

start_idx = code.find(start_str)
end_idx = code.find(end_str)

if start_idx != -1 and end_idx != -1:
    new_code = code[:start_idx] + new_send_command + "\n" + code[end_idx:]
    with open("idk-ir-ws-1.47/src/main.cpp", "w") as f:
        f.write(new_code)
    print("Patched sendCommand")
else:
    print("Could not find sendCommand bounds")

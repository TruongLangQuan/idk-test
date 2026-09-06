import re

with open("idk-ir-ws-1.47/src/main.cpp", "r") as f:
    code = f.read()

# Update sendCommand
code = code.replace("const uint8_t tx_pin = (g_tx_mode == TxMode::TX_EXTERNAL) ? IR_PIN_EXT : IR_PIN;", "const uint8_t tx_pin = IR_PIN;")

# Update drawUI
old_ui = 'M5Display.printf("TX:%s pin:%d %s\\n", g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT",\n                    g_tx_mode == TxMode::TX_EXTERNAL ? IR_PIN_EXT : IR_PIN, g_invert_logic ? "INV" : "NRM");'
new_ui = 'M5Display.printf("TX: GPIO %d Logic:%s\\n", IR_PIN, g_invert_logic ? "INV" : "NRM");'
code = code.replace(old_ui, new_ui)

old_cursor0 = """      if (g_dir_cursor == 0) {
        if (g_tx_mode == TxMode::TX_INTERNAL) {
          g_tx_mode = TxMode::TX_EXTERNAL;
          g_invert_logic = false;
        } else if (g_tx_mode == TxMode::TX_EXTERNAL && !g_invert_logic) {
          g_invert_logic = true;
        } else {
          g_tx_mode = TxMode::TX_INTERNAL;
          g_invert_logic = false;
        }
        g_status = "TX mode changed";
        drawUI();
      } else {"""
new_cursor0 = """      if (g_dir_cursor == 0) {
        g_invert_logic = !g_invert_logic;
        g_status = g_invert_logic ? "Logic: Inverted" : "Logic: Normal";
        drawUI();
      } else {"""
code = code.replace(old_cursor0, new_cursor0)

old_ui2 = """      if (i == 0) {
        M5Display.printf("%c [TX:%s]\\n", selected ? '>' : ' ',
                          g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT");
      } else {"""
new_ui2 = """      if (i == 0) {
        M5Display.printf("%c [Toggle Logic]\\n", selected ? '>' : ' ');
      } else {"""
code = code.replace(old_ui2, new_ui2)

with open("idk-ir-ws-1.47/src/main.cpp", "w") as f:
    f.write(code)

print("Patched to always use GPIO 9")

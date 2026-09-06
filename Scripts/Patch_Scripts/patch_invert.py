import re

with open("idk-ir-ws-1.47/src/main.cpp", "r") as f:
    code = f.read()

# Add a global for inverted logic
global_invert = "static bool g_invert_logic = false;\n\nstruct IrCommand {"
code = code.replace("struct IrCommand {", global_invert)

# Update sendCommand to use inverted logic
old_send = "IRsend sender(tx_pin);"
new_send = "IRsend sender(tx_pin, g_invert_logic, true);"
code = code.replace(old_send, new_send)

# Update UI to show inverted logic state
old_ui = 'M5Display.printf("TX:%s pin:%d\\n", g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT",\n                    g_tx_mode == TxMode::TX_EXTERNAL ? IR_PIN_EXT : IR_PIN);'
new_ui = 'M5Display.printf("TX:%s pin:%d %s\\n", g_tx_mode == TxMode::TX_EXTERNAL ? "EXT" : "INT",\n                    g_tx_mode == TxMode::TX_EXTERNAL ? IR_PIN_EXT : IR_PIN, g_invert_logic ? "INV" : "NRM");'
code = code.replace(old_ui, new_ui)

# Update loop to toggle inverted logic on press when cursor is on the TX mode toggle (dir cursor == 0)
old_cursor0 = """      if (g_dir_cursor == 0) {
        g_tx_mode = (g_tx_mode == TxMode::TX_INTERNAL) ? TxMode::TX_EXTERNAL : TxMode::TX_INTERNAL;
        g_status = (g_tx_mode == TxMode::TX_EXTERNAL) ? "TX -> EXT pin 9" : "TX -> INT pin 9";
        drawUI();
      } else {"""

new_cursor0 = """      if (g_dir_cursor == 0) {
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
code = code.replace(old_cursor0, new_cursor0)

with open("idk-ir-ws-1.47/src/main.cpp", "w") as f:
    f.write(code)

print("Patched inverted logic UI")

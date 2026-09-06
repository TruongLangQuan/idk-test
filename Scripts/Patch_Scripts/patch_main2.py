import re

with open("idk-ir-ws-1.47/src/main.cpp", "r") as f:
    code = f.read()

# Fix setup()
setup_replacement = """
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  M5Display.init();
  M5Display.setRotation(1);
"""
code = re.sub(r"auto cfg = M5\.config\(\);\s*M5\.begin\(cfg\);\s*#if defined\(STICKS3\)\s*M5Display\.setRotation\(1\);\s*#else\s*M5Display\.setRotation\(3\);\s*#endif", setup_replacement, code, flags=re.MULTILINE)

# Fix inputSearchQuery
input_search_replacement = """
    bool btn_press = (digitalRead(5) == LOW);
    bool btn_right = (digitalRead(4) == LOW) || (digitalRead(2) == LOW);
    bool btn_left = (digitalRead(3) == LOW) || (digitalRead(0) == LOW);
    
    bool select = btn_press && !btn_press_last;
    bool right_pressed = btn_right && !btn_right_last;
    bool left_pressed = btn_left && !btn_left_last;
    
    btn_press_last = btn_press;
    btn_right_last = btn_right;
    btn_left_last = btn_left;
    
    if (select) {
"""

# Wait, `inputSearchQuery` is a modal. Let's see how it looks.

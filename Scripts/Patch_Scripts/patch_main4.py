import re

with open("idk-ir-ws-1.47/src/main.cpp", "r") as f:
    code = f.read()

def replacer(match):
    return """void setup() {
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  M5Display.init();
  M5Display.setRotation(1);
  M5Display.setBrightness(180);
"""

code = re.sub(r"void setup\(\)\s*\{\s*auto cfg = M5\.config\(\);\s*M5\.begin\(cfg\);\s*#if defined\(STICKS3\)\s*M5Display\.setRotation\(1\);\s*#else\s*M5Display\.setRotation\(3\);\s*#endif", replacer, code)

with open("idk-ir-ws-1.47/src/main.cpp", "w") as f:
    f.write(code)


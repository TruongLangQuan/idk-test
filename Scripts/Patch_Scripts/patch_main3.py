import re

with open("idk-ir-ws-1.47/src/main.cpp", "r") as f:
    code = f.read()

# Replace M5.BtnA.wasPressed() inside inputSearchQuery
code = code.replace("if (M5.BtnA.wasPressed()) {", "if (select) {")

with open("idk-ir-ws-1.47/src/main.cpp", "w") as f:
    f.write(code)


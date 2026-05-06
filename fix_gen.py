import re

with open("idk-ascii-idk/src/animations/FinalBatchGen.h", "r") as f:
    header = f.read()

# Fix 3Body class name
header = header.replace("class 3Body", "class ThreeBody")

# Add name() method
header = re.sub(r'public:\n    void init', r'public:\n    const char* name() const override { return "GenAnim"; }\n    void init', header)

# Fix drawChar to setCell
header = header.replace("r.drawChar(", "r.setCell(")

with open("idk-ascii-idk/src/animations/FinalBatchGen.h", "w") as f:
    f.write(header)

with open("idk-ascii-idk/src/main.cpp", "r") as f:
    main_code = f.read()

main_code = main_code.replace("new 3Body()", "new ThreeBody()")

with open("idk-ascii-idk/src/main.cpp", "w") as f:
    f.write(main_code)

print("Fixed header and main.cpp")

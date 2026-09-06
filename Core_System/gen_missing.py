import re

with open("idk-ascii-idk/src/menu/MenuSystem.cpp") as f:
    menu_code = f.read()

categories = []
for line in menu_code.splitlines():
    if "{" in line and "}" in line and "ASCII 2D" not in line and "Settings" not in line:
        match = re.search(r'\{"([^"]+)",\s*\{(.*?)\}\}', line)
        if match:
            cat = match.group(1)
            anims = [a.strip().strip('"') for a in match.group(2).split(",")]
            categories.append((cat, anims))
    elif "ASCII 2D" in line and "Matrix Rain" in line:
        match = re.search(r'\{"([^"]+)",\s*\{(.*?)\}\}', line)
        if match:
            cat = match.group(1)
            anims = [a.strip().strip('"') for a in match.group(2).split(",")]
            categories.insert(0, (cat, anims))

with open("idk-ascii-idk/src/main.cpp") as f:
    main_code = f.read()

implemented = set(re.findall(r'new\s+([A-Za-z0-9_]+)', main_code))
implemented.discard('PlaceholderAnim')

# Create a mapping for what we have based on name similarity or existing main.cpp
# Actually we can just parse the switch statement in main.cpp
cat_anims_implemented = {}
current_cat = -1
for line in main_code.splitlines():
    if "case " in line and ":" in line:
        try:
            current_cat = int(re.search(r'case\s+(\d+):', line).group(1))
            cat_anims_implemented[current_cat] = {}
        except:
            pass
    if current_cat != -1 and "activeAnim = new" in line:
        match = re.search(r'if\s*\(anim\s*==\s*(\d+)\)\s*activeAnim\s*=\s*new\s+([A-Za-z0-9_]+)', line)
        if match:
            cat_anims_implemented[current_cat][int(match.group(1))] = match.group(2)

missing = []
for c_idx, (cat_name, anims) in enumerate(categories):
    for a_idx, a_name in enumerate(anims):
        if c_idx not in cat_anims_implemented or a_idx not in cat_anims_implemented[c_idx]:
            class_name = re.sub(r'[^a-zA-Z0-9]', '', a_name)
            missing.append((c_idx, a_idx, class_name, a_name))

print("Missing animations:", len(missing))
for m in missing:
    print(m)

# Generate FinalBatch.h
header = """#pragma once
#include "../engine/IAnimation.h"
#include <cmath>

namespace idk {
"""

for c_idx, a_idx, cls, name in missing:
    header += f"""
class {cls} : public IAnimation {{
private:
    float t = 0;
public:
    void init(ASCIIRenderer& r) override {{ t = 0; }}
    void update(ASCIIRenderer& r, float dt) override {{
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 2, "{name}", TFT_YELLOW);
        for(int i=0; i<5; i++) {{
            int x = 20 + std::sin(t * (i+1)) * 10;
            int y = 8 + std::cos(t * (i+1)) * 5;
            r.drawChar(x, y, '*', TFT_GREEN);
        }}
    }}
}};
"""

header += "} // namespace idk\n"

with open("idk-ascii-idk/src/animations/FinalBatchGen.h", "w") as f:
    f.write(header)

# Modify main.cpp
new_main = main_code.replace('#include "animations/PlaceholderAnim.h"', '#include "animations/PlaceholderAnim.h"\n#include "animations/FinalBatchGen.h"')

for c_idx, a_idx, cls, name in missing:
    search_str = f"case {c_idx}:"
    insert_str = f"            else if (anim == {a_idx}) activeAnim = new {cls}();\n"
    
    # We need to insert it inside the correct case before the 'else activeAnim = new PlaceholderAnim();'
    case_start = new_main.find(f"case {c_idx}:")
    if case_start != -1:
        placeholder_idx = new_main.find("else activeAnim = new PlaceholderAnim();", case_start)
        if placeholder_idx != -1:
            # Check if this case already has 'if' statements. If not, the first one should be 'if' not 'else if'
            has_if = new_main[case_start:placeholder_idx].find("if (anim ==") != -1
            if not has_if:
                insert_str = f"            if (anim == {a_idx}) activeAnim = new {cls}();\n            "
            new_main = new_main[:placeholder_idx] + insert_str + new_main[placeholder_idx:]

with open("idk-ascii-idk/src/main.cpp", "w") as f:
    f.write(new_main)

print("Generated and updated main.cpp!")

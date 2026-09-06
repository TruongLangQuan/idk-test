path = "../shared/idk_vi_font.h"
with open(path, "r") as f:
    content = f.read()

content = content.replace("#include <M5GFX.h>", """#if __has_include(<M5GFX.h>)
#include <M5GFX.h>
#else
#include <LovyanGFX.hpp>
#endif""")

with open(path, "w") as f:
    f.write(content)

import os
import re

ROOT_DIR = "/home/truonglangquan/idk-code/idk-test"

# 5-way Switch replacement template generator
def get_5way_replacement(indent, is_static, is_constexpr):
    pref = ("static " if is_static else "") + ("constexpr int " if is_constexpr else "int ")
    return f"""#if defined(STICKS3)
{indent}{pref}kPinUp     = 1;
{indent}{pref}kPinDown   = 2;
{indent}{pref}kPinLeft   = 3;
{indent}{pref}kPinRight  = 8;
{indent}{pref}kPinCenter = 43;
#elif defined(PCBFUN)
{indent}{pref}kPinUp     = 1;
{indent}{pref}kPinDown   = 2;
{indent}{pref}kPinLeft   = 3;
{indent}{pref}kPinRight  = 4;
{indent}{pref}kPinCenter = 5;
#else
{indent}{pref}kPinUp     = 32;
{indent}{pref}kPinDown   = 33;
{indent}{pref}kPinLeft   = 25;
{indent}{pref}kPinRight  = 26;
{indent}{pref}kPinCenter = 0;
#endif"""

# SD Card Pins replacement template generator
def get_sd_replacement(indent, is_static, is_constexpr):
    pref = ("static " if is_static else "") + ("const int " if not is_constexpr else "constexpr int ")
    return f"""#if defined(STICKS3)
{indent}{pref}kSdCsPin = 7;
{indent}{pref}kSdSckPin = 5;
{indent}{pref}kSdMisoPin = 4;
{indent}{pref}kSdMosiPin = 6;
#elif defined(PCBFUN)
{indent}{pref}kSdCsPin = -1;
{indent}{pref}kSdSckPin = -1;
{indent}{pref}kSdMisoPin = -1;
{indent}{pref}kSdMosiPin = -1;
#else
{indent}{pref}kSdCsPin = 14;
{indent}{pref}kSdSckPin = 0;
{indent}{pref}kSdMisoPin = 36;
{indent}{pref}kSdMosiPin = 26;
#endif"""

# Regex patterns
pattern_5way = re.compile(
    r'^([ \t]*)(static\s+)?constexpr\s+int\s+kPinUp\s*=\s*32\s*;[^\n]*\n'
    r'\s*(static\s+)?constexpr\s+int\s+kPinDown\s*=\s*33\s*;[^\n]*\n'
    r'\s*(static\s+)?constexpr\s+int\s+kPinLeft\s*=\s*25\s*;[^\n]*\n'
    r'\s*(static\s+)?constexpr\s+int\s+kPinRight\s*=\s*26\s*;[^\n]*\n'
    r'\s*(static\s+)?constexpr\s+int\s+kPinCenter\s*=\s*0\s*;[^\n]*',
    re.MULTILINE
)

pattern_sd = re.compile(
    r'^([ \t]*)static\s+const\s+int\s+kSdCsPin\s*=\s*14\s*;[^\n]*\n'
    r'\s*static\s+const\s+int\s+kSdSckPin\s*=\s*0\s*;[^\n]*\n'
    r'\s*static\s+const\s+int\s+kSdMisoPin\s*=\s*36\s*;[^\n]*\n'
    r'\s*static\s+const\s+int\s+kSdMosiPin\s*=\s*26\s*;[^\n]*',
    re.MULTILINE
)

# Rotation replacement patterns
pattern_rot = re.compile(r'^([ \t]*)(M5\.(Display|Lcd)|d)\.setRotation\(3\);', re.MULTILINE)

def process_file_generic(filepath):
    rel_path = os.path.relpath(filepath, ROOT_DIR)
    
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    
    original = content
    modified = False

    # Replace setRotation(3) globally
    match_rot = pattern_rot.search(content)
    if match_rot:
        indent = match_rot.group(1)
        device = match_rot.group(2)
        replacement = f"""#if defined(STICKS3)
{indent}{device}.setRotation(1);
#else
{indent}{device}.setRotation(3);
#endif"""
        content = pattern_rot.sub(replacement, content)
        print(f"  -> Inverted screen rotation in {rel_path}")
        modified = True

    if modified:
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(content)

def process_file_pins(filepath):
    rel_path = os.path.relpath(filepath, ROOT_DIR)
    print(f"Processing pins/specific fixes for {rel_path}...")
    
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    
    original = content
    modified = False

    # 1. Try to replace 5-way switch pins
    match_5way = pattern_5way.search(content)
    if match_5way:
        indent = match_5way.group(1)
        is_static = bool(match_5way.group(2) or match_5way.group(3))
        replacement = get_5way_replacement(indent, is_static, is_constexpr=True)
        content = pattern_5way.sub(replacement, content, count=1)
        print(f"  -> Replaced 5-way switch pins in {rel_path}")
        modified = True

    # 2. Try to replace SD card pins
    match_sd = pattern_sd.search(content)
    if match_sd:
        indent = match_sd.group(1)
        replacement = get_sd_replacement(indent, is_static=True, is_constexpr=False)
        content = pattern_sd.sub(replacement, content, count=1)
        print(f"  -> Replaced SD card pins in {rel_path}")
        modified = True

    # 3. Handle idk-audio and idk-video ensureSdReady()
    if rel_path in ["idk-audio/src/main.cpp", "idk-video/src/main.cpp"]:
        target_init = """static bool ensureSdReady() {
  if (g_sd_ready) return true;
  SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
  g_sd_ready = SD.begin(kSdCsPin, SPI);
  return g_sd_ready;
}"""
        replacement_init = """static bool ensureSdReady() {
  if (g_sd_ready) return true;
#if defined(PCBFUN)
  g_sd_ready = SD.begin();
#else
  SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
  g_sd_ready = SD.begin(kSdCsPin, SPI);
#endif
  return g_sd_ready;
}"""
        if target_init in content:
            content = content.replace(target_init, replacement_init)
            print(f"  -> Replaced ensureSdReady() in {rel_path}")
            modified = True

    # 4. Handle idk-ai/src/main.cpp specific changes (SD enabling and custom variables)
    if rel_path == "idk-ai/src/main.cpp":
        ai_sd_target = "constexpr int kSdCsPin = 14; "
        ai_sd_target_alt = "constexpr int kSdCsPin = 14;"
        ai_sd_replacement = get_sd_replacement("", is_static=False, is_constexpr=True)
        if ai_sd_target in content:
            content = content.replace(ai_sd_target, ai_sd_replacement)
            print(f"  -> Replaced kSdCsPin with full SD block in idk-ai/src/main.cpp")
            modified = True
        elif ai_sd_target_alt in content:
            content = content.replace(ai_sd_target_alt, ai_sd_replacement)
            print(f"  -> Replaced kSdCsPin with full SD block in idk-ai/src/main.cpp")
            modified = True
        
        # Enable SD card in setup()
        ai_init_target = """    // SPI and SD disabled due to pin conflict with 5-way switch (GPIO 0, 26)
    // SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
    // g_sd_ready = SD.begin(kSdCsPin, SPI);
    g_sd_ready = false;"""
        
        ai_init_replacement = """#if defined(STICKS3)
    SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
    g_sd_ready = SD.begin(kSdCsPin, SPI);
#elif defined(PCBFUN)
    g_sd_ready = SD.begin();
#else
    // SPI and SD disabled due to pin conflict with 5-way switch (GPIO 0, 26)
    // SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
    // g_sd_ready = SD.begin(kSdCsPin, SPI);
    g_sd_ready = false;
#endif"""
        if ai_init_target in content:
            content = content.replace(ai_init_target, ai_init_replacement)
            print(f"  -> Enabled SD Card in setup() for idk-ai/src/main.cpp")
            modified = True

    # 5. Handle idk-gamerom/src/main.cpp specific changes (Pins + PSRAM fallbacks)
    if rel_path == "idk-gamerom/src/main.cpp":
        target_5way_end = "#endif\n\nM5Canvas canvas(&M5.Display);"
        replacement_5way_end = """#endif

#if defined(STICKS3)
static const int kSdCsPin = 7;
static const int kSdSckPin = 5;
static const int kSdMisoPin = 4;
static const int kSdMosiPin = 6;
#elif defined(PCBFUN)
static const int kSdCsPin = -1;
static const int kSdSckPin = -1;
static const int kSdMisoPin = -1;
#else
static const int kSdCsPin = 14;
static const int kSdSckPin = 0;
static const int kSdMisoPin = 36;
static const int kSdMosiPin = 26;
#endif

M5Canvas canvas(&M5.Display);"""
        if target_5way_end in content:
            content = content.replace(target_5way_end, replacement_5way_end)
            print(f"  -> Inserted SD card pins in idk-gamerom/src/main.cpp")
            modified = True
            
        target_init = """  SPI.begin(0, 36, 26, 14);
  if (!SD.begin(14, SPI, 15000000)) {"""
        replacement_init = """#if defined(PCBFUN)
  if (!SD.begin()) {
#else
  SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
  if (!SD.begin(kSdCsPin, SPI, 15000000)) {
#endif"""
        if target_init in content:
            content = content.replace(target_init, replacement_init)
            print(f"  -> Configured dynamic SPI/SD init in idk-gamerom/src/main.cpp")
            modified = True

        # PSRAM fallbacks for GB emulation
        target_alloc = """  g_rom_data = (uint8_t*)heap_caps_malloc(sz, MALLOC_CAP_SPIRAM);
  g_ram_data = (uint8_t*)heap_caps_malloc(32768, MALLOC_CAP_SPIRAM); // 32KB RAM max for basic carts"""
        
        replacement_alloc = """  uint32_t caps = MALLOC_CAP_8BIT;
  if (ESP.getPsramSize() > 0) caps |= MALLOC_CAP_SPIRAM;
  g_rom_data = (uint8_t*)heap_caps_malloc(sz, caps);
  g_ram_data = (uint8_t*)heap_caps_malloc(32768, caps);
  if (!g_rom_data) g_rom_data = (uint8_t*)heap_caps_malloc(sz, MALLOC_CAP_8BIT);
  if (!g_ram_data) g_ram_data = (uint8_t*)heap_caps_malloc(32768, MALLOC_CAP_8BIT);"""
        if target_alloc in content:
            content = content.replace(target_alloc, replacement_alloc)
            print(f"  -> Configured DRAM fallback for ROM loading in idk-gamerom/src/main.cpp")
            modified = True

    # 6. Handle idk-badapple/src/main.cpp specific changes
    if rel_path == "idk-badapple/src/main.cpp":
        target_cs = "static constexpr int kSD_CS = 14;"
        target_cs_alt = "static constexpr int kSD_CS = 14; "
        replacement_cs = """#if defined(STICKS3)
static constexpr int kSdCsPin = 7;
static constexpr int kSdSckPin = 5;
static const int kSdMisoPin = 4;
static const int kSdMosiPin = 6;
#elif defined(PCBFUN)
static constexpr int kSdCsPin = -1;
static constexpr int kSdSckPin = -1;
static const int kSdMisoPin = -1;
static const int kSdMosiPin = -1;
#else
static constexpr int kSdCsPin = 14;
static constexpr int kSdSckPin = 0;
static const int kSdMisoPin = 36;
static const int kSdMosiPin = 26;
#endif
static constexpr int kSD_CS = kSdCsPin;"""
        if target_cs in content:
            content = content.replace(target_cs, replacement_cs)
            print(f"  -> Replaced kSD_CS with dynamic SD block in idk-badapple/src/main.cpp")
            modified = True
        elif target_cs_alt in content:
            content = content.replace(target_cs_alt, replacement_cs)
            print(f"  -> Replaced kSD_CS with dynamic SD block in idk-badapple/src/main.cpp")
            modified = True
            
        target_init = """    // Initialize the global SPI bus exactly like idk-video
    SPI.begin(0, 36, 26, 14);

    bool videoLoaded = false;

    // Try SD Card First (CS is Pin 14)
    if (SD.begin(14, SPI, 15000000)) {"""
        replacement_init = """    // Initialize the global SPI bus and SD Card
#if defined(PCBFUN)
    bool sd_ok = SD.begin();
#else
    SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
    bool sd_ok = SD.begin(kSdCsPin, SPI, 15000000);
#endif
    bool videoLoaded = false;

    // Try SD Card First
    if (sd_ok) {"""
        if target_init in content:
            content = content.replace(target_init, replacement_init)
            print(f"  -> Configured dynamic SPI/SD init in idk-badapple/src/main.cpp")
            modified = True

    # 7. Handle shared/idk_ui/src/idk_ui.h specific changes
    if rel_path == "shared/idk_ui/src/idk_ui.h":
        target_h = """#include <M5Unified.h>
#include <WiFi.h>"""
        replacement_h = """#include <M5Unified.h>
#include <WiFi.h>
#include <SD.h>
#include <SD_MMC.h>"""
        if target_h in content:
            content = content.replace(target_h, replacement_h)
            print(f"  -> Added SD and SD_MMC includes in idk_ui.h")
            modified = True

    if modified:
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"  -> Saved specific changes to {rel_path}")

def main():
    print("=== Processing Source Pin Adaptations and Orientation Inversion ===")
    
    files_with_pins = [
        "idk-ai/src/main.cpp",
        "idk-ascii-doom/src/main.cpp",
        "idk-audio/src/main.cpp",
        "idk-chess/src/main.cpp",
        "idk-draw/src/main.cpp",
        "idk-gamerom/src/idk_nes_osd.cpp",
        "idk-gamerom/src/idk_snes.cpp",
        "idk-gamerom/src/main.cpp",
        "idk-geometry-dash/src/main.cpp",
        "idk-maze/src/main.cpp",
        "idk-motion-ascii/src/main.cpp",
        "idk-sanctuaryrpg/src/main.cpp",
        "idk-video/src/main.cpp",
        "idk-worldgen/src/main.cpp",
        "shared/idk_ui/src/idk_ui.cpp",
        "idk-badapple/src/main.cpp",
        "shared/idk_ui/src/idk_ui.h"
    ]
    
    # First apply dynamic pins to core files
    for rel_path in files_with_pins:
        filepath = os.path.join(ROOT_DIR, rel_path)
        if os.path.exists(filepath):
            process_file_pins(filepath)
            
    # Then apply screen rotation replacement globally to all C++ sources
    exclude = {"firmware-sticks3", "firmware-pcbfun", ".venv-pio313", ".git", ".pio"}
    for dirpath, dirnames, filenames in os.walk(ROOT_DIR):
        dirnames[:] = [d for d in dirnames if d not in exclude]
        for filename in filenames:
            if filename.endswith((".cpp", ".h")):
                process_file_generic(os.path.join(dirpath, filename))

    print("\nProcessing complete!")

if __name__ == "__main__":
    main()

# DIRECTORY_MAP

## Repository Structure: 127 PlatformIO projects across 56 directories

---

## FIRMWARE PROJECTS — M5StickC Plus2 (`m5stickc_plus2`)

### Entertainment & Games **[SAFE TO EDIT]**
| Directory | Purpose | Key Files |
|---|---|---|
| `idk-ascii-doom/` | DOOM-style ASCII FPS dungeon crawler | `src/main.cpp` |
| `idk-ascii-idk/` | 55+ ASCII animation engine (batch-generated) | `src/main.cpp`, `src/animations/`, `src/engine/` |
| `idk-ascii-screensaver/` | ASCII screensaver collection | `src/main.cpp` |
| `idk-screensaver/` | 27-mode screensaver suite | `src/main.cpp` |
| `idk-geometry-dash/` | Geometry Dash platformer game | `src/main.cpp` |
| `idk-sanctuaryrpg/` | Text RPG roguelike | `src/main.cpp` |
| `idk-motion-ascii/` | 3D ASCII animation with density modes | `src/main.cpp` |
| `idk-chess/` | Chess game with AI + offline mode | `src/main.cpp` |
| `idk-dice/` | Accelerometer dice shaker | `src/main.cpp` |
| `idk-maze/` | 2D/3D maze with tilt controls | `src/main.cpp` |
| `idk-draw/` | Drawing canvas with 5-way switch | `src/main.cpp` |
| `idk-block-builder-3d/` | 3D block builder game | `src/main.cpp` |
| `idk-bitnet/` | BitNet/cryptocurrency project | `src/main.cpp` |

### Educational — Math (17 sub-projects) **[SAFE TO EDIT]**
| Directory | Purpose |
|---|---|
| `idk-math/idk-math-basic/` | Basic arithmetic operations |
| `idk-math/idk-math-algebra/` | Algebra solver |
| `idk-math/idk-math-calc/` | Calculus tools |
| `idk-math/idk-math-complex/` | Complex number operations |
| `idk-math/idk-math-finance/` | Financial calculations |
| `idk-math/idk-math-geom/` | Geometry tools |
| `idk-math/idk-math-graph/` | Graph theory |
| `idk-math/idk-math-matrix/` | Matrix operations |
| `idk-math/idk-math-prob/` | Probability calculator |
| `idk-math/idk-math-stat/` | Statistics tools |
| `idk-math/idk-math-transform/` | Transform operations |
| `idk-math/idk-math-vector/` | Vector math |
| `idk-math/idk-notes/` | Notes/notebook app |
| `idk-math/idk-sci-formula/` | Scientific formula reference |
| `idk-math/idk-unit-convert/` | Unit converter |
| `idk-math/idk-exam-practice/` | Exam practice mode |
| `idk-math/idk-graph-2d3d/` | 2D/3D graphing |

### Educational — Physics (10 sub-projects) **[SAFE TO EDIT]**
| Directory | Purpose |
|---|---|
| `idk-physics/idk-physics-kinematics/` | Kinematics equations |
| `idk-physics/idk-physics-mechanics/` | Mechanics solver |
| `idk-physics/idk-physics-electric/` | Electric field calculations |
| `idk-physics/idk-physics-circuit/` | Circuit analysis |
| `idk-physics/idk-physics-optics/` | Optics calculations |
| `idk-physics/idk-physics-thermo/` | Thermodynamics |
| `idk-physics/idk-physics-waves/` | Wave mechanics |
| `idk-physics/idk-physics-modern/` | Modern physics |
| `idk-physics/idk-physics-astro/` | Astrophysics |
| `idk-physics/idk-exam-practice/` | Physics exam practice |

### Educational — Chemistry (11 sub-projects) **[SAFE TO EDIT]**
| Directory | Purpose |
|---|---|
| `idk-chemistry/idk-chemistry-periodic/` | Periodic table browser |
| `idk-chemistry/idk-chemistry-stoich/` | Stoichiometry solver |
| `idk-chemistry/idk-chemistry-reaction/` | Chemical reaction balancer |
| `idk-chemistry/idk-chemistry-organic/` | Organic chemistry tools |
| `idk-chemistry/idk-chemistry-redox/` | Redox reactions |
| `idk-chemistry/idk-chemistry-kinetics/` | Reaction kinetics |
| `idk-chemistry/idk-chemistry-solution/` | Solution chemistry |
| `idk-chemistry/idk-chemistry-thermo/` | Chemical thermodynamics |
| `idk-chemistry/idk-chemistry-analytic/` | Analytical chemistry |
| `idk-chemistry/idk-exam-practice/` | Chemistry exam practice |
| `idk-chemistry/idk-lab-tools/` | Lab calculation tools |

### Educational — Other **[SAFE TO EDIT]**
| Directory | Purpose |
|---|---|
| `idk-biology/` | Biology tools (transcription/translation/genetics) |
| `idk-english-wordform/` | English word form lookup |

### Utilities & Tools **[SAFE TO EDIT]**
| Directory | Purpose |
|---|---|
| `idk-miner/` | LTC/BTC cryptocurrency dashboard |
| `idk-translator/` | EN↔VI translator + dictionary |
| `idk-ai/` | LLM AI chat (OpenRouter API) |
| `idk-audio/` | WAV PCM audio player |
| `idk-worldgen/` | Procedural world generator |
| `idk-worldgen-2d/` | 2D procedural terrain |
| `idk-geogen-2d/` | 2D geology generator |
| `idk-geogen-multidim/` | Multi-dimensional geology gen |

---

## FIRMWARE PROJECTS — M5StickC Original (`firmware`)

**[EDIT WITH CAUTION]** — Different LCD/PMIC from Plus2

| Directory | Purpose |
|---|---|
| `idk-gif/` | Embedded GIF player |
| `idk-img/` | Embedded image viewer |
| `idk-txt/` | Embedded text viewer |
| `idk-video/` | MJPEG video player + subtitle UDP |
| `idk-ir/` | IR remote transmitter (NEC/Samsung/Sony/RAW) |
| `idk-clock/` | NTP clock (GMT+7) |
| `idk-atom/` | Orbital atom animation |
| `idk-blackhole/` | Black hole visual effect |
| `idk-linux/` | Fastfetch/CMatrix/Pipe terminal sim |
| `idk-portal/` | Captive portal AP + DNS |
| `idk-badapple/` | Bad Apple ASCII animation player |

---

## FIRMWARE PROJECTS — Tenstar ESP32-S3 (`esp32s3_114tft`)

**[EDIT WITH CAUTION]** — Different pin mapping, uses TFT_eSPI directly

| Directory | Purpose |
|---|---|
| `idk-ts-test/` | SD card browser for Tenstar |
| `idk-ts-ascii/` | ASCII art for Tenstar display |
| `idk-ts-badusb/` | BadUSB HID keyboard emulator (CH9329) |
| `idk-ts-desktop/` | Desktop companion (clock/weather/timetable/Pomodoro) |
| `idk-ts-doom/` | DOOM for Tenstar |
| `idk-ts-gameboy/` | GameBoy emulator for Tenstar |
| `idk-gif-s3/` | GIF player for ESP32-S3 |
| `idk-subtitle/` | UDP subtitle display receiver |
| `idk-bat/` | Battery monitor (serial output) |
| `idk-tenstar-remote/` | Remote subtitle (includes idk-subtitle) |
| `idk-s3-test/` | ESP32-S3 SD browser (16MB flash variant) |

---

## FIRMWARE PROJECTS — CYD & Other

**[DO NOT EDIT HARDWARE PINS]**

| Directory | Purpose |
|---|---|
| `CYD_Firmware/` | Full CYD dashboard (clock/GIF/image/weather/touch) |
| `idk-cyd/` | CYD UI data file (no firmware, `ui.txt` only) |

---

## FIRMWARE PROJECTS — Multi-Target

| Directory | Purpose | Targets |
|---|---|---|
| `idk-mine/` | Crypto mining/lottery suite | CYD, ESP32-WROOM-32U |
| `idk-gamerom/` | ROM browser + NES/SNES emulator hooks | M5StickC Plus2 |

---

## SPECIAL DIRECTORIES

### Third-Party Ports **[EDIT WITH CAUTION]**
| Directory | Purpose |
|---|---|
| `idk-corey/` | 34 ported games/apps from coreycm5 (Tetris variants, RPGs, screensavers, tools) |
| `idk-corey/_libs/` | Shared libraries for corey projects |

### Hardware Test & Diagnostic **[EDIT WITH CAUTION]**
| Directory | Purpose |
|---|---|
| `idk-5way-test/` | 5-way switch + joystick hardware tester |
| `idk-m5-test/` | M5StickC↔ESP32-S3 UART remote test |

---

## SHARED & UTILITIES

| Directory | Purpose | Edit Level |
|---|---|---|
| `shared/` | Common libraries (Vietnamese font, VN text, UI) | **[EDIT WITH CAUTION]** |
| `shared/idk_ui/` | Shared UI component library | **[SAFE TO EDIT]** |
| `shared/idk_vi_font.h` | Vietnamese font wrapper (M5GFX) | **[EDIT WITH CAUTION]** |
| `shared/idk_vn_text.h` | Vietnamese text utilities | **[EDIT WITH CAUTION]** |
| `tools/` | Asset compiler, firmware manager | **[EDIT WITH CAUTION]** |
| `tools/asset_compiler.py` | Compile img/gif/txt/ir → C++ headers | **[EDIT WITH CAUTION]** |
| `tools/firmware_manager.py` | Firmware management utility | **[EDIT WITH CAUTION]** |
| `docs/` | Documentation and reference code | **[SAFE TO EDIT]** |

---

## GENERATED / BUILD / OUTPUT

| Directory | Purpose | Edit Level |
|---|---|---|
| `idk-bin/` | Compiled merged `.bin` files for OTA | **[GENERATED — DO NOT EDIT]** |
| `idk-bin/ota_apps/` | OTA-ready binaries | **[GENERATED — DO NOT EDIT]** |
| `idk-bin/usb_flash/` | USB flash-ready binaries | **[GENERATED — DO NOT EDIT]** |
| `idk-bin/corey/` | Corey project binaries | **[GENERATED — DO NOT EDIT]** |
| `*/.pio/` | PlatformIO build cache (per project) | **[IGNORED — gitignored]** |
| `*/.vscode/` | VS Code workspace settings | **[IGNORED — gitignored]** |

---

## ROOT-LEVEL SCRIPTS

| File | Purpose | Edit Level |
|---|---|---|
| `build_merge_copy.fish` | Build + merge multiple firmwares for OTA | **[EDIT WITH CAUTION]** |
| `gen_distinct.py` | Generate 55 mathematically distinct ASCII animations | **[SAFE TO EDIT]** |
| `gen_missing.py` | Fill in missing animation implementations | **[SAFE TO EDIT]** |
| `fix_gen.py` | Fix generated animation code (3Body → ThreeBody, etc.) | **[SAFE TO EDIT]** |
| `convert_frames.py` | Convert Bad Apple frames.h → binary | **[SAFE TO EDIT]** |
| `process_video.py` | ffmpeg video → ASCII frame C++ header | **[SAFE TO EDIT]** |
| `User_Setup.h` | TFT_eSPI config for Tenstar ST7789 | **[DO NOT EDIT]** |
| `idk.txt` | Misc notes | **[SAFE TO EDIT]** |

---

## AGENT/AI CONFIG DIRECTORIES

| Directory | Purpose | Edit Level |
|---|---|---|
| `.agents/` | Agent skill definitions | **[DO NOT EDIT without understanding]** |
| `.agent/` | Agent configuration | **[DO NOT EDIT without understanding]** |
| `.codex/` | Codex AI config | **[DO NOT EDIT]** |
| `.gemini/` | Gemini AI config | **[DO NOT EDIT]** |

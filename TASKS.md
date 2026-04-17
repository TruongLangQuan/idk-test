# TASKS

## Add new sensor
- Files: project `src/main.cpp`, maybe `platformio.ini`, maybe `shared/`
- Risk: MEDIUM
- Notes: prefer `M5Unified`/existing libs before raw bus code

## Change display logic
- Files: target project `src/main.cpp`, `shared/idk_ui/*`, `shared/idk_vi_font.h`
- Risk: MEDIUM
- Notes: preserve rotation and footer/header collision rules

## Modify WiFi config
- Files: target project `src/main.cpp`, `Launcher/src/settings.cpp` if launcher-wide
- Risk: HIGH
- Notes: AP vs STA behavior is project-specific

## Debug boot issue
- Files: `Launcher/src/main.cpp`, `Launcher/src/settings.cpp`, `Launcher/src/sd_functions.cpp`
- Risk: VERY HIGH
- Notes: verify boot-to-app, config load, SD timing, and button escape path

## Add subtitle support to a player
- Files: `idk-video/src/main.cpp`, `idk-subtitle/src/main.cpp`, `shared/idk_vi_font.h`
- Risk: HIGH
- Notes: keep cue timing, UDP updates, and basename matching aligned

## Add a new mini-firmware
- Files: new `<project>/platformio.ini`, new `<project>/src/main.cpp`, root `README.md`
- Risk: LOW to MEDIUM
- Notes: keep it self-contained first

## Update board pin mapping
- Files: `Launcher/boards/pinouts/*`, maybe board `interface.cpp`
- Risk: VERY HIGH
- Notes: hardware validation required

## Regenerate dictionary/assets
- Files: generated outputs + source assets/tools
- Risk: LOW
- Notes: do not hand-edit generated files unless regeneration is impossible

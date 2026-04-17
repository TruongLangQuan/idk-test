# WORKFLOWS

## Build firmware
- `cd /home/truonglangquan/idk-code/idk-test/<project>`
- `pio run -e <env>`
- Common envs:
  - `firmware` for many M5StickC projects
  - `m5stickc_plus2` for M5StickC Plus2 projects
  - `esp32s3_114tft` for Tenstar/ST7789 subtitle-class projects

## Flash firmware
- `pio run -e <env> -t upload`
- Monitor with `pio device monitor -b 115200`

## Merge bin
- From project build dir:
- `esptool.py --chip esp32 merge_bin -o merged.bin 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin`
- Repo helper: `fish /home/truonglangquan/idk-code/idk-test/build_merge_copy.fish`

## Debug hardware
- Boot issues:
  - check serial monitor first
  - verify board/env matches target hardware
  - verify rotation/button expectations after boot
- SD issues:
  - verify SPI pins
  - retest mount after cold boot
  - compare with `idk-video`/`idk-audio` known-good wiring
- WiFi/AP issues:
  - verify STA/AP mode
  - verify UDP ports `4210-4212`
- Subtitle issues:
  - verify basename match `name.mjpg` + `name.srt`
  - verify local subtitle render before remote testing

## Add new module
- Copy a small single-purpose PlatformIO project as template
- Keep board/env explicit in `platformio.ini`
- Put all feature logic in project-local `src/main.cpp` first
- Reuse `shared/` helpers only for stable UI/text utilities
- If hardware pins are needed, define them near top of project and document target board
- Add a short `README.md` for controls and assets

## Regenerate assets/dictionaries
- Assets: `python3 tools/asset_compiler.py all`
- Wordform dictionary header: `python3 idk-english-wordform/tools/build_dictionary_header.py`

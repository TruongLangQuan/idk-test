# HARDWARE_MAP

## Core targets
- M5StickC / M5StickC Plus2
  - Files: most `idk-*` firmware under repo root and subject packs
  - APIs: `M5Unified`, `M5.Display`, `M5.Imu`, `M5.Rtc`
  - Protocols: display/IMU/RTC abstracted by `M5Unified`
  - Risk: HIGH when changing button flow, rotation, RTC, IMU, or SD access assumptions
- Tenstar ESP32-S3 1.14 TFT
  - Files: `idk-subtitle/src/main.cpp`, `idk-bat/src/*`
  - APIs: `TFT_eSPI`, `WiFi`, `WiFiUDP`
  - Pins: `TFT_I2C_POWER=21`, `TFT_BL` from board config, button on GPIO `0`
  - Protocols: TFT over SPI `ASSUMPTION`, WiFi STA
  - Risk: HIGH because real display pins live in TFT_eSPI/user setup or board config
- Launcher multi-board support
  - Files: `Launcher/boards/pinouts/*.h`, `Launcher/boards/*/interface.cpp`, `Launcher/src/*`
  - Protocols: SPI, SD, SD_MMC, WiFi, AsyncWebServer, HTTP update, FS
  - Risk: VERY HIGH

## Storage
- SD card on M5StickC-class video/audio firmware
  - Files: `idk-video/src/main.cpp`, `idk-audio/src/main.cpp`
  - Pins: CS `14`, SCK `0`, MISO `36`, MOSI `26`
  - Protocol: SPI
  - Critical dependency: file browser, MJPG/WAV playback
- Launcher removable storage
  - Files: `Launcher/src/sd_functions.cpp`, `Launcher/src/main.cpp`
  - Pins/protocol: board-specific via `Launcher/boards/pinouts/*`
  - Critical dependency: config, app bins, web UI assets

## Networking
- WiFi AP + UDP subtitle/control
  - Files: `idk-video/src/main.cpp`, `idk-subtitle/src/main.cpp`, `idk-maze/src/main.cpp`, `idk-dice/src/main.cpp`
  - Ports: subtitle `4210`, maze `4211`, dice `4212`
  - Critical dependency: subtitle sync, remote control
- WiFi STA / NTP / online tools
  - Files: `idk-clock/src/main.cpp`, `idk-english-wordform/src/main.cpp`, `Launcher/src/*`, `idk-miner/src/*`
  - Protocols: WiFi STA, HTTP, NTP
  - Risk: HIGH if blocking or credential logic changes

## Display / graphics
- M5 display path
  - Files: most `src/main.cpp`, `shared/idk_ui/*`, `shared/idk_vi_font.h`
  - Dependency: rotation, text metrics, subtitle overlay space
- Tenstar subtitle display path
  - Files: `idk-subtitle/src/main.cpp`, `shared/idk_vn_text.h`
  - Critical dependency: UTF-8/Vietnamese rendering limits

## RTC / timing
- External RTC via M5
  - Files: `idk-clock/src/main.cpp`
  - API: `M5.Rtc`
  - Critical dependency: offline clock fallback, timezone GMT+7
- Video timing
  - Files: `idk-video/src/main.cpp`
  - Dependency: `kFrameDelayMs`, subtitle cue timing, skip logic

## Input / motion
- Buttons
  - Files: all handheld firmwares, Launcher input task
  - Risk: MEDIUM to HIGH because UI flow is button-driven
- IMU / tilt
  - Files: `idk-maze/src/main.cpp`, `idk-dice/src/main.cpp`
  - Protocol: via `M5Unified`
  - Risk: HIGH for axis mapping and gameplay correctness

## Assumptions
- Exact SPI TFT pins for Tenstar are defined outside firmware source via board/TFT setup.
- M5 display/IMU wiring is abstracted by `M5Unified`; low-level pins are not duplicated in each project.

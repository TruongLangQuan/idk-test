# idk-test - Universal Firmware Repository

This repository contains various firmware projects for different ESP32-based devices. The codebase has been organized into specific device folders to manage hardware variations and dependencies cleanly.

## 🗂️ Directory Structure

- **`Core_System/`** - Shared libraries, build tools, and common core code.
- **`Docs/`** - Project documentation, tasks, workflows, and hardware maps.
- **`Scripts/`** - Python scripts for patching, testing, and processing video/assets.
- **`Outputs/`** - Compiled binaries, output videos, and logs.

### 📱 Device Families

### [Device_CYD](./Device_CYD)
- `CYD_Firmware`
- `idk-cyd`

### [Device_ESP32S3_Generic](./Device_ESP32S3_Generic)
- `idk-gif-s3`
- `idk-s3-coop-receiver`
- `idk-s3-ssh`
- `idk-s3-test`

### [Device_M5Stick](./Device_M5Stick)
- `.agent`
- `.agents`
- `.codex`
- `.gemini`
- `docs`
- `firmware-pcbfun`
- `firmware-sticks3`
- `idk-5way-test`
- `idk-ai`
- `idk-ascii-doom`
- `idk-ascii-idk`
- `idk-ascii-screensaver`
- `idk-atom`
- `idk-audio`
- `idk-badapple`
- `idk-bat`
- `idk-biology`
- `idk-bitnet`
- `idk-blackhole`
- `idk-block-builder-3d`
- `idk-browser`
- `idk-cardkb`
- `idk-chemistry`
- `idk-chess`
- `idk-clock`
- `idk-dice`
- `idk-draw`
- `idk-english-wordform`
- `idk-gamerom`
- `idk-geogen-2d`
- `idk-geogen-multidim`
- `idk-geometry-dash`
- `idk-gif`
- `idk-img`
- `idk-ir`
- `idk-linux`
- `idk-m5-test`
- `idk-math`
- `idk-maze`
- `idk-mine`
- `idk-miner`
- `idk-motion-ascii`
- `idk-physics`
- `idk-portal`
- `idk-sanctuaryrpg`
- `idk-screensaver`
- `idk-subtitle`
- `idk-translator`
- `idk-txt`
- `idk-video`
- `idk-worldgen`
- `idk-worldgen-2d`
- `miniusb-hid`

### [Device_Seeed_XIAO](./Device_Seeed_XIAO)
- `idk-s3-147-hid`
- `idk-s3-lcd-147-test`
- `idk-s3-sd-5way`
- `idk-seeed-s3-test`

### [Device_Tenstar](./Device_Tenstar)
- `idk-tenstar-remote`
- `idk-ts-ascii`
- `idk-ts-badusb`
- `idk-ts-desktop`
- `idk-ts-doom`
- `idk-ts-gameboy`
- `idk-ts-test`

### [Device_Waveshare_1.47](./Device_Waveshare_1.47)
- `Launcher-ws-1.47`
- `firmware-ws-s3`
- `idk-backrooms-ws-s3`
- `idk-badapple-ws-s3`
- `idk-blackhole-ws-s3`
- `idk-blender-ws-s3`
- `idk-casio-ws-s3`
- `idk-ir-ws-1.47`
- `idk-video-ws-1.47`
- `idk-ws-e32s3-1.47-5wts-k12`
- `idk-ws-s3-doom`

## 🛠️ Building & Flashing

Each firmware is an independent PlatformIO project. To build and flash:

```bash
cd Device_Name/Project_Name
pio run -e firmware -t upload
pio device monitor -b 115200
```

*Note: All `platformio.ini` files have been automatically configured to resolve dependencies from the `Core_System/shared` and `Core_System/tools` folders.*

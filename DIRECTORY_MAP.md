# DIRECTORY_MAP

## FIRMWARE PROJECTS (PlatformIO)
- `/idk-ascii-*`: ASCII art and rendering engines (`idk-ascii-idk`, `idk-ascii-doom`). **[SAFE TO EDIT]**
- `/idk-worldgen-*`: 2D/3D procedural generation tools. **[SAFE TO EDIT]**
- `/idk-math/`, `/idk-physics/`, `/idk-chemistry/`, `/idk-biology/`: Educational simulation modules. **[SAFE TO EDIT]**
- `/idk-corey/`: Ports and games specifically for M5StickC Plus 2. **[EDIT WITH CAUTION]**
- `/idk-mine/`: Crypto/Lottery firmware variants (ESP32/CYD). **[EDIT WITH CAUTION]**
- `/CYD_Firmware/`: Cheap Yellow Display base firmware. **[DO NOT EDIT HARDWARE PINS]**

## SHARED & UTILITIES
- `/shared/`: Common libraries or assets (if applicable). **[SAFE TO EDIT]**
- `/tools/`: Scripts for building or flashing. **[EDIT WITH CAUTION]**
- `/docs/`: Project documentation. **[SAFE TO EDIT]**
- `/data-pdf/`: Reference material and datasheets. **[DO NOT EDIT]**

## GENERATED / BUILD FILES
- `/idk-bin/`: Compiled `.bin` files ready for OTA (merged bootloader+partitions+firmware). **[GENERATED - DO NOT EDIT]**
- `.pio/`: PlatformIO build cache inside each project. **[IGNORED]**

## SCRIPTS
- `build_merge_copy.fish`: Script used to build and merge firmware for OTA launchers. **[EDIT WITH CAUTION]**

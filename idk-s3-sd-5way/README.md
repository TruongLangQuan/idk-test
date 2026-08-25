# idk-s3-sd-5way

Firmware program for **M5Stack StickS3** to test if the Micro SD Card module and 5-Way tactile switch can function together on the Hat2-Bus.

## Pinout Configuration

### 1. Micro SD Card Module (SPI)
Connected to the Hat2-Bus:

| SD Card Module | M5StickS3 Pin |
|---|---|
| **3V3** | 3V3 |
| **CS** | G7 |
| **MOSI** | G6 |
| **CLK** | G5 |
| **MISO** | G4 |
| **GND** | GND |

### 2. 5-Way Tactile Switch
Connected to the remaining Hat2-Bus GPIOs (pull-down to GND when pressed, using internal `INPUT_PULLUP`):

| Switch Direction | M5StickS3 Pin |
|---|---|
| **UP** | G1 |
| **DOWN** | G2 |
| **LEFT** | G3 |
| **RIGHT** | G8 |
| **CENTER** | G43 |
| **GND** | GND |

## How to Compile & Flash

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-s3-sd-5way
pio run -e sticks3
pio run -e sticks3 --target upload
```

## Features

- **SD Mount Status:** Displays card type, total size, and free space.
- **Switch Polling:** Displays live button states on the LCD screen.
- **Write-Read Verification:** Pressing the **CENTER** button writes a test text file to the SD card, reads it back, checks that the contents match, and outputs status verification.

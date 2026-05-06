# idk-pdf

PDF Reader for M5StickC Plus 2.

## How it works
This firmware displays pre-converted JPG images of PDF pages stored in the SPIFFS filesystem. It uses a large Sprite (stored in PSRAM) to allow smooth panning of pages that are larger than the 240x135 screen.

## Controls
- **5-Way Switch (Joystick)**:
  - **UP/DOWN/LEFT/RIGHT**: Pan the current page.
  - **CENTER**: Go to the Next Page.
- **M5 Buttons**:
  - **BtnA (Main button)**: Next Page.
  - **BtnB (Side button)**: Previous Page.

## Setup
1. Convert your images to the screen's native resolution:
   ```bash
   python3 tools/convert_images.py
   ```
   (This script processes images from `data-pdf/` to `idk-pdf/data/` at 135x240 resolution with sharpening).
2. Use PlatformIO to upload the firmware and the filesystem:
   ```bash
   pio run -t upload
   pio run -t uploadfs
   ```

## Hardware Requirements
- **M5StickC Plus 2**
- **5-Way Tactile Switch** connected to:
  - UP: G32
  - DOWN: G33
  - LEFT: G25
  - RIGHT: G26
  - CENTER: G0

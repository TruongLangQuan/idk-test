# idk-s3-lcd-147-test

Test firmware for the **Waveshare / PCBFUN ESP32-S3-LCD-1.47** board.

## Pinout Map

### 1. Built-in LCD Display (ST7789, 172x320)
Controlled via LovyanGFX:
- **MOSI:** GPIO 45
- **SCLK:** GPIO 40
- **CS:**   GPIO 42
- **DC:**   GPIO 41
- **RST:**  GPIO 39
- **BL:**   GPIO 48

### 2. Built-in MicroSD Slot (SDMMC 4-Bit Mode)
Controlled via standard `<SD_MMC.h>` library:
- **CLK:** GPIO 14
- **CMD:** GPIO 15
- **D0:**  GPIO 16
- **D1:**  GPIO 18
- **D2:**  GPIO 17
- **D3:**  GPIO 21

### 3. External Peripherals (Connected to Side Headers)
Connected with standard `INPUT_PULLUP` and pulling low to GND when pressed:

| Peripheral | Board Pin |
|---|---|
| **5-Way Switch - UP** | GP1 |
| **5-Way Switch - DOWN** | GP2 |
| **5-Way Switch - LEFT** | GP3 |
| **5-Way Switch - RIGHT** | GP4 |
| **5-Way Switch - CENTER** | GP5 |
| **Push Button - A** | GP6 |
| **Push Button - B** | GP7 |

---

## How to Compile & Flash

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-s3-lcd-147-test
pio run -e esp32s3
pio run -e esp32s3 --target upload
```

## Features

- **Double-Buffered Rendering:** Utilizes LovyanGFX custom Sprites in RAM to eliminate LCD screen flicker entirely.
- **MicroSD Card MMC Mounting:** Mounts built-in MicroSD card over 4-bit MMC lanes, displaying card volume and directory listing.
- **Button Visualizer:** Real-time visual feedback blocks highlighting active switch directions and push buttons.
- **Active Read/Write Test:** Pressing **CENTER** runs active read/write validations on the MicroSD.

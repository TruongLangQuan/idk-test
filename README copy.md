# `idk-ws-e32s3-1.47-5wts-k12`

> **Custom Multi-Feature Testing & Control Firmware for Waveshare ESP32-S3-LCD-1.47B**

---

## 📖 Overview

`idk-ws-e32s3-1.47-5wts-k12` is a high-performance, standalone PlatformIO firmware engineered specifically for the **Waveshare ESP32-S3-LCD-1.47B** development board. 

It integrates dual 5-way tactile switch D-Pads, M5Stack CardKB 1.1 QWERTY keyboard support via I2C, MicroSD card speed benchmarking (`SD_MMC`), a mini 3-pin slide power switch indicator, and 3.7V battery voltage monitoring.

---

## 🛠️ Complete Hardware Pinout & Wiring

### 1. Onboard 1.47" LCD Display (ST7789, 172×320 IPS)
| Signal | ESP32-S3 Pin | Description |
|---|---|---|
| **MOSI** | `GPIO 45` | SPI Master Out Slave In |
| **SCLK** | `GPIO 40` | SPI Clock |
| **LCD_CS** | `GPIO 42` | LCD Chip Select |
| **LCD_DC** | `GPIO 41` | Data / Command Control |
| **LCD_RST** | `GPIO 39` | Hardware Reset |
| **LCD_BL** | `GPIO 46` | Backlight PWM Power Control |
| **RGB_Control** | `GPIO 38` | Onboard WS2812 RGB Light Bead |

### 2. Dual 5-Way Tactile Switches (5WTS-K12 — 10 Pins Total)
| Direction | Switch 1 (K1) Pin | Switch 2 (K2) Pin | Input Mode |
|---|---|---|---|
| **UP** | `GPIO 0` | `GPIO 6` | `INPUT_PULLUP` |
| **DOWN** | `GPIO 2` | `GPIO 7` | `INPUT_PULLUP` |
| **LEFT** | `GPIO 3` | `GPIO 8` | `INPUT_PULLUP` |
| **RIGHT** | `GPIO 4` | `GPIO 11` | `INPUT_PULLUP` |
| **PRESS (OK)** | `GPIO 5` | `GPIO 43` (TX Pin) | `INPUT_PULLUP` |

### 3. M5Stack CardKB 1.1 QWERTY Keyboard (I2C)
| CardKB Wire | Waveshare Header Pin | Protocol / Address |
|---|---|---|
| **SDA** | `GPIO 9` | I2C Data (`0x5F`) |
| **SCL** | `GPIO 10` | I2C Clock (`0x5F`) |
| **VCC** | `3V3` or `5V` (VBUS) | Power Header |
| **GND** | `GND` | Ground Header |

### 4. Mini 3-Pin Slide Switch & 3.7V Battery
| Component | Header Pin | Description |
|---|---|---|
| **Slide Switch** | `GPIO 44` (RX Pin) | `INPUT_PULLUP` (ON/OFF detection) |
| **Battery (+)** | `VBAT` | 3.7V LiPo / Li-ion positive input |
| **Battery (-)** | `GND` | Common ground |

### 5. Onboard MicroSD Card Interface (`SD_MMC`)
| Bus Line | ESP32-S3 Pin | Protocol |
|---|---|---|
| **SD_SCK** | `GPIO 14` | Clock |
| **SD_CMD** | `GPIO 15` | Command |
| **SD_D0** | `GPIO 16` | Data Bit 0 |
| **SD_D1** | `GPIO 18` | Data Bit 1 |
| **SD_D2** | `GPIO 17` | Data Bit 2 |
| **SD_D3** | `GPIO 21` | Data Bit 3 |

---

## ✨ Features

* **Dual D-Pad Visualizer**: Live graphical status rendering for Switch 1 (Cyan) and Switch 2 (Amber) with instant glowing directional highlights.
* **MicroSD Benchmark Engine**:
  * Auto-detects MicroSD cards via 4-bit / 1-bit `SD_MMC` bus.
  * Measures read/write throughput in KB/s.
  * Displays total capacity and remaining free space in MB.
  * **Trigger Test**: Press **K1 PRESS** to re-run the benchmark anytime.
* **Full-Spectrum I2C Bus Auto-Scanner**:
  * Probes addresses `0x01` through `0x7F` continuously every 800ms.
  * Live status bar displays responding I2C address (e.g. `Dev at 0x5F`).
  * Real-time typed text input buffer display (`KB: <text>_`).
* **Power Management & Status Bar**:
  * Samples battery voltage and displays real-time voltage ($V$) and percentage ($0\%-100\%$).
  * Shows Mini Slide Switch position (`SW: ON` / `SW: OFF`).

---

## ⚡ Compilation & Setup

### Prerequisites
* [PlatformIO Core](https://platformio.org/) installed.

### Build Project
```bash
cd idk-ws-e32s3-1.47-5wts-k12
pio run -e waveshare_147
```

### Upload Firmware
```bash
pio run -e waveshare_147 --target upload
```

### Monitor Serial Output
```bash
pio device monitor --baud 115200
```

---

## 📂 File Structure

```
idk-ws-e32s3-1.47-5wts-k12/
├── platformio.ini         # PlatformIO Environment Config (env:waveshare_147)
├── README.md              # Project Documentation
└── src/
    └── main.cpp           # Complete Firmware Source Code
```

# HARDWARE_MAP

## CORE TARGET: M5StickC Plus 2
**MCU:** ESP32-PICO-V3-02 (240MHz, Wi-Fi, Bluetooth)
**Memory:** 8MB Flash, 2MB PSRAM

### Components & Pins (Managed via M5Unified)
- **Display:** ST7789V2 135x240 TFT (SPI)
- **IMU (6-axis):** QMI8658 (I2C)
- **PMIC (Power):** AXP2101 (I2C)
- **RTC:** BM8563 (I2C)
- **Microphone:** SPM1423 (I2S)
- **Buzzer:** Passive (PWM)
- **IR Transmitter:** IR diode
- **Buttons:**
  - Button A (Front): `G37`
  - Button B (Side): `G39`
  - Button C (Power): `G35`
- **LED:** Red LED (`G19`, inverted)

### External Interfaces (Grove Port)
- `G32` / `G33` (I2C / UART / GPIO) - Often used for 5-way joystick in some projects.

---

## ALTERNATE TARGET: CYD (Cheap Yellow Display)
**MCU:** ESP32-WROOM-32
- **Display:** ILI9341 / ILI9488 (2.8" or 2.4")
- **Touch:** XPT2046 / Resistive
- **SD Card:** SPI
- **Audio:** DAC / I2S amp

---

## ALTERNATE TARGET: Tenstar / ESP32-S3
- Custom pinouts defined in specific `idk-ts-*` or `idk-tenstar-*` projects.

---

## ⚠️ CRITICAL DEPENDENCIES
- `M5Unified` abstracts M5StickC hardware. **DO NOT** use raw `pinMode` or `digitalWrite` for I2C devices or display pins.
- **ASSUMPTION:** 5-way tactile switch mapping used in some apps (`idk-draw`, `idk-worldgen`) assumes:
  - UP=`32` (Grove), DOWN=`33` (Grove)
  - LEFT=`25` (Header), RIGHT=`26` (Header), CENTER=`0` (Header)

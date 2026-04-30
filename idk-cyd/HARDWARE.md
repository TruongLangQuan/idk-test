# Hardware Setup Guide - ESP32-2432S028R CYD

## Device Overview

**Cheap Yellow Display (CYD)** is a popular ESP32 breakout board featuring:
- **MCU**: ESP32 (Dual-core, 240MHz)
- **Display**: 2.4" TFT LCD (320×240 pixels)
- **Driver**: ILI9341
- **Touch**: XPT2046 capacitive touchscreen
- **Power**: Micro USB 5V, 500mA
- **Buttons**: 2 (GPIO0, GPIO35)
- **SD Card**: MicroSD slot

## Pin Configuration

### Display (ILI9341 SPI)

| Function | GPIO | Description |
|----------|------|-------------|
| TFT_CS | GPIO15 | Chip Select (active low) |
| TFT_DC | GPIO32 | Data/Command (0=command, 1=data) |
| TFT_MOSI | GPIO13 | Master Out Slave In (SPI data) |
| TFT_MISO | GPIO12 | Master In Slave Out (SPI data) |
| TFT_SCLK | GPIO14 | Serial Clock (SPI) |
| TFT_RST | GPIO27 | Reset (active low) |

### Touchscreen (XPT2046 SPI)

| Function | GPIO | Description |
|----------|------|-------------|
| TOUCH_CS | GPIO4 | Chip Select |
| TOUCH_IRQ | GPIO36 | Interrupt (active low) |
| TOUCH_DIN | GPIO3 | Data In (RX) |
| TOUCH_DOUT | GPIO2 | Data Out (TX) |
| TOUCH_CLK | GPIO25 | Clock |

### Input Devices

| Function | GPIO | Active | Purpose |
|----------|------|--------|---------|
| BTN_DOWN | GPIO0 | LOW | User button (commonly BOOT) |
| BTN_UP | GPIO35 | LOW | User button |

### Power & Analog

| Function | GPIO | Type |
|----------|------|------|
| USB_PWR | 5V | Power Input |
| GND | GND | Ground |
| BAT_ADC | GPIO34 | ADC (12-bit) |

### SPI Bus Details

The device uses **two SPI buses**:

1. **Main SPI** (for display):
   - Clock: GPIO14
   - MOSI: GPIO13
   - MISO: GPIO12
   - Can operate at 80MHz or higher

2. **Secondary SPI** (for touch):
   - Uses different pins
   - Typically 1-2MHz clock

## Wiring Guide

### USB Connection
```
Computer USB → CYD Micro USB
- Data lines for programming/serial
- 5V power (through voltage regulator)
```

### Optional Expansions

#### SD Card Reader
Connect MicroSD card to:
- CS: GPIO9 (or programmable)
- CLK: GPIO18 (typical)
- MOSI: GPIO23 (typical)
- MISO: GPIO19 (typical)

#### Additional Buttons
Connect pushbuttons to GPIO pins:
- GPIO0: Already reserved (BOOT)
- GPIO35: Already reserved
- GPIO22, GPIO23: Available
- GPIO25: Available

#### Sensors via I2C
- SCL: GPIO21
- SDA: GPIO20

#### Analog Input
- GPIO34: Battery voltage (use voltage divider for safety)
- GPIO35: General purpose ADC
- GPIO36: General purpose ADC

## Power Considerations

### Power Budget
- ESP32 typical: 80-100mA
- ILI9341 (backlight): 50-100mA
- Touch module: 5-10mA
- **Total: 150-200mA typical**

### Power Supply
- Recommended: 5V/2A USB power adapter
- Micro USB provides built-in voltage regulation
- Maximum 500mA via USB host (computer USB port)

## Reset & Boot Modes

### Normal Boot
1. Power on or press RESET button
2. Boot from flash memory
3. Run loaded firmware

### Flash Mode (Manual)
1. Hold GPIO0 (BOOT) button
2. Press and release RESET button
3. Release GPIO0 button
4. Device ready for flashing

### Deep Sleep
- Configure GPIO to wake device
- Reduces power consumption to <1mA

## Serial Communication

### UART0 (Programming & Debug)
- Used by USB-to-UART chip
- Baud rate: 115200 (default)
- Auto-reset on USB connection

### Connection
```
Computer → USB Cable → CYD USB Port
```

Serial output appears on monitor after upload.

## Temperature & Environment

- **Operating temperature**: 0°C to 40°C
- **Storage temperature**: -40°C to 85°C
- **Humidity**: Non-condensing, <90%
- **Display**: Backlight rated for ~30,000 hours

## Common Issues & Solutions

### Display Not Lighting Up
- Check power supply (should see LED indicator)
- Verify TFT_RST connection
- Confirm display data pins connected properly
- Check TFT_eSPI user setup matches pinout

### No Serial Output
- Verify USB cable (some cables are power-only)
- Check USB driver installation
- Press RESET button
- Try different USB port

### Touchscreen Not Responding
- Verify XPT2046 pins connected
- Check I2C/SPI bus conflicts
- Calibrate touchscreen
- Confirm touch driver enabled in firmware

### Device Resets Continuously
- Check for short circuits
- Verify all GPIO connections
- Try lower SPI clock speed
- Update ESP32 core libraries

## Mechanical Specifications

- **Dimensions**: ~66mm × 47mm × 25mm
- **Weight**: ~50g
- **Connector**: Micro USB (Type-B)
- **Display**: Mounted on PCB with USB-C connector visible at top

## Stock Firmware Verification

To verify device is working before loading custom firmware:

```bash
# Run built-in test
# (Varies by vendor - check device documentation)

# Alternative: Upload Arduino "Blink" sketch
# - LED on GPIO2 should blink
# - Takes ~2 seconds per cycle
```

## External Resources

- **ESP32 Datasheet**: https://espressif.com/
- **ILI9341 Datasheet**: Ilitek official specs
- **XPT2046 Datasheet**: FocalTech documentation
- **TFT_eSPI Library**: https://github.com/Bodmer/TFT_eSPI

---

**Next Step**: See [QUICKSTART.md](./QUICKSTART.md) for building and flashing instructions.

# idk-ts-badusb

Tenstar TS-ESP32-S3 firmware for a CH9329 USB keyboard bridge with a built-in script.

## Wiring

- TS `3V3` -> CH9329 `VCC`
- TS `GND` -> CH9329 `GND`
- TS `GPIO1 / TX` -> CH9329 `RXD`
- TS `GPIO2 / RX` -> CH9329 `TXD`
- CH9329 USB -> target computer

## Built-in payload

Edit:

```text
include/payload.h
```

Keep the payload inside `R"PAYLOAD(... )PAYLOAD"`.

Supported commands:

```text
DELAY 1000
STRING hello
GUI r
WINDOWS r
ENTER
TAB
SPACE
ESC
```

## Run

The payload does not auto-run. After boot, hold the board `BOOT` button (`GPIO0`) for about 1 second to arm and run.

## Build

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-ts-badusb
pio run -e esp32s3_114tft
```

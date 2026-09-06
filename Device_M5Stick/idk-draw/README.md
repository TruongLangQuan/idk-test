# idk-draw

Pixel drawing firmware for M5StickC Plus2 with external 5-way tactile switch.

## Current behavior

- 16 mau chon nhanh bang `BtnA` / `BtnB`
- pixel nho hon ban cu, de ve chi tiet hon
- bo dong huong dan o duoi, chi giu thanh trang thai gon

## 5-way wiring

- `UP` -> `GPIO32`
- `DOWN` -> `GPIO33`
- `LEFT` -> `GPIO25`
- `RIGHT` -> `GPIO26`
- `CENTER` -> `GPIO0`
- `VCC` -> `3V3`
- `GND` -> `GND`

## Controls

- 5-way `UP/DOWN/LEFT/RIGHT`: move cursor
- 5-way `CENTER`: paint/erase current pixel
- `BtnA`: next color
- `BtnB`: previous color
- `PWR`: toggle draw/erase mode
- hold `PWR`: clear canvas

## Build

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-draw
pio run -e m5stickc_plus2
```

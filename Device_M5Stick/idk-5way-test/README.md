# idk-5way-test

Firmware test nhan nut cho module 5-way tactile switch tren M5StickC Plus2.

## Assumption

- Module dung kieu active-low
- Mỗi chân `UP/DOWN/LEFT/RIGHT/CENTER` nối vào 1 GPIO riêng
- `VCC` -> `3V3`
- `GND` -> `GND`

## Build

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-5way-test
pio run -e m5stickc_plus2
```

## Flash

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-5way-test
pio run -e m5stickc_plus2 -t upload
pio device monitor -b 115200
```

## Hien thi

- M5StickC Plus2 se hien trang thai tung nut tren man hinh
- Serial van in `PRESSED/RELEASED` de debug nhanh

## Pin map hien tai

Mac dinh da doi sang cac chan de dung duoc tren M5StickC Plus2 theo pinout ban gui:

- `UP` -> GPIO `32`
- `DOWN` -> GPIO `33`
- `LEFT` -> GPIO `25`
- `RIGHT` -> GPIO `26`
- `CENTER` -> GPIO `0`

Nguon/goi y dau day:

- `VCC` cua module -> `3V3`
- `GND` cua module -> `GND`
- `UP/DOWN` co the lay qua cong `HY2.0-4` (`G32`, `G33`)
- `LEFT/RIGHT/CENTER` lay o cum GPIO ho dau tren (`G25`, `G26`, `G0`)

Nếu dây của bạn khác, sửa các hằng số ở đầu file `src/main.cpp`.

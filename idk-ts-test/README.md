# idk-ts-test

Tenstar TS-ESP32-S3 test/browser firmware for:

- microSD SPI module
- 5-way tactile switch

## Wiring

### microSD module

- `3V3` -> Tenstar `3.3V`
- `GND` -> Tenstar `GND`
- `CS` -> `GPIO13`
- `MOSI` -> `GPIO11`
- `CLK` -> `GPIO12`
- `MISO` -> `GPIO10`

### 5-way tactile switch

- `VCC` -> Tenstar `3.3V`
- `GND` -> Tenstar `GND`
- `RIGHT` -> `GPIO18`
- `DOWN` -> `GPIO17`
- `LEFT` -> `GPIO16`
- `CENTER` -> `GPIO15`
- `UP` -> `GPIO14`

## Điều khiển

- The 5-way board is a passive active-low switch board.
- microSD detection is real via `SD.begin(...)`.

### Ở browser thư mục

- `UP`: lên
- `DOWN`: xuống
- `LEFT`: quay ra thư mục cha
- `RIGHT`: mở mục đang chọn
- `CENTER`: mở mục đang chọn

### Ở file viewer

- `UP`: trang trước
- `DOWN`: trang sau
- `RIGHT`: trang sau
- `LEFT`: back về browser
- `CENTER`: back về browser

### Khi SD chưa mount được

- `CENTER` hoặc `RIGHT` sẽ thử `remount`

## Pin

ASSUMPTION từ `idk-bat` trong cùng repo:

- `Battery ADC -> GPIO3`
- công thức hiện tại giả định `Vref ~= 4.3V` và mạch chia áp pin `2:1`
- UI sẽ hiển thị cả `V` và `%`

Nếu board `Tenstar TS-ESP32-S3` của bạn dùng chân đo pin hoặc chia áp khác, cần sửa lại trong [src/main.cpp](/home/truonglangquan/idk-code/idk-test/idk-ts-test/src/main.cpp:50)

## Build

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-ts-test
pio run -e esp32s3_114tft
```

## Flash

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-ts-test
pio run -e esp32s3_114tft -t upload
pio device monitor -b 115200
```

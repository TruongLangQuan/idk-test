# idk-s3-test

Firmware test cho `ESP32-S3-N16R8` với các phần đang có trong `/home/truonglangquan/Downloads/module-es32-s3`.

## Chức năng

- `ESP32-S3` làm host cho `microSD` và `5-way tactile switch`
- hiển thị browser cục bộ trên màn hình SPI 1.47"
- gửi toàn bộ UI browser qua `UART` sang `M5StickC Plus2`
- nhận lệnh điều khiển ngược lại từ `M5StickC Plus2`

## Pin map mặc định

Các chân dưới đây là `ASSUMPTION` để tôi có thể tạo firmware ngay. Nếu dây thực tế của bạn khác, chỉ cần sửa ở đầu [src/main.cpp](/home/truonglangquan/idk-code/idk-test/idk-s3-test/src/main.cpp:1).

### Display

- `VDD -> 3V3`
- `GND -> GND`
- `SDA -> GPIO11`
- `SCL -> GPIO12`
- `CS -> GPIO10`
- `DC -> GPIO9`
- `RES -> GPIO14`
- `BL -> GPIO21`

### microSD

- `3V3 -> 3V3`
- `GND -> GND`
- `MOSI -> GPIO15`
- `CLK -> GPIO16`
- `MISO -> GPIO17`
- `CS -> GPIO18`

### 5-way tactile switch

- `VCC -> 3V3`
- `GND -> GND`
- `UP -> GPIO4`
- `DOWN -> GPIO5`
- `LEFT -> GPIO6`
- `RIGHT -> GPIO7`
- `CENTER -> GPIO8`

### UART sang M5StickC Plus2

- `ESP32 TX -> GPIO47`
- `ESP32 RX -> GPIO48`
- `baud = 115200`

## Nối với M5StickC Plus2

- `ESP32-S3 GPIO47 TX -> M5StickC Plus2 GPIO32 RX`
- `ESP32-S3 GPIO48 RX <- M5StickC Plus2 GPIO33 TX`
- `GND ESP32-S3 <-> GND M5StickC Plus2`

## Điều khiển

### Trên 5-way nối vào ESP32-S3

- `UP`: lên / trang trước
- `DOWN`: xuống / trang sau
- `LEFT`: quay lại thư mục cha hoặc back khỏi viewer
- `RIGHT`: mở mục hoặc trang sau
- `CENTER`: mở mục hoặc back khỏi viewer

### Từ M5StickC Plus2 remote UI

- `BtnA`: open
- `BtnB`: down / next page
- `BtnPWR`: up / prev page
- giữ `BtnPWR`: back
- giữ `BtnA`: remount SD

## Build và flash

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-s3-test
pio run -e esp32s3_114tft
pio run -e esp32s3_114tft -t upload
pio device monitor -b 115200
```

## Test nhanh

- boot lên phải thấy browser trên màn hình `ESP32-S3`
- nhấn `5-way` phải di chuyển được trong browser
- cắm thẻ SD phải mount và liệt kê được `folder/file`
- nối `UART` với `M5StickC Plus2` phải thấy remote UI đồng bộ

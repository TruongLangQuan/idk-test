# idk-m5-test

Firmware này giờ là `remote UI client` cho `ESP32-S3`.

`M5StickC Plus2` chỉ làm:

- màn hình hiển thị browser/file viewer
- gửi lệnh điều khiển qua `UART`

`microSD` và `5-way tactile switch` được nối sang `ESP32-S3`, không còn cắm trực tiếp vào `M5StickC Plus2`.

## Wiring UART

```text
ESP32-S3 GPIO47 TX -> M5StickC Plus2 GPIO32 RX
ESP32-S3 GPIO48 RX <- M5StickC Plus2 GPIO33 TX
ESP32-S3 GND       -> M5StickC Plus2 GND
```

## Điều khiển trên M5StickC Plus2

- `BtnA`: open
- `BtnB`: xuống / trang sau
- `BtnPWR`: lên / trang trước
- giữ `BtnPWR`: back
- giữ `BtnA`: remount SD bên `ESP32-S3`

## Lưu ý

- firmware này không mount `SD` cục bộ
- nếu không thấy UI từ `ESP32-S3`, kiểm tra lại `TX33/RX32/GND`
- mọi thao tác duyệt file thực tế diễn ra bên `ESP32-S3`

## Build

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-m5-test
pio run -e m5stickc_plus2
```

## Flash

```bash
cd /home/truonglangquan/idk-code/idk-test/idk-m5-test
pio run -e m5stickc_plus2 -t upload
pio device monitor -b 115200
```

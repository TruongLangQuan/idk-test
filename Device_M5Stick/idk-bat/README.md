# idk-bat
Giám sát pin qua Serial cho Tenstar TS-ESP32-S3 (không dùng màn hình).
## Tính năng
- Đọc ADC pin 3, tính điện áp và % pin (3.0V–4.2V)
- Nháy LED pin 46 để báo đang hoạt động

## Điều khiển
- Không có UI trên màn hình; theo dõi qua Serial 115200

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-bat
pio run -e tenstar-esp32s3
```

## Flash
```bash
pio run -e tenstar-esp32s3 -t upload
```

## Ghi chú
- Output định dạng bảng trên Serial

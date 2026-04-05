# idk-gif-s3
Trình phát GIF cho ESP32-S3 + ST7789 (TFT_eSPI).
## Tính năng
- Chạy GIF từ asset nhúng

## Điều khiển
- BOOT (GPIO0): đổi GIF (vòng)

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-gif-s3
pio run -e esp32s3_114tft
```

## Flash
```bash
pio run -e esp32s3_114tft -t upload
```

## Ghi chú
- Cần cấu hình TFT_eSPI phù hợp board

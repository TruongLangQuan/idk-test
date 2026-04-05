# idk-cyd
UI cảm ứng cho CYD 2.8" (ESP32 + ILI9341).
## Tính năng
- Chọn WiFi bằng cảm ứng
- 3 màn chính: Clock, GIF, Image

## Điều khiển
- Màn WiFi: chạm trái/phải để chọn, chạm giữa để kết nối
- Màn chính: chạm trái/phải để đổi screen; ảnh có chạm trên/dưới để đổi

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-cyd
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Dùng `TFT_eSPI` + `XPT2046_Touchscreen`

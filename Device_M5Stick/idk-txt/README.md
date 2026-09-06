# idk-txt
Trình xem text nhúng cho M5StickC.
## Tính năng
- Đọc `generated/txt_asset.h` theo trang
- Hỗ trợ font tiếng Việt (vi12)

## Điều khiển
- BtnA: trang kế tiếp (giữ để chạy nhanh)
- BtnB hoặc BtnPWR: trang trước (giữ để chạy nhanh)
- Giữ BtnPWR: về trang đầu

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-txt
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Dùng `tools/asset_compiler.py` để build text

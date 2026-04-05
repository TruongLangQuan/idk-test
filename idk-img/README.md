# idk-img
Trình xem ảnh nhúng cho M5StickC.
## Tính năng
- Hiển thị ảnh từ `generated/img_assets.h`

## Điều khiển
- BtnA: ảnh kế tiếp
- BtnB: ảnh trước đó

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-img
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Dùng `tools/asset_compiler.py` để build ảnh

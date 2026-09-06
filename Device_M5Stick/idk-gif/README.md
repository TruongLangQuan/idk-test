# idk-gif
Trình phát GIF từ asset nhúng cho M5StickC.
## Tính năng
- Chạy GIF theo frame delay
- Hỗ trợ nhiều GIF trong `generated/gif_asset.h`

## Điều khiển
- BtnPWR: đổi GIF (vòng)

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-gif
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Dùng `tools/asset_compiler.py` để build GIF

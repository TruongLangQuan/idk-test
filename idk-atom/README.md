# idk-atom
Hiệu ứng đám mây orbital nguyên tử chạy trên M5StickC.
## Tính năng
- 3 kiểu orbital (1s/2p/3d) với chuyển động mượt
- Tối ưu cho màn 240x135

## Điều khiển
- BtnA: orbital kế tiếp
- BtnB: orbital trước đó

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-atom
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Màn hình xoay trái `setRotation(3)`
- Dựa trên ý tưởng kavan010/Atoms

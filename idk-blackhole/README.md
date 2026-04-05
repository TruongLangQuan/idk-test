# idk-blackhole
Hiệu ứng hố đen/đĩa bồi tụ thời gian thực.
## Tính năng
- Trường sao uốn cong và đĩa bồi tụ động
- Hiệu ứng glow quanh chân trời sự kiện

## Điều khiển
- BtnA: tạo lại trường sao
- BtnB: tạm dừng/tiếp tục

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-blackhole
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Màn hình xoay trái `setRotation(3)`
- Dựa trên ý tưởng kavan010/black_hole

# idk-chess
Cờ vua offline có bot đơn giản, chạy trên M5StickC.
## Tính năng
- Luật cơ bản: nhập thành, phong cấp, ăn qua đường
- Chọn WiFi ở màn hình đầu (có thể bỏ qua)

## Điều khiển
- Màn WiFi: BtnA kết nối, BtnB/PWR chọn mạng, A+PWR bỏ qua
- Trên bàn cờ: BtnB/PWR di chuyển, BtnA chọn/đi
- Khi phong cấp/nhập thành sẽ hiện menu chọn

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-chess
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Chơi offline, không cần mạng

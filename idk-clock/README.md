# idk-clock
Đồng hồ NTP (GMT+7) cho M5StickC.
## Tính năng
- Quét WiFi, kết nối và đồng bộ thời gian NTP
- Chế độ offline nếu không có WiFi

## Điều khiển
- Màn WiFi: BtnA kết nối, BtnB/PWR chọn mạng, A+PWR offline

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-clock
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Máy chủ thời gian: pool.ntp.org, time.google.com, time.windows.com

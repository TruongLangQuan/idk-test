# idk-portal
Captive portal AP + DNS, thu thập dữ liệu form.
## Tính năng
- Tạo AP `Truong-Lang-Quan` và DNS hijack
- Lưu danh sách email/password đã submit

## Điều khiển
- BtnB: mục tiếp theo
- BtnPWR: mục trước đó
- BtnA: xem chi tiết mục đang chọn

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-portal
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Chỉ dùng trong môi trường thử nghiệm được phép
- IP AP: 4.3.2.1

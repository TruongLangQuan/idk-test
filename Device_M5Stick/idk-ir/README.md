# idk-ir
IR remote sender với database lệnh nhúng.
## Tính năng
- Danh sách remote + danh sách command
- Tìm kiếm bằng bàn phím
- TX nội/ngoại (IR internal/external)

## Điều khiển
- Danh sách file: BtnB xuống, BtnPWR lên, BtnA chọn
- Mục [TX] để đổi TX, [Search] để lọc
- Danh sách lệnh: BtnA gửi, BtnB/PWR di chuyển, giữ để chạy nhanh

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-ir
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Dữ liệu IR lấy từ `data/ir` qua `tools/asset_compiler.py`

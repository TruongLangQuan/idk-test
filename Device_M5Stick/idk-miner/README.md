# idk-miner
Dashboard theo dõi LTC/BTC (API) cho M5StickC Plus2.
## Tính năng
- Tự kết nối WiFi theo danh sách trong config
- Hiển thị balance/last mined và trạng thái API

## Điều khiển
- BtnA: màn tiếp theo
- BtnB: màn trước đó

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-miner
pio run -e m5stickc_plus2
```

## Flash
```bash
pio run -e m5stickc_plus2 -t upload
```

## Ghi chú
- Cấu hình WiFi & địa chỉ ví trong `include/config.h`
- API: unminable + public-pool

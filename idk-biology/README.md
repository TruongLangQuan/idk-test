# idk-biology
Bộ công cụ sinh học (lớp 9) cho M5StickC Plus2.
## Tính năng
- Phiên mã: DNA mạch gốc/mạch bổ sung → mRNA
- Dịch mã mRNA → axit amin (có start/stop)
- Bổ sung DNA, %GC
- Di truyền Mendel: lai 1 cặp và 2 cặp tính trạng

## Điều khiển
- Menu: BtnA chọn, BtnB xuống, BtnPWR lên
- Bàn phím QWERTY có Telex (VI/EN)

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-biology
pio run -e m5stickc_plus2
```

## Flash
```bash
pio run -e m5stickc_plus2 -t upload
```

## Ghi chú
- Có auto‑connect WiFi theo danh sách có sẵn (không bắt buộc)

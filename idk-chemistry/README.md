# idk-chemistry
Bộ công cụ hoá học cho M5StickC Plus2.
## Tính năng
- Tra bảng tuần hoàn theo ký hiệu hoặc số Z
- Tính khối lượng mol từ công thức
- Cân bằng PTHH (bruteforce giới hạn)

## Điều khiển
- Menu: BtnA chọn, BtnB xuống, BtnPWR lên
- Bàn phím QWERTY có Telex (VI/EN)

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-chemistry
pio run -e m5stickc_plus2
```

## Flash
```bash
pio run -e m5stickc_plus2 -t upload
```

## Ghi chú
- Cân bằng giới hạn: tối đa 6 chất, hệ số 1–8

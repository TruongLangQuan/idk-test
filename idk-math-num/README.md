# idk-math-num
Máy tính khoa học cơ bản cho M5StickC Plus2.
## Tính năng
- Biểu thức: + - * / ^, sin/cos/tan, log/ln, sqrt/abs
- Giải PT bậc 2, hệ 2x2
- Ước chung/lợi bội, kiểm tra nguyên tố, phân tích thừa số
- nCr/nPr, thống kê (mean/std)

## Điều khiển
- Menu: BtnA chọn, BtnB xuống, BtnPWR lên
- Bàn phím QWERTY

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-math-num
pio run -e m5stickc_plus2
```

## Flash
```bash
pio run -e m5stickc_plus2 -t upload
```

## Ghi chú
- Kết quả hiển thị tối đa 6 chữ số thập phân

# idk-linux
Hiệu ứng “idk-linux” gồm fastfetch/cmatrix/pipe.
## Tính năng
- Fastfetch: thông tin hệ thống
- CMatrix: mưa ký tự
- Pipe: hiệu ứng đường ống

## Điều khiển
- BtnA hoặc BtnB: mode kế tiếp
- BtnPWR: mode trước đó

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-linux
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

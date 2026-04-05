# idk-video
Trình phát MJPEG từ SD hoặc LittleFS.
## Tính năng
- Chọn nguồn SD/LittleFS
- Duyệt file `.mjpg/.mjpeg` và phát khung hình

## Điều khiển
- Source menu: BtnA chọn, BtnB xuống, BtnPWR lên
- File menu: BtnA phát, BtnB xuống, BtnPWR lên, giữ PWR để back
- Playback: BtnA restart, BtnB pause/resume, BtnPWR quay lại

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-video
pio run -e firmware
```

## Flash
```bash
pio run -e firmware -t upload
```

## Ghi chú
- Khuyến nghị 240x135 @ 10fps
- Upload LittleFS: `pio run -t uploadfs`

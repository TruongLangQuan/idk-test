# idk-video
Trình phát MJPEG từ SD.
## Tính năng
- Duyệt thư mục trên SD (hiện folder + file video)
- Phát file `.mjpg/.mjpeg`
- Đọc phụ đề `.srt` cùng tên (cùng thư mục) và phát qua UDP

## Điều khiển
- File menu: BtnA mở folder/phát, BtnB xuống, BtnPWR lên, giữ PWR để lên thư mục cha
- Playback: BtnA pause/resume, BtnB skip +10s, giữ BtnPWR để quay lại

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
- M5StickC Plus2 phát AP: SSID `dhe` / pass `dhe`, subtitle UDP port `4210`

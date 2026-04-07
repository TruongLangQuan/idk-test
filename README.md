# idk-test

Bộ firmware thử nghiệm cho nhiều board; mỗi thư mục là một project PlatformIO độc lập. Mỗi project đều có `README.md` riêng để xem chi tiết tính năng và điều khiển.

## Danh sách firmware

| Project | Board/Env | Mô tả ngắn |
| --- | --- | --- |
| `idk-atom` | M5StickC / `firmware` | Hiệu ứng orbital nguyên tử |
| `idk-bat` | Tenstar ESP32-S3 / `tenstar-esp32s3` | Battery monitor qua Serial |
| `idk-biology` | M5StickC Plus2 / `m5stickc_plus2` | Công cụ sinh học (phiên mã/dịch mã/di truyền) |
| `idk-blackhole` | M5StickC / `firmware` | Hiệu ứng hố đen |
| `idk-chemistry` | M5StickC Plus2 / `m5stickc_plus2` | Bảng tuần hoàn + khối lượng mol + cân bằng PTHH |
| `idk-chess` | M5StickC / `firmware` | Cờ vua offline + bot |
| `idk-clock` | M5StickC / `firmware` | Đồng hồ NTP GMT+7 |
| `idk-cyd` | CYD-2432S028 / `firmware` | UI cảm ứng: clock/gif/image |
| `idk-gif` | M5StickC / `firmware` | Trình phát GIF nhúng |
| `idk-gif-s3` | ESP32-S3 + ST7789 / `esp32s3_114tft` | Trình phát GIF nhúng |
| `idk-img` | M5StickC / `firmware` | Trình xem ảnh nhúng |
| `idk-ir` | M5StickC / `firmware` | IR remote sender + search |
| `idk-linux` | M5StickC / `firmware` | Fastfetch/CMatrix/Pipe |
| `idk-math-num` | M5StickC Plus2 / `m5stickc_plus2` | Máy tính khoa học |
| `idk-miner` | M5StickC Plus2 / `m5stickc_plus2` | Dashboard LTC/BTC |
| `idk-physics` | M5StickC Plus2 / `m5stickc_plus2` | Công cụ vật lý cơ bản |
| `idk-portal` | M5StickC / `firmware` | Captive portal AP + DNS |
| `idk-translator` | M5StickC Plus2 / `m5stickc_plus2` | Dịch EN↔VI + dictionary |
| `idk-txt` | M5StickC / `firmware` | Trình xem text nhúng |
| `idk-video` | M5StickC / `firmware` | MJPEG player (SD, folder browser) |
| `idk-mine` | Suite nhiều project | Xem `idk-mine/README.md` |

## Targets

- M5StickC series (board `m5stick-c`, 240x135): `idk-gif`, `idk-txt`, `idk-img`, `idk-chess`, `idk-clock`, `idk-ir`, `idk-blackhole`, `idk-atom`, `idk-video`, `idk-portal`, `idk-linux`
- M5StickC Plus2 (`m5stickc_plus2`): `idk-miner`, `idk-translator`, `idk-math-num`, `idk-chemistry`, `idk-biology`, `idk-physics`
- CYD-2432S028 (ESP32 + ILI9341 320x240): `idk-cyd`
- ESP32-S3 devkit + ST7789 135x240: `idk-gif-s3`
- Tenstar ESP32-S3: `idk-bat`
- Suite đa project: `idk-mine`

## Build và flash

Mỗi project có `platformio.ini` riêng. Cú pháp chung:

```bash
cd /home/truonglangquan/idk-code/idk-test/<project>
pio run -e <env>
pio run -e <env> -t upload
pio device monitor -b 115200
```

Env thường gặp:

- `firmware`: hầu hết project M5StickC
- `m5stickc_plus2`: các project M5StickC Plus2
- `esp32s3_114tft`: `idk-gif-s3`
- `tenstar-esp32s3`: `idk-bat`

Filesystem (nếu có):

```bash
pio run -e <env> -t uploadfs
```

## Asset pipeline

Nguồn assets dùng chung:

- `gifs`: `/home/truonglangquan/idk-code/data/gifs`
- `img`: `/home/truonglangquan/idk-code/data/img`
- `txt`: `/home/truonglangquan/idk-code/data/txt`
- `ir`: `/home/truonglangquan/idk-code/data/ir`

Dùng tool compile assets:

```bash
cd /home/truonglangquan/idk-code/idk-test
python3 -m venv .venv
source .venv/bin/activate
pip install -r tools/requirements.txt
python3 tools/asset_compiler.py all  # mặc định xoay ngang trái
```

Tool sẽ:

- resize/fit `img`, `gif` về 240x135
- compile `img`, `gif`, `txt`, `ir` thành header C++ built-in (`PROGMEM`)
- export ảnh đã chuẩn hóa ra `.png` hoặc `.bmp` nếu yêu cầu

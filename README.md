# idk-test

Bộ firmware thử nghiệm cho nhiều board; mỗi thư mục là một project PlatformIO độc lập.

## Targets

- M5StickC series (board `m5stick-c`, 240x135): `idk-gif`, `idk-txt`, `idk-img`, `idk-chess`, `idk-clock`, `idk-ir`, `idk-blackhole`, `idk-atom`, `idk-video`, `idk-portal`, `idk-linux`, `idk-miner`, `idk-translator`
- CYD-2432S028 (ESP32 + ILI9341 320x240): `idk-cyd`
- ESP32-S3 devkit + ST7789 135x240: `idk-gif-s3`
- Tenstar ESP32-S3 4MB + BMP280: `idk-bat`
- Suite đa project: `idk-mine` (xem `idk-mine/README.md`)

## Ghi chú nhanh theo project

- `idk-blackhole`: dựa trên `kavan010/black_hole`.
- `idk-atom`: dựa trên `kavan010/Atoms`.
- `idk-video`: trình phát MJPEG, hỗ trợ nguồn từ SD và LittleFS, cần file `.mjpg/.mjpeg` 240x135.
- `idk-cyd`: dùng `TFT_eSPI` + `XPT2046_Touchscreen` (ILI9341 + touch).
- `idk-miner`, `idk-translator`: dùng `ArduinoJson` và phân vùng `huge_app.csv`.

## Cấu trúc tổng quan

```
idk-test/
  tools/
    asset_compiler.py
    requirements.txt
  idk-gif/
  idk-txt/
  idk-img/
  idk-chess/
  idk-clock/
  idk-ir/
  idk-blackhole/
  idk-atom/
  idk-video/
  idk-portal/
  idk-linux/
  idk-miner/
  idk-translator/
  idk-cyd/
  idk-gif-s3/
  idk-bat/
  idk-mine/
```

## Build và flash

Mỗi project có `platformio.ini` riêng. Cú pháp chung:

```bash
cd /home/truonglangquan/idk-code/idk-test/<project>
pio run -e <env>
pio run -e <env> -t upload
pio device monitor -b 115200
```

Env mặc định theo project:

- `firmware`: hầu hết project M5StickC series và `idk-cyd`, `idk-video`, `idk-blackhole`, `idk-atom`, `idk-clock`, `idk-chess`
- `m5stickc_plus2`: `idk-miner`, `idk-translator`
- `esp32s3_114tft`: `idk-gif-s3`
- `tenstar-esp32s3`: `idk-bat`
- `idk-mine`: dùng script trong `idk-mine/scripts` (xem README riêng)

Filesystem (nếu có):

```bash
pio run -e firmware -t uploadfs
```

Gợi ý: `idk-video` dùng LittleFS và phân vùng `partitions.csv`.

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

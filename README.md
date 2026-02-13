# idk-test

Bộ firmware gồm 6 project độc lập:

1. `idk-gif` (M5StickC Plus2, 240x135)
2. `idk-txt` (M5StickC Plus2, 240x135)
3. `idk-img` (M5StickC Plus2, 240x135)
4. `idk-chess` (M5StickC Plus2, 240x135)
5. `idk-ir` (M5StickC Plus2, 240x135)
6. `idk-cyd` (CYD-2432S028, 320x240)

Assets nguồn dùng chung:

- `gifs`: `/home/truonglangquan/idk-code/data/gifs`
- `img`: `/home/truonglangquan/idk-code/data/img`
- `txt`: `/home/truonglangquan/idk-code/data/txt`
- `ir`: `/home/truonglangquan/idk-code/data/ir`

## Cấu trúc

```
idk-test/
  tools/
    asset_compiler.py
    requirements.txt
  idk-gif/
    include/generated/
    src/main.cpp
    platformio.ini
  idk-txt/
    include/generated/
    src/main.cpp
    platformio.ini
  idk-img/
    include/generated/
    src/main.cpp
    platformio.ini
  idk-chess/
    include/generated/
    src/main.cpp
    platformio.ini
  idk-ir/
    include/generated/
    src/main.cpp
    platformio.ini
  idk-cyd/
    include/generated/
    src/main.cpp
    platformio.ini
```

## Dùng tool compile assets

```bash
cd /home/truonglangquan/idk-code/idk-test
python3 -m venv .venv
source .venv/bin/activate
pip install -r tools/requirements.txt
python3 tools/asset_compiler.py all  # mac dinh xoay ngang trai
```

Tool sẽ:

- resize/fit `img`, `gif` về 240x135
- compile `img`, `gif`, `txt` thành header C++ built-in (`PROGMEM`)
- export ảnh đã chuẩn hóa ra `.png` hoặc `.bmp` nếu yêu cầu

# idk-ts-doom

Project scaffold cho port Doom thật trên `Tenstar TS-ESP32-S3`.

Hiện có:

- init màn `ST7789 135x240`
- mount `microSD`
- scan WAD trong `/doom`
- map `5-way` để chọn WAD
- cấu trúc project sẵn để gắn `doomgeneric`

Mapping phần cứng:

- `microSD`: `CS=13`, `MOSI=11`, `CLK=12`, `MISO=10`
- `5-way`: `UP=14`, `DOWN=17`, `LEFT=16`, `RIGHT=18`, `CENTER=15`

Trạng thái:

- đây là `scaffold`, chưa phải bản Doom chạy được
- mục tiêu tiếp theo là cấy `doomgeneric` từ `M5Doom` vào project này, bỏ sound, đọc WAD từ SD, rồi remap 5-way thành move/turn/fire/use

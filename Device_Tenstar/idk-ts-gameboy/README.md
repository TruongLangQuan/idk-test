# idk-ts-gameboy

Firmware Game Boy cho `Tenstar TS-ESP32-S3` dùng:

- `microSD`: `CS=13`, `MOSI=11`, `CLK=12`, `MISO=10`
- `5-way`: `UP=14`, `DOWN=17`, `LEFT=16`, `RIGHT=18`, `CENTER=15`

ROM:

- chép `.gb` hoặc `.gbc` vào `/gameboy` trên thẻ nhớ
- firmware sẽ scan `/gameboy`, nếu không có sẽ scan `/`

Điều khiển:

- menu: `UP/DOWN` chọn ROM, `CENTER` để boot
- trong game:
  - `UP/DOWN/LEFT/RIGHT`: D-pad
  - `CENTER`: `A`
  - giữ `CENTER`: `B`
  - giữ `UP`: `START`
  - giữ `DOWN`: `SELECT`

Ghi chú:

- đây là bản port thực dụng từ repo `esp32-gameboy`, đổi sang màn `ST7789 135x240`
- ảnh Game Boy được scale để vừa màn hình Tenstar
- chưa có save state và chưa có menu thoát trong lúc đang chơi

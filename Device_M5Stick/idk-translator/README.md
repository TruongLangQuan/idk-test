# idk-translator
Dịch Anh↔Việt với keyboard và WebUI.
## Tính năng
- Translate qua LibreTranslate (cấu hình endpoint)
- Dictionary EN: IPA + nghĩa theo POS (noun/verb/adj/adv)
- WebUI hiển thị input/output/dictionary

## Điều khiển
- Menu: BtnA chọn, BtnB xuống, BtnPWR lên
- Bàn phím QWERTY có Telex (VI/EN)

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-translator
pio run -e m5stickc_plus2
```

## Flash
```bash
pio run -e m5stickc_plus2 -t upload
```

## Ghi chú
- Dictionary dùng api.dictionaryapi.dev
- Cần WiFi để dịch và tra từ

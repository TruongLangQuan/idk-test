# idk-physics
Công cụ vật lý cơ bản cho M5StickC Plus2.
## Tính năng
- Ohm: V, I, R + công suất
- Điện trở nối tiếp/song song
- F=ma, động lượng, động năng, thế năng
- v=s/t, a=(v-v0)/t, định luật Snell

## Điều khiển
- Menu: BtnA chọn, BtnB xuống, BtnPWR lên

## Build
```bash
cd /home/truonglangquan/idk-code/idk-test/idk-physics
pio run -e m5stickc_plus2
```

## Flash
```bash
pio run -e m5stickc_plus2 -t upload
```

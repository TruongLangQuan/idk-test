# idk-motion-ascii

**Animated ASCII renderer** for M5StickC Plus 2 with multiple density modes.

## Features

- Animated 3D ASCII rendering
- Multiple density modes: detailed, standard, minimal, block, dots
- Real-time animation with frame counter
- Landscape display (rotation = 3)
- 5-way tactile switch controls

## ASCII Modes

### detailed
Full Unicode box-drawing + shading characters
- ▓ ▒ ░ █ ■ ◆ ● ○ ◉ ◎ ◈ ∎ ▰ ▱ ▲ △ ▼ ▽

### standard
ASCII-only characters (no Unicode)
- # @ % * + - = ~ ` + borders

### minimal
Sparse rendering
- # . - | + / \ O

### block
ASCII block patterns
- # □ ■ ▪ ▫ █ ░

### dots
Dot matrix style
- . , : ; ' " ^ v < >

## Controls

- 5-way UP/DOWN: cycle through animation speed
- 5-way LEFT/RIGHT: switch between ASCII modes
- 5-way CENTER: pause/resume animation
- PWR: reset position

## Build

```bash
cd idk-motion-ascii
pio run -e m5stickc_plus2 -t upload
pio device monitor -b 115200
```

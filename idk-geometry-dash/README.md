# idk-geometry-dash

**Geometry Dash-inspired platformer** for M5StickC Plus 2, ported from the OpenGD project.

## Features

- Rhythm-based platformer gameplay
- Colorful geometric obstacles
- Multiple levels with increasing difficulty
- Particle effects and animations
- Score/time tracking
- Simple control with 5-way switch
- Landscape display (rotation = 3)

## Gameplay

Guide your geometric character through obstacle-filled levels. Time your jumps to the music rhythm to avoid spikes, blocks, and other hazards.

## 5-way Button Mapping

```
CENTER / UP = Jump / Fly (hold for longer)
DOWN  = Fall faster / Reverse mode
LEFT/RIGHT = Turn around
PWR   = Menu / Pause
```

## Game Mechanics

- **Normal Mode**: Jump over obstacles
- **Reverse Mode**: Upside-down controls
- **Fly Mode**: Hold jump to fly through sections
- **Speed Ramps**: Sections with boosted movement

## Level Progression

- 5 starter levels (difficulty 1-3)
- 5 intermediate levels (difficulty 4-6)
- 5 advanced levels (difficulty 7-10)
- Custom level editor (planned)

## Graphics

- Simple colored blocks and shapes
- Particle explosion on collision
- Animated background
- Score display

## Build

```bash
cd idk-geometry-dash
pio run -e m5stickc_plus2 -t upload
pio device monitor -b 115200
```

## Notes

- Original concept from Geometry Dash (RobTop)
- OpenGD was used as reference for porting logic
- M5StickC Plus 2 version optimized for small screen

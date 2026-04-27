# idk-ascii-doom

**DOOM-style ASCII dungeon crawler** for M5StickC Plus 2.

## Features

- First-person ASCII rendering (raycasting style)
- Procedural dungeon generation
- Multiple enemy types (Demon, Cyborg, Spider, etc.)
- Weapon system (Pistol, Shotgun, Plasma Rifle)
- Ammo management
- Health/Armor system
- Mini-map view
- Landscape display (rotation = 3)

## Gameplay

1. Navigate the dungeon using 5-way switch
2. Turn to face enemies (LEFT/RIGHT rotate view)
3. Move forward/backward (UP/DOWN)
4. Aim and shoot with CENTER button
5. Collect ammo, health packs, armor

## 5-way Button Mapping

```
UP    = Move forward / Look up
DOWN  = Move backward / Look down
LEFT  = Rotate view left / Turn left
RIGHT = Rotate view right / Turn right
CENTER= Shoot weapon
PWR   = Weapon select menu
```

## ASCII Rendering

The game uses a raycasting engine to draw ASCII representation of walls:
- `#` `█` = wall (close)
- `@` `▒` = wall (medium)
- `-` `░` = wall (far)
- ` ` = empty

## Enemy Types

- **Demon** (D): 20 HP, slow, melee
- **Cyborg** (C): 40 HP, fast, ranged
- **Spider** (S): 15 HP, very fast, weak
- **Spectre** (G): 30 HP, invisible phase attacks

## Weapons

- **Pistol**: 10 ammo, 5 damage
- **Shotgun**: 12 ammo, 15 damage (heavy recoil)
- **Plasma Rifle**: 40 ammo, 20 damage

## Build

```bash
cd idk-ascii-doom
pio run -e m5stickc_plus2 -t upload
pio device monitor -b 115200
```

## Notes

- Turn-based combat (player takes action, then enemy)
- Ammo is limited; find supply caches
- Health regenerates slowly when standing still
- The goal is to reach the level exit alive

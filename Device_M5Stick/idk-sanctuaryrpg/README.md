# idk-sanctuaryrpg

**Text-based roguelike RPG** for M5StickC Plus 2, inspired by SanctuaryRPG.

## Features

- Procedural dungeon generation
- ASCII-based UI and graphics
- Turn-based combat system
- Character progression (level, stats, skills)
- Item system (weapons, armor, potions)
- Multiple enemy types
- Save/Load game state
- Landscape display (rotation = 3)

## Gameplay

1. **Exploration**: Navigate dungeons using 5-way switch
2. **Combat**: Turn-based fights with enemy AI
3. **Progression**: Gain experience, level up, find loot
4. **Death**: Permadeath mode (can be toggled)

## 5-way Button Mapping

```
UP    = Move north
DOWN  = Move south
LEFT  = Move west
RIGHT = Move east
CENTER= Interact with NPC/item, attack
PWR   = Menu (inventory, stats, quit)
```

## Game Stats

- HP: Hit Points
- STR: Strength (attack power)
- DEF: Defense (damage mitigation)
- INT: Intelligence (magic power, unused for now)
- XP: Experience points

## Controls in Combat

- UP/DOWN/LEFT/RIGHT: Choose attack type or defense
- CENTER: Execute action
- PWR: Flee from combat

## Save/Load

Game state saved to SPIFFS automatically after each turn.
- `/sanctuary_save.dat` - current game state
- `/sanctuary_stats.dat` - all-time statistics

## Build

```bash
cd idk-sanctuaryrpg
pio run -e m5stickc_plus2 -t upload
pio run -e m5stickc_plus2 -t uploadfs
pio device monitor -b 115200
```

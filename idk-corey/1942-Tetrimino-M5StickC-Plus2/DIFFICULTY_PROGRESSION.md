# 1942 Tetrimino Edition - Difficulty Progression

## 📊 Complete Difficulty Scaling System

### Enemy Movement Speed
- **Formula**: 1.0 + (wave - 1) × 0.15
- **Wave 1**: 1.0x (base speed)
- **Wave 2**: 1.15x (+15%)
- **Wave 3**: 1.3x (+30%)
- **Wave 4**: 1.45x (+45%)
- **Wave 5**: 1.6x (+60%)
- **Wave 10**: 2.35x (+135%)
- **Wave 14**: 2.95x (+195%)
- **Wave 14+**: **3.0x MAXED** (capped)

### Enemy Spawn Rate (More Enemies)
- **Formula**: max(600, 1200 - (wave - 1) × 100) milliseconds
- **Wave 1**: 1200ms (0.83 enemies/sec)
- **Wave 2**: 1100ms (0.91 enemies/sec) +9%
- **Wave 3**: 1000ms (1.0 enemies/sec) +20%
- **Wave 4**: 900ms (1.11 enemies/sec) +33%
- **Wave 5**: 800ms (1.25 enemies/sec) +50%
- **Wave 7**: 600ms (1.67 enemies/sec) +100%
- **Wave 7+**: **600ms MAXED**

### Enemy Shooting Rate
- **Formula**: max(400, 1000 - (wave - 1) × 50) milliseconds
- **Wave 1**: 1000ms
- **Wave 2**: 950ms (5% faster)
- **Wave 3**: 900ms (10% faster)
- **Wave 5**: 800ms (20% faster)
- **Wave 10**: 550ms (45% faster)
- **Wave 13**: 400ms (60% faster)
- **Wave 13+**: **400ms MAXED**

### Boss Shooting Rate
- **Formula**: max(200, 500 - (wave - 1) × 100) milliseconds
- **Wave 1**: 500ms
- **Wave 2**: 400ms (20% faster)
- **Wave 3**: 300ms (40% faster)
- **Wave 4**: 200ms (60% faster) - **BULLET HELL!**
- **Wave 4+**: **200ms MAXED**

### Boss Health
- **Formula**: 20 + (wave - 1) × 5
- **Wave 1**: 20 HP
- **Wave 2**: 25 HP
- **Wave 3**: 30 HP
- **Wave 5**: 40 HP
- **Wave 10**: 65 HP
- **Wave 20**: 115 HP
- **NO CAP** - Scales forever!

### Boss Size
- **Formula**: min(2.0 + (wave - 1) × 0.5, 8.0)
- **Wave 1**: 2.0x (8 pixel blocks)
- **Wave 2**: 2.5x (10 pixel blocks)
- **Wave 3**: 3.0x (12 pixel blocks)
- **Wave 5**: 4.0x (16 pixel blocks)
- **Wave 10**: 6.5x (26 pixel blocks)
- **Wave 13**: 8.0x (32 pixel blocks) - **HUGE!**
- **Wave 13+**: **8.0x MAXED**

---

## 🎮 Wave Composition

### Waves 1-5: Regular Tetriminos
- 6 tetrimino shapes (I, O, L, J, S, Z)
- Wave 2+: 30% chance of large enemies (1.5x size)
- Pure Tetris gameplay

### Wave 6: TETRIS Letters & Symbols
- 50% TETRIS letters (T-E-T-R-I-S in sequence)
- 50% Strange symbols (8 unique patterns)
- Same difficulty scaling as other waves

### Wave 7+: Everything Mixed
- 33% Tetriminos (including large variants)
- 33% TETRIS letters
- 33% Strange symbols
- Maximum variety and challenge

---

## 🏆 Max Difficulty Reached

When **ALL** caps are hit around **Wave 14-15**:
- Enemy speed: 3.0x
- Spawn rate: 600ms (1.67/sec)
- Enemy fire: 400ms
- Boss fire: 200ms
- Boss size: 8.0x

**After this, only Boss Health continues to scale infinitely.**

The game becomes truly endless - same maximum difficulty, but bosses get tankier!

---

## 💡 Design Notes

### Balanced Scaling
- Early game (Waves 1-3): Learning curve
- Mid game (Waves 4-7): Ramping challenge
- Late game (Waves 8-13): Maximum intensity building
- End game (Wave 14+): Endless at peak difficulty

### Safety Caps
All scaling has caps to prevent:
- Enemies moving too fast to dodge
- Spawn rates causing screen clutter
- Fire rates becoming impossible
- Boss sizes going off-screen

### Why Boss Health is Uncapped
- Gives endless progression
- Rewards player skill and power-ups
- Creates meaningful long-term challenge
- Prevents infinite stalemates

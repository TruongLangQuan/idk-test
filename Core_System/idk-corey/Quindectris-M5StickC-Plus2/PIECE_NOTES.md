# Quintris Piece Rotation Notes

## ✅ CRITICAL: Rotation Direction (FROM THREETRIS SUCCESS)
**ALL pieces MUST rotate CLOCKWISE (to the right)**

In the code, this means:
```cpp
int newRot = (currentRot + 1) % 4;  // +1 gives clockwise rotation
```

**NOTE: ThreeTris analysis shows they use +1, not +3!**

## 🎯 WALL KICK SYSTEM - **WORKING PERFECTLY!**
✅ **6-Step Wall Kick Algorithm (from ThreeTris v3.3)**
1. Direct rotation
2. Kick right (+1)
3. Kick left (-1)
4. Kick up (-1 Y)
5. Kick right 2 (+2) - for left wall
6. Kick left 2 (-2) - for right wall

**This system works flawlessly for spins and wall rotations on both sides!**

---

## Completed Pieces (3/7+)
1. ✅ **I-Triomino** - Straight line of 3 blocks (horizontal/vertical)
2. ✅ **L-Triomino** - Corner going right (4 rotations)
3. ✅ **J-Triomino** - Corner going left (4 rotations, mirror of L)

---

## 🧩 Pentomino Pieces for Quintris (5 blocks each)
**TO BE DETERMINED** - Will research actual Quintris pieces tomorrow

Standard Pentominoes include:
- I-Pentomino (5-block line)
- L-Pentomino (4+1 L-shape)  
- T-Pentomino (T with 5 blocks)
- And others...

**Will get exact piece list from handheld Quintris gameplay**

---

## 📐 Coordinate Format (EXACT - Same as ThreeTris)
```cpp
// Format: [piece][rotation][axis][block0, block1, block2, block3, block4]
// axis 0 = Y coords
// axis 1 = X coords  
// Pivot always at (0,0)

int tempPieces[NUM_PIECES][4][2][5] = {  // Changed from [3] to [5] blocks
  {
    // R0
    { { y0, y1, y2, y3, y4 }, { x0, x1, x2, x3, x4 } },
    // R90 (clockwise)
    { { y0, y1, y2, y3, y4 }, { x0, x1, x2, x3, x4 } },
    // etc...
  }
};
```

## 🔄 Key Changes from ThreeTris to Quintris:
1. **Array size**: `[3]` → `[5]` blocks per piece
2. **Loop updates**: `for (int i = 0; i < 3; i++)` → `for (int i = 0; i < 5; i++)`
3. **New piece definitions** with proper clockwise rotation
4. **More complex collision detection** due to larger pieces

## ✅ What's Already Perfect (Don't Change!):
- **Clockwise rotation system**
- **6-step wall kick algorithm** 
- **Coordinate system and pivot at (0,0)**
- **Hardware setup and controls**
- **Display and game mechanics**

---

## 🎯 Testing Checklist (Same as ThreeTris):
- [ ] Piece spawns correctly
- [ ] Rotates CLOCKWISE (to the right)
- [ ] All 4 rotations work (or 2 for symmetrical pieces)  
- [ ] No morphing/glitching
- [ ] Collision detection works
- [ ] Wall kicks work on both walls
- [ ] Piece places correctly on field
- [ ] **Spins work in tight spaces**

**The rotation foundation is rock solid - ThreeTris proved it works perfectly!**

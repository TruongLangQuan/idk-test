# Mixed Tetris Debug Log

## 🐛 **Problem Found & Fixed**

### **Issue:** 
New tetromino pieces (I, T, O) were not appearing in the game even though they were defined.

### **Root Cause:**
The `newPiece()` function was still hardcoded to `random(0, 8)` instead of `random(0, 11)`.

### **Fix Applied:**
```cpp
// BEFORE (broken):
nextPiece = random(0, 8);  // Only original 8 pieces

// AFTER (fixed): 
nextPiece = random(0, 11); // All 11 pieces including tetrominoes
```

### **Files Changed:**
- `src/tetris.cpp` - Lines 478 and 480 in `newPiece()` function

---

## 🎮 **Current Piece Lineup (11 Total):**

### **Triomino Pieces (0-7):**
0. I-Triomino (cyan) - 3 blocks
1. L-Triomino (blue) - 3 blocks  
2. J-Triomino (orange) - 3 blocks
3. Single Block (yellow) - 1 block
4. Double Block (green) - 2 blocks
5. Diagonal Double (red) - 2 blocks
6. Diagonal Triple (purple) - 3 blocks
7. T-minus (white) - 3 blocks

### **Tetromino Pieces (8-10):**
8. **I-Tetromino** (cyan) - 4 blocks straight line
9. **T-Tetromino** (purple) - 4 blocks T-shape  
10. **O-Tetromino** (yellow) - 4 blocks 2x2 square

---

## ✅ **Verification:**

### **Build Status:** ✅ Success
- RAM: 8.2% (26,968 bytes)
- Flash: 34.1% (446,433 bytes)

### **Upload Status:** ✅ Success  
- Flashed to M5StickC Plus2 at /dev/ttyACM0

### **Expected Behavior:**
- You should now see **I-Tetromino (long 4-block line)**, **T-Tetromino (T-shape)**, and **O-Tetromino (2x2 square)** appearing randomly with the original triomino pieces
- All pieces rotate **clockwise** 
- Wall kicks work for all pieces

---

## 🎯 **Test Instructions:**
1. Power on M5StickC Plus2
2. Wait for pieces to spawn 
3. Look for:
   - **Long 4-block lines** (I-Tetromino)
   - **T-shaped pieces** (T-Tetromino) 
   - **2x2 squares** (O-Tetromino)
4. Test rotation with joystick button (should be clockwise)
5. Test wall kicks near edges

## 🔄 **T-Tetromino Morphing Issue - FIXED**

### **Problem:** 
T-tetromino was "morphing" (changing shape) instead of properly rotating - one rotation position looked like a triomino.

### **Root Cause:**
Incorrect coordinate definitions for T-tetromino rotations.

### **Fix Applied:**
Updated T-tetromino coordinates to proper tetromino rotations:

```cpp
// R0:  #     (4 blocks: top, left, center, right)
//     ###  
{{ -1,  0,  0,  0 }, {  0, -1,  0,  1 }},

// R90: #     (4 blocks: top, center, right, bottom) 
//      ##
//      #
{{ -1,  0,  0,  1 }, {  0,  0,  1,  0 }},

// R180: ###  (4 blocks: left, center, right, bottom)
//        #
{{  0,  0,  0,  1 }, { -1,  0,  1,  0 }},

// R270:  #   (4 blocks: top, left, center, bottom)
//       ##
//        #
{{ -1,  0,  0,  1 }, {  0, -1,  0,  0 }}
```

### **Status:** ✅ **FIXED** - T-tetromino now rotates properly without morphing!

---

## 🎯 **L & J Tetrominoes Added - v1.1**

### **New Pieces Added:**
- **L-Tetromino** (piece #11) - Classic orange L-shape
- **J-Tetromino** (piece #12) - Classic blue reverse L-shape

### **Total Pieces Now:** 13 pieces
- **Triominoes**: 8 pieces (1-3 blocks each)
- **Tetrominoes**: 5 pieces (4 blocks each)

### **Rotation Verified:**
Both L and J tetrominoes rotate **clockwise** with proper 4-block shapes:

**L-Tetromino clockwise rotations:**
```
R0:  #     R90: ###    R180: ##     R270:   #
     #          #            #           ###
     ##                      #
```

**J-Tetromino clockwise rotations:**
```
R0:   #    R90: #      R180: ##     R270: ###
      #        ###           #             #
     ##                      #
```

### **Build Stats:**
- **RAM**: 8.3% (27,224 bytes) 
- **Flash**: 34.1% (446,697 bytes)
- **Status**: ✅ Success - Built and flashed

### **Status:** ✅ **MorphTris v1.1 with L & J tetrominoes ready for testing!**
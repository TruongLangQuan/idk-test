/*
 * Columns for M5StickC Plus2
 * Match-3 puzzle game styled like Tetris
 * 
 * Based on Tetris-2P-Battle foundation
 */

#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"

#define Disp M5.Lcd

// JoyC axis indices
#define POS_X 0
#define POS_Y 1

// Display configuration (matching Tetris)
#define offsetx 14
#define offsety 20
#define blockSize 11
#define fieldx 10
#define fieldy 20

// Game constants
#define NUM_COLORS 5  // Red, Yellow, Blue, White, Green (with X)

// Global variables
UNIT_JOYC Joystick;

// Game grid (0 = empty, 1-6 = gem colors)
byte field[fieldy][fieldx];

// Current falling piece (3 gems in vertical column)
struct {
    int x;
    int y;
    byte gems[3];  // top, middle, bottom
    bool active;
} currentPiece;

// Game state
int score = 0;
int level = 1;
long unsigned speed = 500;
long unsigned timez;
long unsigned lockDelayStart = 0;  // Track when piece started landing
bool lockDelayActive = false;
const int LOCK_DELAY_MS = 400;  // 400ms lock delay
bool gameOver = false;
bool gamePaused = false;

// Gem colors - 5 COLORS (Red, Yellow, Blue, White with dot, Green with X)
// RGB565 format: RRRRR GGGGGG BBBBB
uint16_t gemColors[NUM_COLORS + 1] = {
    0x0000,      // 0 = empty (BLACK)
    0xF800,      // 1 = RED (pure bright red)
    0xFFE0,      // 2 = YELLOW (bright yellow)
    0x001F,      // 3 = BLUE (pure blue)
    0xFFFF,      // 4 = WHITE (pure white with black dot)
    0x0400       // 5 = GREEN (grass green with black X)
};

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void drawGhostPiece();
boolean test(int y, int x);

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

// Draw a single block/gem
void blockDraw(int y, int x, int colorz) {
    int screenX = offsetx + (x * blockSize);
    int screenY = offsety + (y * blockSize);
    
    Disp.fillRect(screenX, screenY, blockSize - 1, blockSize - 1, gemColors[colorz]);
    
    int centerX = screenX + (blockSize / 2);
    int centerY = screenY + (blockSize / 2);
    
    // Add black dot to white gems (colorz == 4)
    if (colorz == 4) {
        Disp.fillCircle(centerX, centerY, 1, BLACK);  // Tiny dot
        // Also add small X for extra distinction
        Disp.drawLine(screenX + 3, screenY + 3, screenX + blockSize - 4, screenY + blockSize - 4, BLACK);
        Disp.drawLine(screenX + blockSize - 4, screenY + 3, screenX + 3, screenY + blockSize - 4, BLACK);
    }
    
    // Add black X to green gems (colorz == 5)
    if (colorz == 5) {
        // Draw X using two diagonal lines
        Disp.drawLine(screenX + 2, screenY + 2, screenX + blockSize - 3, screenY + blockSize - 3, BLACK);
        Disp.drawLine(screenX + blockSize - 3, screenY + 2, screenX + 2, screenY + blockSize - 3, BLACK);
    }
}

// Draw the entire playfield
void showField(bool showPiece) {
    // Draw placed gems
    for (int y = 0; y < fieldy; y++) {
        for (int x = 0; x < fieldx; x++) {
            if (field[y][x] != 0) {
                blockDraw(y, x, field[y][x]);
            } else {
                blockDraw(y, x, 0);
            }
        }
    }
    
    // Draw ghost piece first (behind current piece)
    if (showPiece && currentPiece.active) {
        drawGhostPiece();
    }
    
    // Draw current falling piece on top
    if (showPiece && currentPiece.active && currentPiece.y >= 0) {
        for (int i = 0; i < 3; i++) {
            if (currentPiece.y + i >= 0 && currentPiece.y + i < fieldy) {
                blockDraw(currentPiece.y + i, currentPiece.x, currentPiece.gems[i]);
            }
        }
    }
}

// Draw game border
void drawGameBorder() {
    Disp.drawRect(offsetx - 2, offsety - 2, 
                  fieldx * blockSize + 3, fieldy * blockSize + 3, WHITE);
}

// Draw score
void scoreBoardWrite(int scorez) {
    Disp.fillRect(0, 0, 135, 18, BLACK);
    Disp.setTextColor(WHITE);
    Disp.setTextSize(1);
    Disp.setCursor(2, 2);
    Disp.printf("SCORE:%d", scorez);
    Disp.setCursor(2, 10);
    Disp.printf("LVL:%d", level);
}

// ============================================================================
// GAME LOGIC
// ============================================================================

// Initialize field
void initField() {
    for (int y = 0; y < fieldy; y++) {
        for (int x = 0; x < fieldx; x++) {
            field[y][x] = 0;
        }
    }
}

// Random gem color (1-6)
byte randomGem() {
    return random(1, NUM_COLORS + 1);
}

// Test if position is valid
boolean test(int y, int x) {
    if (x < 0 || x >= fieldx) return false;
    if (y < 0) return true;  // Allow spawning above field
    if (y >= fieldy) return false;
    return field[y][x] == 0;
}

// Spawn new piece
void newPiece() {
    currentPiece.x = fieldx / 2;
    currentPiece.y = -2;  // Start above visible field
    currentPiece.gems[0] = randomGem();
    currentPiece.gems[1] = randomGem();
    currentPiece.gems[2] = randomGem();
    currentPiece.active = true;
    
    // Check game over (spawn blocked)
    if (!test(0, currentPiece.x) || !test(1, currentPiece.x) || !test(2, currentPiece.x)) {
        gameOver = true;
    }
}

// Cycle gems (instead of rotation)
void cycleGems() {
    byte temp = currentPiece.gems[0];
    currentPiece.gems[0] = currentPiece.gems[1];
    currentPiece.gems[1] = currentPiece.gems[2];
    currentPiece.gems[2] = temp;
}

// Move piece down
void moveDown() {
    if (!currentPiece.active) return;
    
    bool canMove = true;
    for (int i = 0; i < 3; i++) {
        if (!test(currentPiece.y + i + 1, currentPiece.x)) {
            canMove = false;
            break;
        }
    }
    
    if (canMove) {
        currentPiece.y++;
        lockDelayActive = false;  // Reset lock delay when piece moves
    } else {
        // Start lock delay if not already active
        if (!lockDelayActive) {
            lockDelayActive = true;
            lockDelayStart = millis();
        }
        
        // Check if lock delay has expired (400ms)
        if (millis() - lockDelayStart >= LOCK_DELAY_MS) {
            // Place piece
            for (int i = 0; i < 3; i++) {
                int y = currentPiece.y + i;
                if (y >= 0 && y < fieldy) {
                    field[y][currentPiece.x] = currentPiece.gems[i];
                }
            }
            currentPiece.active = false;
            lockDelayActive = false;
        }
    }
}

// Move piece left
void moveLeft() {
    if (!currentPiece.active) return;
    
    bool canMove = true;
    for (int i = 0; i < 3; i++) {
        if (!test(currentPiece.y + i, currentPiece.x - 1)) {
            canMove = false;
            break;
        }
    }
    
    if (canMove) {
        currentPiece.x--;
        lockDelayActive = false;  // Reset lock delay on horizontal move
    }
}

// Move piece right
void moveRight() {
    if (!currentPiece.active) return;
    
    bool canMove = true;
    for (int i = 0; i < 3; i++) {
        if (!test(currentPiece.y + i, currentPiece.x + 1)) {
            canMove = false;
            break;
        }
    }
    
    if (canMove) {
        currentPiece.x++;
        lockDelayActive = false;  // Reset lock delay on horizontal move
    }
}

// Hard drop
void plummet() {
    if (!currentPiece.active) return;
    
    while (currentPiece.active) {
        moveDown();
    }
}

// Calculate ghost piece position (where piece will land)
int calculateGhostY() {
    if (!currentPiece.active) return currentPiece.y;
    
    int ghostY = currentPiece.y;
    
    // Keep moving down until we hit something
    while (true) {
        bool canMove = true;
        for (int i = 0; i < 3; i++) {
            if (!test(ghostY + i + 1, currentPiece.x)) {
                canMove = false;
                break;
            }
        }
        
        if (canMove) {
            ghostY++;
        } else {
            break;
        }
    }
    
    return ghostY;
}

// Draw ghost piece (outline showing where piece will land)
void drawGhostPiece() {
    if (!currentPiece.active) return;
    
    int ghostY = calculateGhostY();
    
    // Don't draw ghost if it's at same position as current piece
    if (ghostY == currentPiece.y) return;
    
    // Draw ghost pieces as outlines (just borders)
    for (int i = 0; i < 3; i++) {
        int y = ghostY + i;
        if (y >= 0 && y < fieldy) {
            int screenX = offsetx + (currentPiece.x * blockSize);
            int screenY = offsety + (y * blockSize);
            
            // Draw outline in same color as gem
            Disp.drawRect(screenX, screenY, blockSize - 1, blockSize - 1, gemColors[currentPiece.gems[i]]);
            
            int centerX = screenX + (blockSize / 2);
            int centerY = screenY + (blockSize / 2);
            
            // Add white dot to white ghost gems (gem == 4)
            if (currentPiece.gems[i] == 4) {
                Disp.fillCircle(centerX, centerY, 1, WHITE);
            }
            
            // Add white X to green ghost gems (gem == 5)
            if (currentPiece.gems[i] == 5) {
                Disp.drawLine(screenX + 3, screenY + 3, screenX + blockSize - 4, screenY + blockSize - 4, WHITE);
                Disp.drawLine(screenX + blockSize - 4, screenY + 3, screenX + 3, screenY + blockSize - 4, WHITE);
            }
        }
    }
}

// ============================================================================
// MATCH-3 LOGIC
// ============================================================================

// Find and remove matches
bool findAndRemoveMatches() {
    bool foundMatches = false;
    bool toRemove[fieldy][fieldx] = {false};
    
    // Check horizontal
    for (int y = 0; y < fieldy; y++) {
        for (int x = 0; x < fieldx - 2; x++) {
            byte color = field[y][x];
            if (color == 0) continue;
            
            int len = 1;
            while (x + len < fieldx && field[y][x + len] == color) len++;
            
            if (len >= 3) {
                for (int i = 0; i < len; i++) toRemove[y][x + i] = true;
                foundMatches = true;
            }
        }
    }
    
    // Check vertical
    for (int x = 0; x < fieldx; x++) {
        for (int y = 0; y < fieldy - 2; y++) {
            byte color = field[y][x];
            if (color == 0) continue;
            
            int len = 1;
            while (y + len < fieldy && field[y + len][x] == color) len++;
            
            if (len >= 3) {
                for (int i = 0; i < len; i++) toRemove[y + i][x] = true;
                foundMatches = true;
            }
        }
    }
    
    // Check diagonal (\)
    for (int y = 0; y < fieldy - 2; y++) {
        for (int x = 0; x < fieldx - 2; x++) {
            byte color = field[y][x];
            if (color == 0) continue;
            
            int len = 1;
            while (x + len < fieldx && y + len < fieldy && 
                   field[y + len][x + len] == color) len++;
            
            if (len >= 3) {
                for (int i = 0; i < len; i++) toRemove[y + i][x + i] = true;
                foundMatches = true;
            }
        }
    }
    
    // Check diagonal (/)
    for (int y = 0; y < fieldy - 2; y++) {
        for (int x = 2; x < fieldx; x++) {
            byte color = field[y][x];
            if (color == 0) continue;
            
            int len = 1;
            while (x - len >= 0 && y + len < fieldy && 
                   field[y + len][x - len] == color) len++;
            
            if (len >= 3) {
                for (int i = 0; i < len; i++) toRemove[y + i][x - i] = true;
                foundMatches = true;
            }
        }
    }
    
    // Remove marked gems
    if (foundMatches) {
        for (int y = 0; y < fieldy; y++) {
            for (int x = 0; x < fieldx; x++) {
                if (toRemove[y][x]) field[y][x] = 0;
            }
        }
    }
    
    return foundMatches;
}

// Apply gravity
void applyGravity() {
    for (int x = 0; x < fieldx; x++) {
        int writePos = fieldy - 1;
        for (int y = fieldy - 1; y >= 0; y--) {
            if (field[y][x] != 0) {
                if (y != writePos) {
                    field[writePos][x] = field[y][x];
                    field[y][x] = 0;
                }
                writePos--;
            }
        }
    }
}

// Process all matches and chains
void processMatches() {
    int chainCount = 0;
    bool foundMatches;
    
    do {
        foundMatches = findAndRemoveMatches();
        if (foundMatches) {
            chainCount++;
            score += 100 * chainCount;
            
            showField(false);
            delay(200);
            
            applyGravity();
            
            showField(false);
            delay(150);
        }
    } while (foundMatches);
    
    // Level up every 1000 points (3% speed increase per level)
    if (score > level * 1000 && level < 20) {
        level++;
        speed = max(100L, (long)(500 * pow(0.97, level - 1)));  // 3% faster each level
    }
}

// ============================================================================
// INPUT & GAME LOOP
// ============================================================================

int check_Btn() {
    return Joystick.getButtonStatus() == 0 ? 1 : 0;
}

int check_Left() {
    int val = Joystick.getADCValue(POS_X);
    return (val < 1350) ? 1 : 0;  // Left is low value
}

int check_Right() {
    int val = Joystick.getADCValue(POS_X);
    return (val > 2950) ? 1 : 0;  // Right is high value
}

int check_Down() {
    int val = Joystick.getADCValue(POS_Y);
    return (val < 1600) ? 1 : 0;  // Only trigger if strongly down
}

int check_Up() {
    int val = Joystick.getADCValue(POS_Y);
    return (val > 2950) ? 1 : 0;  // Only trigger if strongly up
}

void wait() {
    static long unsigned leftHold = 0;
    static long unsigned rightHold = 0;
    static long unsigned downHold = 0;
    static long unsigned btnHold = 0;
    
    M5.update();
    
    if (gameOver || gamePaused) return;
    
    // Cycle gems (M5 button, joystick up, or joystick button)
    if ((M5.BtnA.wasPressed() || check_Up() || check_Btn()) && currentPiece.active) {
        cycleGems();
        showField(true);
    }
    
    // Hard drop (BtnB)
    if (M5.BtnB.wasPressed() && currentPiece.active) {
        plummet();
        processMatches();
        newPiece();
        showField(true);
        scoreBoardWrite(score);
    }
    
    // Move left
    if (check_Left() && millis() - leftHold > 150) {
        moveLeft();
        leftHold = millis();
        showField(true);
    }
    
    // Move right
    if (check_Right() && millis() - rightHold > 150) {
        moveRight();
        rightHold = millis();
        showField(true);
    }
    
    // Soft drop
    if (check_Down() && millis() - downHold > 100) {
        moveDown();
        downHold = millis();
        showField(true);
        if (!currentPiece.active) {
            processMatches();
            newPiece();
            scoreBoardWrite(score);
        }
    }
    
    // Auto-fall
    if (millis() - timez > speed && currentPiece.active) {
        moveDown();
        timez = millis();
        showField(true);
        
        if (!currentPiece.active) {
            processMatches();
            newPiece();
            scoreBoardWrite(score);
        }
    }
}

void reset() {
    gameOver = false;
    gamePaused = false;
    score = 0;
    level = 1;
    speed = 500;
    initField();
    newPiece();
    Disp.fillScreen(BLACK);
    drawGameBorder();
    showField(true);
    scoreBoardWrite(score);
}

// ============================================================================
// SETUP & MAIN LOOP
// ============================================================================

void setup() {
    M5.begin();
    
    Disp.setRotation(0);  // Portrait mode like Tetris
    Disp.fillScreen(BLACK);
    
    // Initialize JoyC (same as Tetris)
    while (!(Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL))) {
        Disp.println("JoyC not found");
        delay(1000);
    }
    
    // Initialize game
    randomSeed(esp_random());
    reset();
    
    timez = millis();
}

void loop() {
    M5.update();
    
    if (gameOver) {
        Disp.setTextSize(2);
        Disp.setTextColor(RED);
        Disp.setCursor(20, 100);
        Disp.print("GAME OVER");
        
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
            reset();
        }
    } else {
        wait();
    }
    
    delay(10);
}

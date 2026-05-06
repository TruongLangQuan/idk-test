#include <M5Unified.h>

static constexpr int kCols = 40;
static constexpr int kRows = 16;
static constexpr int kCellW = 6;
static constexpr int kCellH = 8;

struct Cell {
    uint8_t ch;
    uint16_t fg;
    uint16_t bg;
};

Cell g_grid[kRows][kCols];
M5Canvas canvas(&M5.Display);

void setCell(int col, int row, uint8_t ch, uint16_t fg, uint16_t bg = TFT_BLACK) {
    if (col >= 0 && col < kCols && row >= 0 && row < kRows) {
        g_grid[row][col] = {ch, fg, bg};
    }
}

void printStr(int col, int row, const char* str, uint16_t fg, uint16_t bg = TFT_BLACK) {
    while (*str) setCell(col++, row, *str++, fg, bg);
}

void render() {
    canvas.startWrite();
    canvas.fillSprite(TFT_BLACK);
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (g_grid[r][c].ch != ' ' || g_grid[r][c].bg != TFT_BLACK) {
                canvas.setTextColor(g_grid[r][c].fg, g_grid[r][c].bg);
                canvas.drawChar(g_grid[r][c].ch, c * kCellW, r * kCellH);
            }
        }
    }
    canvas.endWrite();
    canvas.pushSprite(0, 0);
}

void clear() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            g_grid[r][c] = {' ', TFT_WHITE, TFT_BLACK};
        }
    }
}

uint32_t timer = 0;
int step = 0;
int layer_activations[3][8]; // 3 layers, 8 nodes

void setup() {
    pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);
    delay(100);

    auto cfg = M5.config();
    cfg.internal_mic = false; // Disable MIC (G0 conflicts with SD SCK)
    cfg.internal_spk = false; // Disable SPK
    M5.begin(cfg);
    M5.Display.setRotation(3);
    M5.Display.setBrightness(128);

    canvas.setPsram(true);
    canvas.setColorDepth(8);
    canvas.createSprite(240, 135);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
}

void loop() {
    M5.update();
    uint32_t now = millis();
    
    if (now - timer > 100) {
        timer = now;
        clear();
        
        step++;
        
        printStr(2, 0, "BitNet 1-bit LLM Sim", TFT_YELLOW);
        printStr(2, 1, "(-1, 0, 1) Quantization", TFT_DARKGREY);
        
        // Draw input tokens
        printStr(1, 4, "INP", TFT_CYAN);
        for(int i=0; i<8; i++) {
            char val = (esp_random() % 2 == 0) ? '1' : '0';
            setCell(2, 6+i, val, TFT_CYAN);
        }
        
        // Draw 3 BitLinear layers
        int x_offsets[] = {8, 20, 32};
        for (int l = 0; l < 3; l++) {
            printStr(x_offsets[l]-1, 4, "BitLin", TFT_ORANGE);
            for (int r = 0; r < 8; r++) {
                for (int c = 0; c < 4; c++) {
                    int w = esp_random() % 3; // 0, 1, 2
                    char ch = '0';
                    uint16_t color = TFT_DARKGREY;
                    if (w == 0) { ch = '-'; color = TFT_RED; }
                    if (w == 2) { ch = '+'; color = TFT_GREEN; }
                    
                    // Flash active weights
                    if (esp_random() % 10 == 0) {
                        setCell(x_offsets[l] + c, 6+r, ch, TFT_WHITE, color);
                    } else {
                        setCell(x_offsets[l] + c, 6+r, ch, color);
                    }
                }
            }
            
            // Draw flowing data between layers
            if (l < 2) {
                for (int i=0; i<8; i++) {
                    if ((step + i) % 4 == 0) {
                        setCell(x_offsets[l] + 5, 6+i, '>', TFT_MAGENTA);
                    }
                }
            }
        }
        
        render();
    }
}

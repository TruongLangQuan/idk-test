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
int g_mode = 0;
int g_complexity = 0;
String g_generated = "";

const char* modeName() {
    if (g_mode == 0) return "BitNet";
    if (g_mode == 1) return "Text";
    if (g_mode == 2) return "Math";
    return "Equation";
}

void wrapPrint(int col, int row, const String& text, uint16_t fg) {
    int c = col;
    int r = row;
    for (int i = 0; i < text.length() && r < kRows; ++i) {
        char ch = text[i];
        if (ch == '\n' || c >= kCols - 1) {
            c = col;
            r++;
            if (ch == '\n') continue;
        }
        setCell(c++, r, ch, fg);
    }
}

String genText() {
    static const char* subjects[] = {"robot", "sensor", "student", "model", "screen"};
    static const char* verbs[] = {"learns", "draws", "solves", "predicts", "builds"};
    static const char* objects[] = {"patterns", "signals", "maps", "words", "numbers"};
    int a = esp_random() % 5;
    int b = esp_random() % 5;
    int c = esp_random() % 5;
    if (g_complexity == 0) return String(subjects[a]) + " " + verbs[b] + ".";
    if (g_complexity == 1) return String("The ") + subjects[a] + " " + verbs[b] + " " + objects[c] + " using tiny weights.";
    return String("Because the ") + subjects[a] + " " + verbs[b] + " noisy " + objects[c] + ", it compresses context, compares tokens, then answers step by step.";
}

String genMath() {
    int a = 2 + (esp_random() % (8 + g_complexity * 20));
    int b = 1 + (esp_random() % (7 + g_complexity * 15));
    int c = 1 + (esp_random() % (5 + g_complexity * 9));
    if (g_complexity == 0) return String(a) + " + " + String(b) + " = " + String(a + b);
    if (g_complexity == 1) return String(a) + " * " + String(b) + " - " + String(c) + " = " + String(a * b - c);
    return String("(") + a + " + " + b + ") * " + c + " - " + a + "^2 = " + String((a + b) * c - a * a);
}

String genEquation() {
    int a = 1 + (esp_random() % (4 + g_complexity * 3));
    int x = 1 + (esp_random() % (8 + g_complexity * 8));
    int b = esp_random() % (10 + g_complexity * 20);
    int rhs = a * x + b;
    if (g_complexity == 0) return String("x + ") + b + " = " + rhs + "\nx = " + String(rhs - b);
    if (g_complexity == 1) return String(a) + "x + " + b + " = " + rhs + "\nx = " + x;
    int c = 1 + (esp_random() % 5);
    return String(a) + "(x + " + c + ") + " + b + " = " + (a * (x + c) + b) + "\nx = " + x;
}

void generateOutput() {
    if (g_mode == 1) g_generated = genText();
    else if (g_mode == 2) g_generated = genMath();
    else if (g_mode == 3) g_generated = genEquation();
}

void setup() {
    pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);
    delay(100);

    auto cfg = M5.config();
    cfg.internal_mic = false; // Disable MIC (G0 conflicts with SD SCK)
    cfg.internal_spk = false; // Disable SPK
    M5.begin(cfg);
#if defined(STICKS3)
    M5.Display.setRotation(1);
#else
    M5.Display.setRotation(3);
#endif
    M5.Display.setBrightness(128);

    canvas.setPsram(true);
    canvas.setColorDepth(8);
    canvas.createSprite(240, 135);
    canvas.setFont(&fonts::Font0);
    canvas.setTextSize(1);
}

void loop() {
    M5.update();
    if (M5.BtnA.wasPressed()) {
        g_mode = (g_mode + 1) % 4;
        generateOutput();
    }
    if (M5.BtnB.wasPressed()) {
        g_complexity = (g_complexity + 1) % 3;
        generateOutput();
    }
    if (M5.BtnPWR.wasPressed()) generateOutput();
    uint32_t now = millis();
    
    if (now - timer > 100) {
        timer = now;
        clear();
        
        step++;
        
        char title[38];
        snprintf(title, sizeof(title), "%s gen  C:%d", modeName(), g_complexity + 1);
        printStr(2, 0, title, TFT_YELLOW);
        printStr(2, 1, "A:mode B:level PWR:gen", TFT_DARKGREY);

        if (g_mode != 0) {
            if (g_generated.isEmpty()) generateOutput();
            wrapPrint(2, 4, g_generated, g_mode == 1 ? TFT_CYAN : (g_mode == 2 ? TFT_GREEN : TFT_ORANGE));
            render();
            return;
        }
        
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

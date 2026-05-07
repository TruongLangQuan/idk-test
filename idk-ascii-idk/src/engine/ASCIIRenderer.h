#pragma once

#include <M5Unified.h>
#include <stdint.h>

namespace idk {

struct Cell {
    uint8_t ch;
    uint16_t fg;
    uint16_t bg;

    bool operator==(const Cell& other) const {
        return ch == other.ch && fg == other.fg && bg == other.bg;
    }
    bool operator!=(const Cell& other) const {
        return !(*this == other);
    }
};

class ASCIIRenderer {
public:
    static constexpr int kCols = 40;
    static constexpr int kRows = 16;
    static constexpr int kCellW = 6;
    static constexpr int kCellH = 8;

    ASCIIRenderer();
    
    bool begin();
    void clear(uint8_t ch = ' ', uint16_t fg = TFT_WHITE, uint16_t bg = TFT_BLACK);
    void setCell(int col, int row, uint8_t ch, uint16_t fg, uint16_t bg = 0);
    void print(int col, int row, const char* str, uint16_t fg, uint16_t bg = 0);
    void flush();

private:
    Cell _grid[kRows][kCols];
    Cell _prevGrid[kRows][kCols];
    M5Canvas _canvas;
    bool _initialized = false;
    bool _forceFullRedraw = true;
};

} // namespace idk

#pragma once
#include <M5Unified.h>
#include <vector>
#include <string>

namespace idk {

class ASCIIRenderer {
public:
    static constexpr int kCols = 48; // 240 / 5
    static constexpr int kRows = 22; // 135 / 6
    static constexpr int kCellW = 5;
    static constexpr int kCellH = 6;

    struct Cell {
        uint16_t glyph;
        uint16_t fg;
        uint16_t bg;
    };

    ASCIIRenderer() {}

    void begin() {
        _sprite.setPsram(true);
        _sprite.setColorDepth(8); 
        _sprite.createSprite(240, 135);
        _sprite.setFont(&fonts::Font0); 
        _sprite.setTextSize(1);
    }

    void clear(uint16_t bg = 0) {
        _sprite.fillSprite(bg);
    }

    void setCell(int col, int row, uint16_t glyph, uint16_t fg, uint16_t bg = 0) {
        if (col < 0 || col >= kCols || row < 0 || row >= kRows) return;
        _grid[row][col] = {glyph, fg, bg};
    }

    void render() {
        _sprite.startWrite();
        for (int r = 0; r < kRows; ++r) {
            for (int c = 0; c < kCols; ++c) {
                const auto& cell = _grid[r][c];
                _sprite.setTextColor(cell.fg, cell.bg);
                _sprite.drawChar(cell.glyph, c * kCellW, r * kCellH);
            }
        }
        _sprite.endWrite();
        _sprite.pushSprite(0, 0);
    }

    void drawHeader(const char* styleName, float x, float z, int styleIdx) {
        _sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        _sprite.setCursor(2, 2);
        _sprite.printf("[%d/%d] %s", styleIdx + 1, 68, styleName);
        _sprite.setCursor(180, 2);
        _sprite.printf("%.1f, %.1f", x, z);
    }

    M5Canvas& getSprite() { return _sprite; }

private:
    M5Canvas _sprite{&M5.Display};
    Cell _grid[kRows][kCols];
};

} // namespace idk

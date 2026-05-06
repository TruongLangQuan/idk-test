#include "ASCIIRenderer.h"

namespace idk {

ASCIIRenderer::ASCIIRenderer() : _canvas(&M5.Display) {}

bool ASCIIRenderer::begin() {
    if (_initialized) return true;

    // Allocate canvas in PSRAM for 240x135 resolution
    // 240 * 135 * 2 bytes (RGB565) = 64.8 KB
    _canvas.setPsram(true);
    if (!_canvas.createSprite(240, 135)) {
        return false;
    }

    _canvas.setFont(&fonts::Font0); // 6x8 default font
    _canvas.setTextSize(1);
    
    clear();
    _initialized = true;
    return true;
}

void ASCIIRenderer::clear(uint8_t ch, uint16_t fg, uint16_t bg) {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            _grid[r][c] = {ch, fg, bg};
        }
    }
}

void ASCIIRenderer::setCell(int col, int row, uint8_t ch, uint16_t fg, uint16_t bg) {
    if (col >= 0 && col < kCols && row >= 0 && row < kRows) {
        _grid[row][col] = {ch, fg, bg};
    }
}

void ASCIIRenderer::print(int col, int row, const char* str, uint16_t fg, uint16_t bg) {
    while (*str && col < kCols) {
        setCell(col++, row, (uint8_t)*str++, fg, bg);
    }
}

void ASCIIRenderer::flush() {
    if (!_initialized) return;

    // Optimization: Only redraw cells that changed in the grid
    // This is much faster than clearing the entire sprite and redrawing every char
    // especially for static UI elements.
    _canvas.startWrite();
    
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (_grid[r][c] != _prevGrid[r][c]) {
                _canvas.fillRect(c * kCellW, r * kCellH, kCellW, kCellH, _grid[r][c].bg);
                _canvas.setTextColor(_grid[r][c].fg, _grid[r][c].bg);
                _canvas.drawChar(_grid[r][c].ch, c * kCellW, r * kCellH);
                _prevGrid[r][c] = _grid[r][c];
            }
        }
    }
    
    _canvas.endWrite();
    
    // Push the entire PSRAM buffer to the display in one go (DMA)
    _canvas.pushSprite(0, 0);
}

} // namespace idk

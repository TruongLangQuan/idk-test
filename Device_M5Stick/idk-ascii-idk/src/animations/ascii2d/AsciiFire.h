#pragma once

#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class AsciiFire : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        memset(_heat, 0, sizeof(_heat));
        _palette = {
            M5.Display.color565(0, 0, 0),       // Black
            M5.Display.color565(128, 0, 0),     // Dark Red
            M5.Display.color565(255, 0, 0),     // Red
            M5.Display.color565(255, 128, 0),   // Orange
            M5.Display.color565(255, 255, 0),   // Yellow
            M5.Display.color565(255, 255, 255)  // White
        };
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        // Seed bottom row
        for (int c = 0; c < ASCIIRenderer::kCols; ++c) {
            _heat[ASCIIRenderer::kRows - 1][c] = esp_random() % 256;
        }

        // Cool and propagate up
        for (int row = 0; row < ASCIIRenderer::kRows - 1; ++row) {
            for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
                int sum = 0;
                sum += _heat[row + 1][(col - 1 + ASCIIRenderer::kCols) % ASCIIRenderer::kCols];
                sum += _heat[row + 1][col];
                sum += _heat[row + 1][(col + 1) % ASCIIRenderer::kCols];
                sum += _heat[(row + 2) % ASCIIRenderer::kRows][col];

                int h = (sum / 4) - (esp_random() % 10);
                if (h < 0) h = 0;
                _heat[row][col] = h;
                
                // Render
                char ch = _chars[h * (sizeof(_chars)-1) / 256];
                uint16_t color = _palette[h * (_palette.size()-1) / 256];
                r.setCell(col, row, ch, color, TFT_BLACK);
            }
        }
    }

    const char* name() const override { return "ASCII Fire"; }

private:
    uint8_t _heat[ASCIIRenderer::kRows][ASCIIRenderer::kCols];
    const char* _chars = " .:-=+*#%@";
    std::vector<uint16_t> _palette;
};

} // namespace idk

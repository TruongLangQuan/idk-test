#pragma once

#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class MatrixRain : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear(' ', TFT_WHITE, TFT_BLACK);
        _drops.assign(ASCIIRenderer::kCols, {-1, 0.0f});
        for (auto& d : _drops) {
            d.y = -(float)(esp_random() % ASCIIRenderer::kRows);
            d.speed = 0.5f + (float)(esp_random() % 100) / 100.0f;
        }
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        // Fade existing characters
        for (int row = 0; row < ASCIIRenderer::kRows; ++row) {
            for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
                // We don't have a "get" in the renderer easily, so we just redraw 
                // or use a more efficient trail method.
                // For simplicity, we just clear and redraw the drops each frame 
                // but for true matrix rain, we want trails.
            }
        }
        
        r.clear();

        for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
            auto& d = _drops[col];
            d.y += d.speed * (dtMs / 33.3f);
            
            if (d.y >= ASCIIRenderer::kRows) {
                d.y = -1;
                d.speed = 0.5f + (float)(esp_random() % 100) / 100.0f;
            }

            if (d.y >= 0) {
                int row = (int)d.y;
                // Bright head
                r.setCell(col, row, (char)(33 + esp_random() % 94), TFT_WHITE, TFT_BLACK);
                // Green tail
                for (int i = 1; i < 8; ++i) {
                    if (row - i >= 0) {
                        uint8_t green = 255 - (i * 30);
                        r.setCell(col, row - i, (char)(33 + esp_random() % 94), 
                                  M5.Display.color565(0, green, 0), TFT_BLACK);
                    }
                }
            }
        }
    }

    const char* name() const override { return "Matrix Rain"; }

private:
    struct Drop {
        float y;
        float speed;
    };
    std::vector<Drop> _drops;
};

} // namespace idk

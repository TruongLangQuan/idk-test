#pragma once
#include "../../engine/IAnimation.h"

namespace idk {

class FluidWave : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        memset(_height, 0, sizeof(_height));
        memset(_vel, 0, sizeof(_vel));
        _timer = 0;
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        _timer += dtMs;
        if (_timer > 500.0f) {
            _timer = 0;
            // Drop water at random position
            int cx = esp_random() % ASCIIRenderer::kCols;
            int cy = esp_random() % ASCIIRenderer::kRows;
            _height[cy][cx] = 10.0f;
        }

        // Wave equation
        float c2 = 0.5f; // Wave speed squared
        float damping = 0.95f;
        float nextHeight[ASCIIRenderer::kRows][ASCIIRenderer::kCols];

        for (int row = 0; row < ASCIIRenderer::kRows; ++row) {
            for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
                float laplacian = 0;
                if (row > 0) laplacian += _height[row-1][col];
                if (row < ASCIIRenderer::kRows-1) laplacian += _height[row+1][col];
                if (col > 0) laplacian += _height[row][col-1];
                if (col < ASCIIRenderer::kCols-1) laplacian += _height[row][col+1];
                
                laplacian -= 4.0f * _height[row][col];

                _vel[row][col] += c2 * laplacian * (dtMs / 33.3f);
                _vel[row][col] *= damping;
                nextHeight[row][col] = _height[row][col] + _vel[row][col] * (dtMs / 33.3f);
            }
        }

        // Render
        r.clear();
        for (int row = 0; row < ASCIIRenderer::kRows; ++row) {
            for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
                _height[row][col] = nextHeight[row][col];
                float h = _height[row][col];
                
                char ch = ' ';
                uint16_t color = TFT_BLACK;
                
                if (h > 1.0f) { ch = '#'; color = TFT_WHITE; }
                else if (h > 0.5f) { ch = '*'; color = TFT_CYAN; }
                else if (h > 0.1f) { ch = '~'; color = TFT_BLUE; }
                else if (h < -0.1f) { ch = '.'; color = TFT_NAVY; }

                if (ch != ' ') {
                    r.setCell(col, row, ch, color, TFT_BLACK);
                }
            }
        }
    }

    const char* name() const override { return "Droplet Splash"; }

private:
    float _height[ASCIIRenderer::kRows][ASCIIRenderer::kCols];
    float _vel[ASCIIRenderer::kRows][ASCIIRenderer::kCols];
    float _timer;
};

} // namespace idk

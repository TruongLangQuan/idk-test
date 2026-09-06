#pragma once
#include "../../engine/IAnimation.h"

namespace idk {

class Mandelbrot : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _currentRow = 0;
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        if (_currentRow >= ASCIIRenderer::kRows) {
            // Done rendering
            return; 
        }

        // Render one row per frame to keep 30 FPS responsive
        float xMin = -2.5f, xMax = 1.0f;
        float yMin = -1.0f, yMax = 1.0f;

        for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
            float cx = xMin + col * (xMax - xMin) / ASCIIRenderer::kCols;
            float cy = yMin + _currentRow * (yMax - yMin) / ASCIIRenderer::kRows;

            float zx = 0, zy = 0;
            int iter = 0;
            while (zx*zx + zy*zy < 4.0f && iter < 32) {
                float zx_new = zx*zx - zy*zy + cx;
                zy = 2.0f * zx * zy + cy;
                zx = zx_new;
                iter++;
            }

            char ch = _chars[iter * 9 / 32];
            uint16_t color = M5.Display.color565(iter * 8, iter * 4, 255 - iter*4);
            if (iter == 32) color = TFT_BLACK; // inside set

            r.setCell(col, _currentRow, ch, color, TFT_BLACK);
        }
        
        _currentRow++;
    }

    const char* name() const override { return "Mandelbrot"; }

private:
    int _currentRow = 0;
    const char* _chars = " .:-=+*#%@";
};

} // namespace idk

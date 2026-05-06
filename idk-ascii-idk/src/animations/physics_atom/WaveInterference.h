#pragma once
#include "../../engine/IAnimation.h"
#include <cmath>

namespace idk {

class WaveInterference : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _time = 0;
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        _time += dtMs * 0.005f;
        
        float cx1 = ASCIIRenderer::kCols / 3.0f;
        float cy1 = ASCIIRenderer::kRows / 2.0f;
        
        float cx2 = ASCIIRenderer::kCols * 2.0f / 3.0f;
        float cy2 = ASCIIRenderer::kRows / 2.0f;

        for (int row = 0; row < ASCIIRenderer::kRows; ++row) {
            for (int col = 0; col < ASCIIRenderer::kCols; ++col) {
                // Distance to source 1 (scaled for char aspect ratio)
                float dx1 = (col - cx1) * 0.5f;
                float dy1 = (row - cy1);
                float d1 = std::sqrt(dx1*dx1 + dy1*dy1);

                // Distance to source 2
                float dx2 = (col - cx2) * 0.5f;
                float dy2 = (row - cy2);
                float d2 = std::sqrt(dx2*dx2 + dy2*dy2);

                float wave1 = std::sin(d1 - _time);
                float wave2 = std::sin(d2 - _time);
                
                float total = wave1 + wave2; // Range: -2 to 2
                
                int charIdx = (int)((total + 2.0f) * 2.5f); // Map 0-4 to 0-9
                if (charIdx < 0) charIdx = 0;
                if (charIdx > 9) charIdx = 9;

                uint16_t color = M5.Display.color565(100 + charIdx*15, 50, 255 - charIdx*15);
                r.setCell(col, row, _chars[charIdx], color, TFT_BLACK);
            }
        }
    }

    const char* name() const override { return "Wave Interference"; }

private:
    float _time;
    const char* _chars = " .:-=+*#%@";
};

} // namespace idk

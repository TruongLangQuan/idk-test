#pragma once
#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class Starfield : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _stars.assign(40, {0,0,0});
        for(auto& s : _stars) resetStar(s);
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        
        float cx = ASCIIRenderer::kCols / 2.0f;
        float cy = ASCIIRenderer::kRows / 2.0f;

        for (auto& s : _stars) {
            s.z -= 2.0f * (dtMs / 33.3f); // Move towards viewer
            
            if (s.z <= 0) {
                resetStar(s);
            }

            // Project 3D to 2D
            float pX = cx + (s.x / s.z) * 10.0f; // FOV approx
            float pY = cy + (s.y / s.z) * 10.0f;

            if (pX < 0 || pX >= ASCIIRenderer::kCols || pY < 0 || pY >= ASCIIRenderer::kRows) {
                resetStar(s);
                continue;
            }

            char ch = '.';
            if (s.z < 10.0f) ch = '*';
            if (s.z < 5.0f) ch = '#';

            uint16_t c = (uint16_t)(255 * (1.0f - (s.z / 20.0f)));
            if (c > 255) c = 255;
            uint16_t color = M5.Display.color565(c, c, c);

            r.setCell((int)pX, (int)pY, ch, color, TFT_BLACK);
        }
    }

    const char* name() const override { return "Starfield Warp"; }

private:
    struct Star { float x, y, z; };
    std::vector<Star> _stars;

    void resetStar(Star& s) {
        s.x = ((esp_random() % 100) - 50) / 2.0f;
        s.y = ((esp_random() % 100) - 50) / 2.0f;
        s.z = 20.0f;
    }
};

} // namespace idk

#pragma once
#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class LorenzAttractor : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _x = 0.1f; _y = 0.0f; _z = 0.0f;
        _trail.clear();
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        
        // 5 integration steps per frame for smooth rendering
        float dt = 0.005f;
        for (int i = 0; i < 5; ++i) {
            float dx = 10.0f * (_y - _x);
            float dy = _x * (28.0f - _z) - _y;
            float dz = _x * _y - (8.0f / 3.0f) * _z;
            
            _x += dx * dt;
            _y += dy * dt;
            _z += dz * dt;
            
            // Map 3D to 2D
            int px = ASCIIRenderer::kCols / 2 + (int)(_x * 0.7f);
            int py = ASCIIRenderer::kRows - (int)(_z * 0.35f);
            
            _trail.push_back({px, py});
            if (_trail.size() > 100) {
                _trail.erase(_trail.begin());
            }
        }
        
        // Render trail
        for (size_t i = 0; i < _trail.size(); ++i) {
            int px = _trail[i].x;
            int py = _trail[i].y;
            
            if (px >= 0 && px < ASCIIRenderer::kCols && py >= 0 && py < ASCIIRenderer::kRows) {
                char ch = _chars[(i * 5) / _trail.size()];
                uint16_t color = M5.Display.color565(255, (i * 255) / _trail.size(), 50);
                r.setCell(px, py, ch, color, TFT_BLACK);
            }
        }
    }

    const char* name() const override { return "Lorenz Attractor"; }

private:
    float _x, _y, _z;
    struct Point { int x, y; };
    std::vector<Point> _trail;
    const char* _chars = " .-*#";
};

} // namespace idk

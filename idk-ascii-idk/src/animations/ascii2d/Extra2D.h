#pragma once
#include "../../engine/IAnimation.h"
#include <cmath>
#include <vector>

namespace idk {

// 3.2 Sine Wave Scroll
class SineWave : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _phase = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        _phase += 0.05f * (dtMs / 33.3f);
        for (int x = 0; x < ASCIIRenderer::kCols; ++x) {
            int y = ASCIIRenderer::kRows/2 + (int)(6.0f * std::sin(x * 0.2f + _phase));
            r.setCell(x, y, '~', TFT_CYAN, TFT_BLACK);
        }
    }
    const char* name() const override { return "Sine Wave"; }
private: float _phase;
};

// 3.4 Fireworks
class Fireworks : public IAnimation {
public:
    struct Particle { float x, y, vx, vy; int life; uint16_t color; };
    void init(ASCIIRenderer& r) override { r.clear(); _parts.clear(); }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        if (esp_random() % 20 == 0) {
            float cx = 10 + esp_random() % (ASCIIRenderer::kCols - 20);
            uint16_t col = M5.Display.color565(esp_random()%256, esp_random()%256, esp_random()%256);
            for(int i=0; i<15; ++i) {
                float ang = (i / 15.0f) * 6.28f;
                _parts.push_back({cx, (float)ASCIIRenderer::kRows/2, std::cos(ang)*0.5f, std::sin(ang)*0.5f, 20, col});
            }
        }
        for (auto it = _parts.begin(); it != _parts.end();) {
            it->x += it->vx; it->y += it->vy; it->life--;
            if (it->life <= 0) it = _parts.erase(it);
            else {
                r.setCell((int)it->x, (int)it->y, '.', it->color, TFT_BLACK);
                ++it;
            }
        }
    }
    const char* name() const override { return "Fireworks"; }
private: std::vector<Particle> _parts;
};

} // namespace idk

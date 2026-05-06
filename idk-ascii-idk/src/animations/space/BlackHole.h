#pragma once
#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class BlackHole : public IAnimation {
public:
    struct Particle { float ang, dist, speed; uint16_t col; };
    void init(ASCIIRenderer& r) override {
        _parts.clear();
        for(int i=0; i<60; ++i) reset(i);
    }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        int cx = ASCIIRenderer::kCols/2, cy = ASCIIRenderer::kRows/2;
        r.setCell(cx, cy, 'O', TFT_BLACK, 0x4208); // Event horizon

        for(int i=0; i<_parts.size(); ++i) {
            _parts[i].dist -= 0.1f;
            _parts[i].ang += _parts[i].speed;
            if(_parts[i].dist < 1.0f) reset(i);
            
            int px = cx + (int)(_parts[i].dist * std::cos(_parts[i].ang) * 1.5f);
            int py = cy + (int)(_parts[i].dist * std::sin(_parts[i].ang));
            r.setCell(px, py, '.', _parts[i].col, TFT_BLACK);
        }
    }
    const char* name() const override { return "Black Hole"; }
private:
    std::vector<Particle> _parts;
    void reset(int i) {
        if(i >= _parts.size()) _parts.push_back({});
        _parts[i] = {(float)(esp_random()%628)/100.0f, 15.0f + (esp_random()%10), 0.05f + (esp_random()%10)/100.0f, 
                     M5.Display.color565(255, 100 + esp_random()%155, 50)};
    }
};

} // namespace idk

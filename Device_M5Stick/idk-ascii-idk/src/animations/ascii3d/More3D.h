#pragma once
#include "../../engine/IAnimation.h"
#include <cmath>

namespace idk {

class InfiniteTunnel : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _time = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _time += 0.1f * (dtMs / 33.3f);
        int cx = r.kCols/2, cy = r.kRows/2;
        for(int row=0; row<r.kRows; ++row) {
            for(int col=0; col<r.kCols; ++col) {
                float dx = (col - cx) * 0.5f, dy = (row - cy);
                float dist = std::sqrt(dx*dx + dy*dy);
                float angle = std::atan2(dy, dx);
                
                int u = (int)(angle * 5.0f + _time) % 2;
                int v = (int)(10.0f / (dist + 0.1f) + _time) % 2;
                
                char ch = (u ^ v) ? '#' : '.';
                uint16_t colr = M5.Display.color565(0, 0, (int)(dist * 10));
                r.setCell(col, row, ch, colr, TFT_BLACK);
            }
        }
    }
    const char* name() const override { return "Infinite Tunnel"; }
private: float _time;
};

class DnaHelix : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _ang = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        _ang += 0.1f * (dtMs / 33.3f);
        for(int y=0; y<r.kRows; ++y) {
            float a = _ang + y * 0.3f;
            int x1 = r.kCols/2 + (int)(8.0f * std::sin(a) * 1.5f);
            int x2 = r.kCols/2 + (int)(8.0f * std::sin(a + 3.14f) * 1.5f);
            
            r.setCell(x1, y, 'O', TFT_CYAN);
            r.setCell(x2, y, 'O', TFT_MAGENTA);
            
            if(y % 3 == 0) {
                int start = std::min(x1, x2), end = std::max(x1, x2);
                for(int x=start+1; x<end; ++x) r.setCell(x, y, '-', TFT_WHITE);
            }
        }
    }
    const char* name() const override { return "DNA Helix"; }
private: float _ang;
};

} // namespace idk

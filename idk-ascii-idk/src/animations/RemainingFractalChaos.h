#pragma once
#include "../engine/IAnimation.h"
#include <cmath>

namespace idk {

// 7.2 Julia Set
class JuliaSet : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _t += 0.05f;
        float cx = 0.355f + 0.1f * sin(_t);
        float cy = 0.355f + 0.1f * cos(_t*1.3f);
        for(int y=0; y<r.kRows; ++y) {
            for(int x=0; x<r.kCols; ++x) {
                float zx = 1.5f * (x - r.kCols/2) / (r.kCols/2);
                float zy = 1.0f * (y - r.kRows/2) / (r.kRows/2);
                int i=0;
                while(zx*zx+zy*zy<4 && i<16) {
                    float tmp = zx*zx - zy*zy + cx;
                    zy = 2*zx*zy + cy; zx = tmp; i++;
                }
                r.setCell(x,y, " .:-=+*#%@"[i*9/16], M5.Display.color565(i*16, i*8, 255-i*16));
            }
        }
    }
    const char* name() const override { return "Julia Set"; }
private: float _t;
};

// 8.3 Bifurcation Diagram
class Bifurcation : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { r.clear(); _r=2.5f; }
    void update(ASCIIRenderer& r, float dtMs) override {
        if(_r > 4.0f) { _r=2.5f; r.clear(); }
        for(int step=0; step<2; ++step) {
            float x = 0.5f;
            for(int i=0; i<100; ++i) x = _r * x * (1.0f - x);
            for(int i=0; i<50; ++i) {
                x = _r * x * (1.0f - x);
                int py = (int)(r.kRows - x * r.kRows);
                int px = (int)((_r - 2.5f) / 1.5f * r.kCols);
                r.setCell(px, py, '.', TFT_GREEN);
            }
            _r += 0.01f;
        }
    }
    const char* name() const override { return "Bifurcation"; }
private: float _r;
};

} // namespace idk

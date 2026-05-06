#pragma once
#include "../../engine/IAnimation.h"
#include <cmath>

namespace idk {

class Lissajous : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        _t += 0.05f;
        float a = 3.0f, b = 2.0f, delta = _t;
        int cx = r.kCols/2, cy = r.kRows/2;
        for(float i=0; i<6.28f; i+=0.02f) {
            int x = cx + (int)(15.0f * std::sin(a * i + delta) * 1.2f);
            int y = cy + (int)(7.0f * std::sin(b * i));
            r.setCell(x, y, '*', M5.Display.color565(255, 255, 0));
        }
    }
    const char* name() const override { return "Lissajous"; }
private: float _t;
};

class VectorField : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _t += 0.05f;
        for(int y=0; y<r.kRows; y+=2) {
            for(int x=0; x<r.kCols; x+=2) {
                float vx = std::sin(y * 0.2f + _t);
                float vy = std::cos(x * 0.2f + _t);
                char ch = '.';
                if(std::abs(vx) > std::abs(vy)) ch = vx > 0 ? '>' : '<';
                else ch = vy > 0 ? 'v' : '^';
                r.setCell(x, y, ch, TFT_YELLOW);
            }
        }
    }
    const char* name() const override { return "Vector Field"; }
private: float _t;
};

} // namespace idk

#pragma once
#include "../../engine/IAnimation.h"
#include <cmath>

namespace idk {

class DoublePendulum : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        _th1 = 2.0f; _th2 = 1.0f; _w1 = 0; _w2 = 0;
    }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        float g = 1.0f, m1 = 1.0f, m2 = 1.0f, l1 = 6.0f, l2 = 6.0f;
        float dt = 0.05f;
        
        for(int i=0; i<4; ++i) {
            float num1 = -g * (2 * m1 + m2) * sin(_th1);
            float num2 = -m2 * g * sin(_th1 - 2 * _th2);
            float num3 = -2 * sin(_th1 - _th2) * m2;
            float num4 = _w2 * _w2 * l2 + _w1 * _w1 * l1 * cos(_th1 - _th2);
            float den = l1 * (2 * m1 + m2 - m2 * cos(2 * _th1 - 2 * _th2));
            float a1 = (num1 + num2 + num3 * num4) / den;

            num1 = 2 * sin(_th1 - _th2);
            num2 = (_w1 * _w1 * l1 * (m1 + m2));
            num3 = g * (m1 + m2) * cos(_th1);
            num4 = _w2 * _w2 * l2 * m2 * cos(_th1 - _th2);
            den = l2 * (2 * m1 + m2 - m2 * cos(2 * _th1 - 2 * _th2));
            float a2 = (num1 * (num2 + num3 + num4)) / den;

            _w1 += a1 * dt; _w2 += a2 * dt; _th1 += _w1 * dt; _th2 += _w2 * dt;
        }

        int cx = ASCIIRenderer::kCols/2, cy = 2;
        int x1 = cx + (int)(l1 * sin(_th1));
        int y1 = cy + (int)(l1 * cos(_th1));
        int x2 = x1 + (int)(l2 * sin(_th2));
        int y2 = y1 + (int)(l2 * cos(_th2));

        drawLine(r, cx, cy, x1, y1, 'o', TFT_YELLOW);
        drawLine(r, x1, y1, x2, y2, '@', TFT_RED);
    }
    const char* name() const override { return "Double Pendulum"; }
private:
    float _th1, _th2, _w1, _w2;
    void drawLine(ASCIIRenderer& r, int x0, int y0, int x1, int y1, char ch, uint16_t col) {
        int dx = std::abs(x1-x0), dy = std::abs(y1-y0);
        int sx = x0<x1 ? 1 : -1, sy = y0<y1 ? 1 : -1;
        int err = dx-dy;
        while(true) {
            r.setCell(x0, y0, ch, col);
            if(x0==x1 && y0==y1) break;
            int e2 = 2*err;
            if(e2 > -dy) { err -= dy; x0 += sx; }
            if(e2 < dx) { err += dx; y0 += sy; }
        }
    }
};

} // namespace idk

#pragma once
#include "../engine/IAnimation.h"
#include <cmath>
#include <vector>

namespace idk {

// 11.2 Galaxy Spiral
class GalaxySpiral : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear(); _t += 0.02f;
        int cx=r.kCols/2, cy=r.kRows/2;
        for(float a=0; a<12.56f; a+=0.1f) {
            float r_val = a * 1.2f;
            for(int arm=0; arm<2; ++arm) {
                float ang = a + _t + arm * 3.14f;
                int px = cx + (int)(r_val * cos(ang) * 1.5f);
                int py = cy + (int)(r_val * sin(ang));
                r.setCell(px, py, esp_random()%5==0?'*':'.', TFT_WHITE);
            }
        }
    }
    const char* name() const override { return "Spiral Galaxy"; }
private: float _t;
};

// 11.3 Pulsar
class Pulsar : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear(); _t += 0.1f;
        int cx=r.kCols/2, cy=r.kRows/2;
        r.setCell(cx, cy, '@', TFT_WHITE, 0x4208);
        for(float d=0; d<20.0f; d+=0.5f) {
            int px1 = cx + (int)(d * cos(_t) * 2.0f), py1 = cy + (int)(d * sin(_t));
            int px2 = cx - (int)(d * cos(_t) * 2.0f), py2 = cy - (int)(d * sin(_t));
            r.setCell(px1, py1, '#', TFT_CYAN); r.setCell(px2, py2, '#', TFT_CYAN);
        }
    }
    const char* name() const override { return "Pulsar"; }
private: float _t;
};

// 12.2 Spirograph
class Spirograph : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear(); _t += 0.05f;
        float R=8, rr=3, p=4;
        int cx=r.kCols/2, cy=r.kRows/2;
        for(float a=0; a<12.56f; a+=0.05f) {
            float x = (R-rr)*cos(a) + p*cos((R-rr)*a/rr);
            float y = (R-rr)*sin(a) - p*sin((R-rr)*a/rr);
            r.setCell(cx+(int)(x*1.5f), cy+(int)y, '.', TFT_MAGENTA);
        }
    }
    const char* name() const override { return "Spirograph"; }
private: float _t;
};

} // namespace idk

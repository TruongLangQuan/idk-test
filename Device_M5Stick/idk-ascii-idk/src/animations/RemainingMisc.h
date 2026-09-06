#pragma once
#include "../engine/IAnimation.h"
#include <vector>

namespace idk {

// 5.1 Meteor Shower
class MeteorShower : public IAnimation {
public:
    struct Met { float x,y,v; };
    void init(ASCIIRenderer& r) override { _m.clear(); }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        if(esp_random()%10==0) _m.push_back({(float)(esp_random()%r.kCols), 0, 0.5f+(esp_random()%100)/100.0f});
        for(auto it=_m.begin(); it!=_m.end();){
            it->x += it->v; it->y += it->v;
            if(it->y >= r.kRows) it=_m.erase(it);
            else {
                for(int i=0; i<4; ++i) r.setCell((int)it->x-i, (int)it->y-i, i==0?'*':'.', TFT_WHITE);
                ++it;
            }
        }
    }
    const char* name() const override { return "Meteor Shower"; }
private: std::vector<Met> _m;
};

// 6.7 Plasma
class PlasmaSim : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _t += 0.05f;
        for(int y=0; y<r.kRows; ++y) {
            for(int x=0; x<r.kCols; ++x) {
                float v = sin(x*0.1f + _t) + sin(y*0.1f + _t) + sin((x+y)*0.1f + _t) + sin(sqrt(x*x+y*y)*0.1f + _t);
                int charIdx = (int)((v+4.0f)*1.25f);
                if(charIdx<0) charIdx=0; if(charIdx>9) charIdx=9;
                r.setCell(x, y, " .:-=+*#%@"[charIdx], M5.Display.color565(charIdx*25, 0, 255-charIdx*25));
            }
        }
    }
    const char* name() const override { return "Plasma Simulation"; }
private: float _t;
};

// 6.8 Radioactive Decay
class RadioactiveDecay : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        for(int y=0; y<r.kRows; ++y) for(int x=0; x<r.kCols; ++x) _g[y][x] = 1;
        _count = r.kRows * r.kCols;
    }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        for(int y=0; y<r.kRows; ++y) {
            for(int x=0; x<r.kCols; ++x) {
                if(_g[y][x] && esp_random()%1000 < 5) { _g[y][x]=0; _count--; }
                r.setCell(x,y, _g[y][x] ? 'X' : '.', _g[y][x] ? TFT_GREEN : TFT_DARKGREY);
            }
        }
        if(_count == 0) init(r);
    }
    const char* name() const override { return "Radioactive Decay"; }
private: uint8_t _g[ASCIIRenderer::kRows][ASCIIRenderer::kCols]; int _count;
};

} // namespace idk

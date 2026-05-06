#pragma once
#include "../../engine/IAnimation.h"
#include <cmath>
#include <vector>

namespace idk {

// 4.3 Sphere
class Sphere3D : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _ang = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        _ang += 0.05f;
        int cx = r.kCols/2, cy = r.kRows/2;
        for(float lat=0; lat<3.14f; lat+=0.3f) {
            for(float lon=0; lon<6.28f; lon+=0.3f) {
                float x = sin(lat) * cos(lon);
                float y = sin(lat) * sin(lon);
                float z = cos(lat);
                
                float rx = x*cos(_ang) - z*sin(_ang);
                float rz = x*sin(_ang) + z*cos(_ang);
                
                float ooz = 1.0f / (rz + 3.0f);
                int px = cx + (int)(rx * ooz * 30.0f * 1.5f);
                int py = cy + (int)(y * ooz * 30.0f);
                r.setCell(px, py, 'o', TFT_WHITE);
            }
        }
    }
    const char* name() const override { return "Sphere"; }
private: float _ang;
};

// 4.6 Rotating Pyramid
class Pyramid3D : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _ang = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        _ang += 0.03f;
        float s=sin(_ang), c=cos(_ang);
        float v[5][3] = {{0,-1,0},{-1,1,-1},{1,1,-1},{1,1,1},{-1,1,1}};
        int e[8][2] = {{0,1},{0,2},{0,3},{0,4},{1,2},{2,3},{3,4},{4,1}};
        auto project = [&](int i){
            float rx = v[i][0]*c - v[i][2]*s;
            float rz = v[i][0]*s + v[i][2]*c;
            float ooz = 1.0f / (rz + 4.0f);
            return Point{(int)(r.kCols/2 + rx*ooz*50), (int)(r.kRows/2 + v[i][1]*ooz*30)};
        };
        for(int i=0; i<8; ++i) {
            Point p1=project(e[i][0]), p2=project(e[i][1]);
            drawLine(r, p1.x, p1.y, p2.x, p2.y, '#', TFT_YELLOW);
        }
    }
    const char* name() const override { return "Pyramid"; }
private:
    float _ang; struct Point { int x,y; };
    void drawLine(ASCIIRenderer& r, int x0, int y0, int x1, int y1, char ch, uint16_t col) {
        int dx=abs(x1-x0), dy=abs(y1-y0), sx=x0<x1?1:-1, sy=y0<y1?1:-1, err=dx-dy;
        while(1){ r.setCell(x0,y0,ch,col); if(x0==x1&&y0==y1) break; int e2=2*err;
            if(e2>-dy){err-=dy; x0+=sx;} if(e2<dx){err+=dx; y0+=sy;} }
    }
};

// 4.8 Wormhole
class Wormhole : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _t=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _t += 0.1f;
        int cx=r.kCols/2, cy=r.kRows/2;
        for(int y=0; y<r.kRows; ++y) {
            for(int x=0; x<r.kCols; ++x) {
                float dx=(x-cx)*0.5f, dy=(y-cy);
                float dist=sqrt(dx*dx+dy*dy);
                float ang=atan2(dy,dx);
                float distort = sin(ang*3.0f + _t);
                int v = (int)(20.0f/(dist+0.1f+distort*0.2f) + _t)%2;
                r.setCell(x, y, v ? '@' : ' ', M5.Display.color565(0, (int)(dist*10), 255));
            }
        }
    }
    const char* name() const override { return "Wormhole"; }
private: float _t;
};

// 4.9 Hypercube (4D wireframe)
class Hypercube : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _ang=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear(); _ang+=0.02f;
        float s=sin(_ang), c=cos(_ang);
        // Simplified 4D rotation and projection
        for(int i=0; i<16; ++i) {
            float x=(i&1)?1:-1, y=(i&2)?1:-1, z=(i&4)?1:-1, w=(i&8)?1:-1;
            float zw = z*c - w*s, rw = z*s + w*c; // Rotate ZW
            float ooz = 1.0f / (rw + 3.0f);
            int px = r.kCols/2 + (int)(x*ooz*30*1.5f), py = r.kRows/2 + (int)(y*ooz*30);
            r.setCell(px, py, '+', TFT_MAGENTA);
        }
    }
    const char* name() const override { return "Hypercube"; }
private: float _ang;
};

} // namespace idk

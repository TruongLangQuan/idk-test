#pragma once
#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class Cube3D : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _ang = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        _ang += 0.02f;
        float s = std::sin(_ang), c = std::cos(_ang);
        float verts[8][3] = {{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};
        int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
        
        auto project = [&](float x, float y, float z) {
            float rx = x*c - z*s;
            float rz = x*s + z*c;
            float ry = y*c - rz*s;
            rz = y*s + rz*c;
            float ooz = 1.0f / (rz + 4.0f);
            return Point{(int)(ASCIIRenderer::kCols/2 + rx*ooz*40), (int)(ASCIIRenderer::kRows/2 + ry*ooz*20)};
        };

        for(auto& e : edges) {
            Point p1 = project(verts[e[0]][0], verts[e[0]][1], verts[e[0]][2]);
            Point p2 = project(verts[e[1]][0], verts[e[1]][1], verts[e[1]][2]);
            drawLine(r, p1.x, p1.y, p2.x, p2.y, '#', TFT_WHITE);
        }
    }
    const char* name() const override { return "Cube 3D"; }
private:
    float _ang;
    struct Point { int x, y; };
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

#pragma once
#include "../../engine/IAnimation.h"
#include <cmath>

namespace idk {

// 3.6 Maze Generation
class MazeGen : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear('#', TFT_DARKGREY, TFT_BLACK);
        for(int row=0; row<r.kRows; ++row) for(int col=0; col<r.kCols; ++col) _maze[row][col] = 1;
        _stack.clear();
        _cx = 1; _cy = 1;
        _maze[_cy][_cx] = 0;
        _stack.push_back({_cx, _cy});
    }
    void update(ASCIIRenderer& r, float dtMs) override {
        if(_stack.empty()) return;
        _timer += dtMs;
        if(_timer < 50.0f) return;
        _timer = 0;

        int dirs[4][2] = {{0,-2},{0,2},{-2,0},{2,0}};
        std::vector<int> valid;
        for(int i=0; i<4; ++i) {
            int nx = _cx + dirs[i][0], ny = _cy + dirs[i][1];
            if(nx>0 && nx<r.kCols-1 && ny>0 && ny<r.kRows-1 && _maze[ny][nx] == 1) valid.push_back(i);
        }

        if(!valid.empty()) {
            int d = valid[esp_random()%valid.size()];
            _maze[_cy + dirs[d][1]/2][_cx + dirs[d][0]/2] = 0;
            _cx += dirs[d][0]; _cy += dirs[d][1];
            _maze[_cy][_cx] = 0;
            _stack.push_back({_cx, _cy});
        } else {
            auto back = _stack.back(); _stack.pop_back();
            _cx = back.x; _cy = back.y;
        }

        for(int row=0; row<r.kRows; ++row) {
            for(int col=0; col<r.kCols; ++col) {
                r.setCell(col, row, _maze[row][col] ? '#' : ' ', _maze[row][col] ? 0x4208 : TFT_BLACK, TFT_BLACK);
            }
        }
        r.setCell(_cx, _cy, '@', TFT_RED, TFT_BLACK);
    }
    const char* name() const override { return "Maze Gen"; }
private:
    uint8_t _maze[ASCIIRenderer::kRows][ASCIIRenderer::kCols];
    struct Point { int x, y; };
    std::vector<Point> _stack;
    int _cx, _cy;
    float _timer = 0;
};

// 3.14 Radar Sweep
class RadarSweep : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _ang = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        _ang += 0.05f * (dtMs / 33.3f);
        int cx = r.kCols/2, cy = r.kRows/2;
        for(float a = _ang; a > _ang - 1.0f; a -= 0.05f) {
            uint16_t col = M5.Display.color565(0, (uint8_t)(255 * (1.0f - (_ang - a))), 0);
            for(float d=0; d<15.0f; d+=0.5f) {
                int px = cx + (int)(d * std::cos(a) * 1.5f);
                int py = cy + (int)(d * std::sin(a));
                r.setCell(px, py, '.', col, TFT_BLACK);
            }
        }
        // Border
        for(float a=0; a<6.28f; a+=0.1f) {
            r.setCell(cx + (int)(15.0f * std::cos(a) * 1.5f), cy + (int)(15.0f * std::sin(a)), '+', TFT_DARKGREEN);
        }
    }
    const char* name() const override { return "Radar Sweep"; }
private: float _ang;
};

} // namespace idk

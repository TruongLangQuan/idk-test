#pragma once
#include "../../engine/IAnimation.h"
#include <deque>
#include <vector>

namespace idk {

// 3.7 Worm / Snake
class WormSnake : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _worm.clear();
        _worm.push_back({20, 8});
        _dir = 0; // Right
        _food = {5 + (int)(esp_random() % 30), 5 + (int)(esp_random() % 10)};
    }
    void update(ASCIIRenderer& r, float dtMs) override {
        _timer += dtMs;
        if (_timer < 100.0f) return;
        _timer = 0;
        
        r.clear();
        Point head = _worm.front();
        if (esp_random() % 10 == 0) _dir = esp_random() % 4;
        
        if (_dir == 0) head.x++; else if (_dir == 1) head.x--;
        else if (_dir == 2) head.y++; else head.y--;
        
        head.x = (head.x + r.kCols) % r.kCols;
        head.y = (head.y + r.kRows) % r.kRows;
        
        _worm.push_front(head);
        if (head.x == _food.x && head.y == _food.y) {
            _food = {esp_random() % r.kCols, esp_random() % r.kRows};
        } else if (_worm.size() > 8) {
            _worm.pop_back();
        }

        for (auto& p : _worm) r.setCell(p.x, p.y, 'o', TFT_GREEN);
        r.setCell(_worm.front().x, _worm.front().y, '@', TFT_YELLOW);
        r.setCell(_food.x, _food.y, '*', TFT_RED);
    }
    const char* name() const override { return "Worm/Snake"; }
private:
    struct Point { int x, y; };
    std::deque<Point> _worm;
    Point _food;
    int _dir;
    float _timer = 0;
};

// 3.8 Particle System (Wind)
class WindParticles : public IAnimation {
public:
    struct Part { float x, y, vx, vy; int life; };
    void init(ASCIIRenderer& r) override { _p.clear(); }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        if (_p.size() < 40) _p.push_back({0, (float)(esp_random()%r.kRows), 0.5f + (esp_random()%100)/100.0f, (esp_random()%50-25)/100.0f, 40});
        for (auto it = _p.begin(); it != _p.end();) {
            it->x += it->vx; it->y += it->vy; it->life--;
            if (it->life <= 0 || it->x >= r.kCols) it = _p.erase(it);
            else {
                r.setCell((int)it->x, (int)it->y, '.', TFT_LIGHTGREY);
                ++it;
            }
        }
    }
    const char* name() const override { return "Wind Particles"; }
private: std::vector<Part> _p;
};

// 3.9 Ripple / Raindrop
class Ripples : public IAnimation {
public:
    struct Rip { float x, y, r; int life; };
    void init(ASCIIRenderer& r) override { _rips.clear(); }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        if (esp_random() % 20 == 0) _rips.push_back({(float)(esp_random()%r.kCols), (float)(esp_random()%r.kRows), 0, 15});
        for (auto it = _rips.begin(); it != _rips.end();) {
            it->r += 0.5f; it->life--;
            if (it->life <= 0) it = _rips.erase(it);
            else {
                for (float a=0; a<6.28f; a+=0.5f) {
                    r.setCell((int)(it->x + it->r * cos(a) * 1.5f), (int)(it->y + it->r * sin(a)), (it->life > 10 ? 'O' : 'o'), TFT_BLUE);
                }
                ++it;
            }
        }
    }
    const char* name() const override { return "Ripples"; }
private: std::vector<Rip> _rips;
};

// 3.10 Typewriter
class Typewriter : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _idx = 0; _timer = 0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _timer += dtMs;
        if (_timer > 150.0f) { _timer = 0; _idx = (_idx + 1) % 40; }
        r.clear();
        char buf[41];
        strncpy(buf, "IDK-ASCII-IDK FIRMWARE v1.0 ONLINE... ", _idx);
        buf[_idx] = '\0';
        r.print(2, 8, buf, TFT_GREEN);
        if (((int)millis()/500) % 2) r.setCell(2 + _idx, 8, '_', TFT_GREEN);
    }
    const char* name() const override { return "Typewriter"; }
private: int _idx; float _timer;
};

// 3.11 Bouncing Ball
class BouncingBall : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _x=10; _y=5; _vx=0.4f; _vy=0.3f; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _x += _vx; _y += _vy;
        if (_x <= 0 || _x >= r.kCols-1) _vx = -_vx;
        if (_y <= 0 || _y >= r.kRows-1) _vy = -_vy;
        r.clear();
        r.setCell((int)_x, (int)_y, 'O', TFT_WHITE);
    }
    const char* name() const override { return "Bouncing Ball"; }
private: float _x, _y, _vx, _vy;
};

// 3.12 Ant Colony (Langton's Ant)
class LangtonsAnt : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        memset(_grid, 0, sizeof(_grid)); _ax=20; _ay=8; _ad=0;
    }
    void update(ASCIIRenderer& r, float dtMs) override {
        for(int i=0; i<5; ++i) { // Multi-step
            if (_grid[_ay][_ax]) { _ad = (_ad+1)%4; _grid[_ay][_ax]=0; }
            else { _ad = (_ad+3)%4; _grid[_ay][_ax]=1; }
            if (_ad==0) _ax++; else if (_ad==1) _ay++; else if (_ad==2) _ax--; else _ay--;
            _ax = (_ax+r.kCols)%r.kCols; _ay = (_ay+r.kRows)%r.kRows;
        }
        r.clear();
        for(int y=0; y<r.kRows; ++y) for(int x=0; x<r.kCols; ++x) if(_grid[y][x]) r.setCell(x,y, '#', TFT_WHITE);
        r.setCell(_ax, _ay, '@', TFT_RED);
    }
    const char* name() const override { return "Langton's Ant"; }
private: uint8_t _grid[ASCIIRenderer::kRows][ASCIIRenderer::kCols]; int _ax,_ay,_ad;
};

// 3.13 Binary Rain
class BinaryRain : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _y.assign(r.kCols, 0); }
    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        for(int x=0; x<r.kCols; ++x) {
            _y[x] = (_y[x] + 1) % r.kRows;
            r.setCell(x, _y[x], (esp_random()%2 ? '1' : '0'), TFT_GREEN);
        }
    }
    const char* name() const override { return "Binary Rain"; }
private: std::vector<int> _y;
};

// 3.15 Spinning Loader
class Spinner : public IAnimation {
public:
    void init(ASCIIRenderer& r) override { _f=0; }
    void update(ASCIIRenderer& r, float dtMs) override {
        _f = (int)(millis()/100) % 4;
        r.clear();
        const char s[] = "|/-\\";
        r.setCell(r.kCols/2, r.kRows/2, s[_f], TFT_CYAN);
    }
    const char* name() const override { return "Spinner"; }
private: int _f;
};

} // namespace idk

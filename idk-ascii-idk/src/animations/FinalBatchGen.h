#pragma once
#include "../engine/IAnimation.h"
#include <cmath>

namespace idk {

class MobiusStrip : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Mobius Strip"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Mobius Strip", TFT_YELLOW);
        
        for(int a=0; a<360; a+=15) {
            float u = a * 3.1415f / 180.0f;
            float v = std::sin(t + u)*0.5f;
            int x = 20 + (1.0f + v*std::cos(u/2.0f))*std::cos(u)*12;
            int y = 8 + (1.0f + v*std::cos(u/2.0f))*std::sin(u)*6;
            r.setCell(x, y, '8', TFT_MAGENTA);
        }
    
    }
};

class FractalTree : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Fractal Tree"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Fractal Tree", TFT_YELLOW);
        
        int x = 20; int y = 14;
        r.setCell(x, y, '|', TFT_GREEN);
        r.setCell(x-2, y-2, '/', TFT_GREEN); r.setCell(x+2, y-2, '\\', TFT_GREEN);
        int sway = std::sin(t)*2;
        r.setCell(x-4+sway, y-4, '*', TFT_DARKGREEN); r.setCell(x+4+sway, y-4, '*', TFT_DARKGREEN);
    
    }
};

class Aurora : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Aurora"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Aurora", TFT_YELLOW);
        
        for(int x=0; x<40; x++) {
            int y = 8 + std::sin(x*0.2f + t)*3 + std::cos(x*0.1f - t*0.5f)*2;
            r.setCell(x, y, '|', TFT_GREEN);
            r.setCell(x, y-1, ':', TFT_CYAN);
            r.setCell(x, y+1, '.', TFT_BLUE);
        }
    
    }
};

class Bubbles : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Bubbles"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Bubbles", TFT_YELLOW);
        
        for(int i=0; i<8; i++) {
            int x = (i * 5 + (int)t) % 40;
            int y = 15 - ((int)(t * (1.0f+i*0.2f)) % 16);
            r.setCell(x, y, 'O', TFT_CYAN);
            r.setCell(x+1, y-1, 'o', TFT_WHITE);
        }
    
    }
};

class SnowFall : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Snow Fall"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Snow Fall", TFT_YELLOW);
        
        for(int i=0; i<15; i++) {
            int x = (i * 7 + (int)(std::sin(t+i)*3)) % 40;
            int y = (int)(t * 2.0f + i*3) % 16;
            if(x<0) x+=40;
            r.setCell(x, y, '*', TFT_WHITE);
        }
    
    }
};

class OceanWaves : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Ocean Waves"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Ocean Waves", TFT_YELLOW);
        
        for(int x=0; x<40; x++) {
            int y = 10 + std::sin(x*0.3f + t*2.0f)*2.0f;
            r.setCell(x, y, '~', TFT_BLUE);
            r.setCell(x, y+1, '=', TFT_CYAN);
        }
    
    }
};

class AutumnLeaves : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Autumn Leaves"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Autumn Leaves", TFT_YELLOW);
        
        for(int i=0; i<6; i++) {
            int x = (i * 8 + (int)(t*2)) % 40;
            int y = (int)(t * 1.5f + i*2) % 16;
            r.setCell(x, y, "v/-\\"[(int)(t*3+i)%4], TFT_ORANGE);
        }
    
    }
};

class Fireflies : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Fireflies"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Fireflies", TFT_YELLOW);
        
        for(int i=0; i<10; i++) {
            int x = 20 + std::sin(t*0.5f + i)*15;
            int y = 8 + std::cos(t*0.7f + i*2)*6;
            if((int)(t*5+i)%3 == 0) r.setCell(x, y, '.', TFT_YELLOW);
        }
    
    }
};

class SandStorm : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Sand Storm"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Sand Storm", TFT_YELLOW);
        
        for(int i=0; i<40; i++) {
            int x = (i + (int)(t*10)) % 40;
            int y = (i*3 + (int)(t*2)) % 16;
            r.setCell(x, y, '-', TFT_ORANGE);
        }
    
    }
};

class Clock : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Clock"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Clock", TFT_YELLOW);
        
        int cx = 20, cy = 8;
        r.setCell(cx, cy, 'O', TFT_WHITE);
        int sx = cx + std::sin(t)*6; int sy = cy - std::cos(t)*4;
        r.setCell(sx, sy, '.', TFT_RED);
        int mx = cx + std::sin(t/10.0f)*4; int my = cy - std::cos(t/10.0f)*3;
        r.setCell(mx, my, 'o', TFT_LIGHTGREY);
    
    }
};

class ElectronOrbitals : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Electron Orbitals"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Electron Orbitals", TFT_YELLOW);
        
        int cx = 20, cy = 8;
        r.setCell(cx, cy, 'N', TFT_RED);
        int x1 = cx + std::sin(t*3)*8; int y1 = cy + std::cos(t*3)*4;
        int x2 = cx + std::cos(t*2)*4; int y2 = cy + std::sin(t*2)*6;
        r.setCell(x1, y1, 'e', TFT_BLUE);
        r.setCell(x2, y2, 'e', TFT_CYAN);
    
    }
};

class NuclearFission : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Nuclear Fission"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Nuclear Fission", TFT_YELLOW);
        
        if(((int)t)%4 < 2) {
            r.setCell(20, 8, 'U', TFT_YELLOW);
        } else {
            r.setCell(20 - ((int)t%4)*2, 8, 'B', TFT_RED);
            r.setCell(20 + ((int)t%4)*2, 8, 'K', TFT_ORANGE);
            r.setCell(20, 8 - ((int)t%4), '.', TFT_WHITE);
        }
    
    }
};

class EMWave : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "EM Wave"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "EM Wave", TFT_YELLOW);
        
        for(int x=0; x<40; x++) {
            int e = 8 + std::sin(x*0.5f - t*3)*4;
            int m = 8 + std::cos(x*0.5f - t*3)*4;
            r.setCell(x, e, 'E', TFT_BLUE);
            r.setCell(x, m, 'M', TFT_RED);
        }
    
    }
};

class Photoelectric : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Photoelectric"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Photoelectric", TFT_YELLOW);
        
        for(int i=0; i<40; i++) r.setCell(i, 14, '=', TFT_DARKGREY);
        int px = (int)(t*5) % 20;
        r.setCell(px, 14 - px/2, '~', TFT_YELLOW);
        if(px > 10) r.setCell(px+2, 10 - (px-10), 'e', TFT_CYAN);
    
    }
};

class Tunneling : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Tunneling"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Tunneling", TFT_YELLOW);
        
        for(int y=4; y<12; y++) r.setCell(20, y, '|', TFT_WHITE);
        int px = (int)(t*8) % 40;
        if(px < 20) r.setCell(px, 8, '>', TFT_GREEN);
        else if((int)t%3==0) r.setCell(px, 8, '>', TFT_DARKGREEN);
    
    }
};

class BurningShip : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Burning Ship"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Burning Ship", TFT_YELLOW);
        
        for(int x=0; x<40; x+=2) {
            for(int y=0; y<16; y+=2) {
                float zx=0, zy=0;
                float cx = (x-30)/10.0f; float cy = (y-8)/8.0f;
                int i=0;
                while(zx*zx+zy*zy < 4 && i<5) {
                    float tmp = zx*zx - zy*zy + cx;
                    zy = std::abs(2*zx*zy) + cy; zx = std::abs(tmp); i++;
                }
                if(i==5) r.setCell(x, y, '#', TFT_RED);
                else r.setCell(x, y, '.', TFT_ORANGE);
            }
        }
    
    }
};

class Sierpinski : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Sierpinski"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Sierpinski", TFT_YELLOW);
        
        for(int y=0; y<16; y++) {
            for(int x=0; x<32; x++) {
                if((x & y) == 0) r.setCell(x+4, y, '^', TFT_CYAN);
            }
        }
    
    }
};

class KochSnowflake : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Koch Snowflake"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Koch Snowflake", TFT_YELLOW);
        
        int s = (int)t % 3 + 1;
        r.setCell(20, 8, '*', TFT_WHITE);
        if(s>1) { r.setCell(20-2, 8, '-', TFT_CYAN); r.setCell(20+2, 8, '-', TFT_CYAN); }
        if(s>2) { r.setCell(20-3, 7, '/', TFT_BLUE); r.setCell(20+3, 9, '\\', TFT_BLUE); }
    
    }
};

class BarnsleyFern : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Barnsley Fern"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Barnsley Fern", TFT_YELLOW);
        
        float x=0, y=0;
        for(int i=0; i<40; i++) {
            float rnd = (esp_random()%100)/100.0f;
            float nx, ny;
            if(rnd<0.01f) { nx=0; ny=0.16f*y; }
            else if(rnd<0.86f) { nx=0.85f*x + 0.04f*y; ny=-0.04f*x + 0.85f*y + 1.6f; }
            else if(rnd<0.93f) { nx=0.2f*x - 0.26f*y; ny=0.23f*x + 0.22f*y + 1.6f; }
            else { nx=-0.15f*x + 0.28f*y; ny=0.26f*x + 0.24f*y + 0.44f; }
            x=nx; y=ny;
            int px = 20 + x*4; int py = 15 - y*1.5f;
            r.setCell(px, py, ',', TFT_GREEN);
        }
    
    }
};

class DragonCurve : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Dragon Curve"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Dragon Curve", TFT_YELLOW);
        
        int len = ((int)t % 10) + 2;
        int px = 20, py = 8;
        for(int i=0; i<len; i++) {
            r.setCell(px, py, '+', TFT_MAGENTA);
            if((i & (i-1)) == 0) px++; else py++;
        }
    
    }
};

class HilbertCurve : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Hilbert Curve"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Hilbert Curve", TFT_YELLOW);
        
        int len = ((int)t % 8) + 2;
        int px = 10, py = 4;
        for(int i=0; i<len; i++) {
            r.setCell(px + i*2, py + (i%2)*2, '_', TFT_CYAN);
            r.setCell(px + i*2 + 1, py + (i%2)*2, '|', TFT_CYAN);
        }
    
    }
};

class LSystem : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "L-System"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "L-System", TFT_YELLOW);
        
        int len = (int)(t*5) % 15;
        int x = 20, y = 15;
        for(int i=0; i<len; i++) {
            r.setCell(x, y-i, '|', TFT_GREEN);
            if(i%3==0) r.setCell(x-1, y-i, '/', TFT_DARKGREEN);
            if(i%4==0) r.setCell(x+1, y-i, '\\', TFT_DARKGREEN);
        }
    
    }
};

class MZoom : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "MZoom"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "MZoom", TFT_YELLOW);
        
        float zoom = 1.0f + std::fmod(t, 5.0f);
        for(int x=0; x<40; x+=2) {
            for(int y=0; y<16; y+=2) {
                float cx = (x-20)/(10.0f*zoom) - 0.5f; 
                float cy = (y-8)/(8.0f*zoom);
                float zx=0, zy=0; int i=0;
                while(zx*zx+zy*zy < 4 && i<10) {
                    float tmp = zx*zx - zy*zy + cx;
                    zy = 2*zx*zy + cy; zx = tmp; i++;
                }
                if(i==10) r.setCell(x, y, '#', TFT_WHITE);
            }
        }
    
    }
};

class Rossler : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Rossler"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Rossler", TFT_YELLOW);
        
        float a=0.2f, b=0.2f, c=5.7f;
        float x=1, y=1, z=1;
        for(int i=0; i<100; i++) {
            float dx = -y - z;
            float dy = x + a*y;
            float dz = b + z*(x - c);
            x += dx*0.05f; y += dy*0.05f; z += dz*0.05f;
            int px = 20 + x; int py = 8 + y;
            r.setCell(px, py, '.', TFT_ORANGE);
        }
    
    }
};

class LogisticMap : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Logistic Map"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Logistic Map", TFT_YELLOW);
        
        float rx = 2.5f + std::fmod(t*0.5f, 1.5f);
        float x = 0.5f;
        for(int i=0; i<40; i++) {
            x = rx * x * (1 - x);
            int py = 15 - (int)(x*15);
            r.setCell(i, py, '*', TFT_CYAN);
        }
    
    }
};

class HenonMap : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Henon Map"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Henon Map", TFT_YELLOW);
        
        float x=0, y=0;
        for(int i=0; i<100; i++) {
            float nx = 1 - 1.4f*x*x + y;
            float ny = 0.3f*x;
            x = nx; y = ny;
            int px = 20 + x*10; int py = 8 + y*10;
            r.setCell(px, py, '.', TFT_MAGENTA);
        }
    
    }
};

class ArnoldCat : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Arnold Cat"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Arnold Cat", TFT_YELLOW);
        
        for(int x=0; x<10; x++) {
            for(int y=0; y<10; y++) {
                int nx = (x + y) % 10;
                int ny = (x + 2*y) % 10;
                int step = (int)t % 5;
                if(step%2==0) r.setCell(15+nx, 3+ny, '#', TFT_YELLOW);
                else r.setCell(15+x, 3+y, '#', TFT_YELLOW);
            }
        }
    
    }
};

class Lyapunov : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Lyapunov"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Lyapunov", TFT_YELLOW);
        
        for(int x=0; x<40; x+=2) {
            float rval = 2.0f + (x/40.0f)*2.0f;
            float pop = 0.5f;
            for(int i=0; i<10; i++) pop = rval * pop * (1-pop);
            r.setCell(x, 15-(int)(pop*10), '~', TFT_GREEN);
        }
    
    }
};

class ThreeBody : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "3-Body"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "3-Body", TFT_YELLOW);
        
        for(int i=0; i<3; i++) {
            int px = 20 + std::sin(t*(i+1) + i*2.0f)*10;
            int py = 8 + std::cos(t*(1.5f-i*0.2f) + i)*5;
            r.setCell(px, py, 'O', i==0?TFT_RED:(i==1?TFT_GREEN:TFT_BLUE));
        }
    
    }
};

class ParticleGas : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Particle Gas"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Particle Gas", TFT_YELLOW);
        
        for(int i=0; i<20; i++) {
            int px = (int)(std::abs(std::sin(t*2+i*13)*40)) % 40;
            int py = (int)(std::abs(std::cos(t*3+i*7)*16)) % 16;
            r.setCell(px, py, '.', TFT_LIGHTGREY);
        }
    
    }
};

class GalaxyMerge : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Galaxy Merge"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Galaxy Merge", TFT_YELLOW);
        
        for(int i=0; i<20; i++) {
            float a = i*0.5f + t;
            int x1 = 15 + std::cos(a)*i*0.4f; int y1 = 8 + std::sin(a)*i*0.2f;
            int x2 = 25 + std::cos(-a)*i*0.4f; int y2 = 8 + std::sin(-a)*i*0.2f;
            r.setCell(x1, y1, '*', TFT_CYAN);
            r.setCell(x2, y2, '*', TFT_MAGENTA);
        }
    
    }
};

class Swarm : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Swarm"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Swarm", TFT_YELLOW);
        
        int cx = 20 + std::sin(t*2)*10;
        int cy = 8 + std::cos(t*1.5f)*4;
        r.setCell(cx, cy, 'X', TFT_RED);
        for(int i=0; i<10; i++) {
            r.setCell(cx + std::sin(t*5+i)*3, cy + std::cos(t*4+i*2)*2, '-', TFT_YELLOW);
        }
    
    }
};

class SpringMass : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Spring-Mass"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Spring-Mass", TFT_YELLOW);
        
        int py = 2 + std::abs(std::sin(t*3)*10);
        for(int y=0; y<py; y++) r.setCell(20, y, '|', TFT_DARKGREY);
        r.setCell(20, py, 'W', TFT_WHITE);
    
    }
};

class Vortex : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Vortex"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Vortex", TFT_YELLOW);
        
        for(int r_i=1; r_i<8; r_i++) {
            int x = 20 + std::cos(t*3 - r_i*0.5f)*r_i*2;
            int y = 8 + std::sin(t*3 - r_i*0.5f)*r_i;
            r.setCell(x, y, '@', TFT_CYAN);
        }
    
    }
};

class Charged : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Charged"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Charged", TFT_YELLOW);
        
        r.setCell(15, 8, '+', TFT_RED); r.setCell(25, 8, '-', TFT_BLUE);
        for(int i=0; i<8; i++) {
            int x = 15 + (int)(t*10 + i*5)%10;
            int y = 8 + std::sin(x*0.6f)*3;
            r.setCell(x, y, '.', TFT_WHITE);
        }
    
    }
};

class Smoke : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Smoke"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Smoke", TFT_YELLOW);
        
        for(int i=0; i<10; i++) {
            int py = 15 - ((int)(t*5 + i*2)%16);
            int px = 20 + std::sin(py*0.3f + t)*3;
            r.setCell(px, py, '&', TFT_DARKGREY);
        }
    
    }
};

class VortexStreet : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Vortex Street"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Vortex Street", TFT_YELLOW);
        
        r.setCell(10, 8, 'O', TFT_RED);
        for(int x=12; x<40; x+=2) {
            int py = 8 + std::sin(x*0.5f - t*5)*3;
            r.setCell(x, py, '~', TFT_CYAN);
        }
    
    }
};

class KHInstability : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "KH Instability"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "KH Instability", TFT_YELLOW);
        
        for(int x=0; x<40; x++) {
            int y1 = 6 + std::sin(x*0.4f + t*2)*2;
            int y2 = 10 + std::sin(x*0.4f - t*2 + 3.14f)*2;
            r.setCell(x, y1, '>', TFT_BLUE);
            r.setCell(x, y2, '<', TFT_DARKGREEN);
        }
    
    }
};

class Convection : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Convection"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Convection", TFT_YELLOW);
        
        for(int i=0; i<8; i++) {
            int px = 10 + i*3;
            int py = 14 - ((int)(t*4 + i*3)%10);
            r.setCell(px, py, '^', TFT_RED);
            r.setCell(px+1, 14-py+4, 'v', TFT_BLUE);
        }
    
    }
};

class Turbulence : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Turbulence"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Turbulence", TFT_YELLOW);
        
        for(int i=0; i<40; i++) {
            int px = esp_random()%40;
            int py = esp_random()%16;
            r.setCell(px, py, ".,~%*"[esp_random()%5], TFT_DARKGREY);
        }
    
    }
};

class Soliton : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Soliton"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Soliton", TFT_YELLOW);
        
        for(int x=0; x<40; x++) {
            float dist = std::abs(x - (std::fmod(t*10, 60.0f) - 10));
            int h = dist < 5 ? 5 - dist : 0;
            r.setCell(x, 12 - h, '^', TFT_CYAN);
            r.setCell(x, 13, '=', TFT_BLUE);
        }
    
    }
};

class Eclipse : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Eclipse"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Eclipse", TFT_YELLOW);
        
        int mx = 20 + std::sin(t)*15;
        r.setCell(20, 8, 'O', TFT_YELLOW);
        r.setCell(mx, 8, 'o', TFT_DARKGREY);
        if(std::abs(mx-20) < 2) r.setCell(20, 8, '@', TFT_WHITE);
    
    }
};

class Comet : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Comet"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Comet", TFT_YELLOW);
        
        int cx = 40 - ((int)(t*10)%50);
        int cy = ((int)(t*4)%20) - 2;
        r.setCell(cx, cy, '*', TFT_WHITE);
        for(int i=1; i<6; i++) r.setCell(cx+i, cy-i/2, '-', TFT_CYAN);
    
    }
};

class Supernova : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Supernova"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Supernova", TFT_YELLOW);
        
        int stage = (int)(t*2) % 5;
        if(stage == 0) r.setCell(20, 8, '*', TFT_WHITE);
        else if(stage == 1) { r.setCell(20, 8, 'O', TFT_YELLOW); }
        else {
            for(int r_i=0; r_i<stage*3; r_i++) {
                int px = 20 + std::cos(r_i*0.5f)*stage*2;
                int py = 8 + std::sin(r_i*0.5f)*stage;
                r.setCell(px, py, '.', TFT_RED);
            }
        }
    
    }
};

class GWaves : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "G-Waves"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "G-Waves", TFT_YELLOW);
        
        for(int x=0; x<40; x+=2) {
            for(int y=0; y<16; y+=2) {
                float d1 = std::sqrt(std::pow(x-15,2) + std::pow(y-8,2));
                float d2 = std::sqrt(std::pow(x-25,2) + std::pow(y-8,2));
                int val = (int)(std::sin(d1-t*5) + std::sin(d2-t*5));
                if(val > 0) r.setCell(x, y, ')', TFT_MAGENTA);
            }
        }
    
    }
};

class BigBang : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Big Bang"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Big Bang", TFT_YELLOW);
        
        int exp = (int)(t*5) % 20;
        for(int i=0; i<exp*4; i++) {
            float ang = esp_random()%360 * 3.14f/180.0f;
            int px = 20 + std::cos(ang)*(esp_random()%exp);
            int py = 8 + std::sin(ang)*(esp_random()%exp)/2;
            r.setCell(px, py, '.', TFT_YELLOW);
        }
    
    }
};

class Asteroids : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Asteroids"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Asteroids", TFT_YELLOW);
        
        for(int i=0; i<5; i++) {
            int px = (int)(i*10 - t*5) % 40;
            if(px<0) px+=40;
            int py = (i*5 + (int)t) % 16;
            r.setCell(px, py, "0oO"[i%3], TFT_DARKGREY);
        }
    
    }
};

class Nebula : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Nebula"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Nebula", TFT_YELLOW);
        
        for(int x=10; x<30; x+=2) {
            for(int y=4; y<12; y+=2) {
                if((std::sin(x*0.5f+t) + std::cos(y*0.8f-t)) > 0.5f)
                    r.setCell(x, y, '%', TFT_MAGENTA);
                else r.setCell(x, y, '.', TFT_BLUE);
            }
        }
    
    }
};

class Fourier : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Fourier"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Fourier", TFT_YELLOW);
        
        for(int x=0; x<40; x++) {
            float y = std::sin(x*0.2f - t) + std::sin(x*0.6f + t*2)*0.5f;
            r.setCell(x, 8 + (int)(y*3), '~', TFT_GREEN);
        }
    
    }
};

class Voronoi : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Voronoi"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Voronoi", TFT_YELLOW);
        
        int p1x = 10+std::sin(t)*5, p1y = 8+std::cos(t)*3;
        int p2x = 30+std::cos(t)*5, p2y = 8+std::sin(t)*3;
        for(int x=0; x<40; x+=2) {
            for(int y=0; y<16; y+=2) {
                float d1 = (x-p1x)*(x-p1x) + (y-p1y)*(y-p1y);
                float d2 = (x-p2x)*(x-p2x) + (y-p2y)*(y-p2y);
                r.setCell(x, y, d1<d2?'+':'-', d1<d2?TFT_RED:TFT_BLUE);
            }
        }
    
    }
};

class Contour : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Contour"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Contour", TFT_YELLOW);
        
        for(int x=0; x<40; x++) {
            for(int y=0; y<16; y+=2) {
                float v = std::sin(x*0.2f+t)*std::cos(y*0.3f+t);
                if(std::abs(v) < 0.1f) r.setCell(x, y, '=', TFT_YELLOW);
            }
        }
    
    }
};

class Quaternion : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Quaternion"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Quaternion", TFT_YELLOW);
        
        int qx = 20 + std::sin(t*2)*std::cos(t)*10;
        int qy = 8 + std::sin(t*2)*std::sin(t)*5;
        r.setCell(qx, qy, 'Q', TFT_CYAN);
        r.setCell(40-qx, 16-qy, 'q', TFT_MAGENTA);
    
    }
};

class NonEuclidean : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Non-Euclidean"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Non-Euclidean", TFT_YELLOW);
        
        for(int x=0; x<40; x+=2) {
            float dist = std::abs(x-20);
            int y = 8 + std::sin(t*2)*std::log(dist+1)*2;
            r.setCell(x, y, '#', TFT_RED);
        }
    
    }
};

class Sieve : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Sieve"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Sieve", TFT_YELLOW);
        
        int max_n = ((int)(t*10)%40) + 2;
        for(int i=2; i<=max_n; i++) {
            bool prime = true;
            for(int j=2; j*j<=i; j++) if(i%j==0) prime=false;
            if(prime) r.setCell(i, 8, 'P', TFT_GREEN);
            else r.setCell(i, 10, '.', TFT_DARKGREY);
        }
    
    }
};

class Penrose : public IAnimation {
private:
    float t = 0;
public:
    const char* name() const override { return "Penrose"; }
    void init(ASCIIRenderer& r) override { t = 0; }
    void update(ASCIIRenderer& r, float dt) override {
        t += dt * 0.001f;
        r.clear(' ', TFT_BLACK);
        r.print(2, 0, "Penrose", TFT_YELLOW);
        
        for(int i=0; i<6; i++) {
            int x = 20 + std::cos(i*3.14f/3.0f + t)*8;
            int y = 8 + std::sin(i*3.14f/3.0f + t)*4;
            r.setCell(x, y, '/', TFT_CYAN);
        }
    
    }
};

} // namespace idk

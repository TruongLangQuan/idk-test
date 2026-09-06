#pragma once
#include <M5Unified.h>
#include <cmath>

namespace idk {

class WorldGenerator {
public:
    void randomize() {
        _seed = esp_random();
        _a = (float)(esp_random() % 100) / 50.0f + 0.5f;
        _b = (float)(esp_random() % 100) / 1000.0f + 0.01f;
        _c = (float)(esp_random() % 100) / 50.0f + 0.5f;
        _d = (float)(esp_random() % 100) / 1000.0f + 0.01f;
        _e = (float)(esp_random() % 100) / 20.0f + 2.0f;
    }

    // Mathematical terrain function: y = f(x, z)
    float getHeight(float x, float z) {
        float h = _a * std::sin(x * _b + _seed * 0.001f) + 
                  _c * std::cos(z * _d + _seed * 0.002f);
        
        // Simple fractal noise approximation
        h += std::sin(x * 0.1f) * std::cos(z * 0.1f) * 0.5f;
        h += std::sin(x * 0.5f + z * 0.5f) * 0.2f;
        
        return h * _e;
    }

private:
    uint32_t _seed;
    float _a, _b, _c, _d, _e;
};

} // namespace idk

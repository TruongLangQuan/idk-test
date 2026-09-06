#pragma once
#include "../../engine/IAnimation.h"
#include <vector>

namespace idk {

class Boids : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        r.clear();
        _boids.resize(30);
        for (auto& b : _boids) {
            b.x = esp_random() % ASCIIRenderer::kCols;
            b.y = esp_random() % ASCIIRenderer::kRows;
            b.vx = ((esp_random() % 100) / 50.0f) - 1.0f;
            b.vy = ((esp_random() % 100) / 50.0f) - 1.0f;
        }
    }

    void update(ASCIIRenderer& r, float dtMs) override {
        r.clear();
        
        for (auto& b : _boids) {
            // Very simplified Boids logic for performance
            float cx = 0, cy = 0; // Cohesion
            float ax = 0, ay = 0; // Alignment
            float sx = 0, sy = 0; // Separation
            int neighbors = 0;

            for (const auto& other : _boids) {
                if (&b == &other) continue;
                float distSq = (b.x - other.x)*(b.x - other.x) + (b.y - other.y)*(b.y - other.y);
                
                if (distSq < 25.0f) {
                    cx += other.x; cy += other.y;
                    ax += other.vx; ay += other.vy;
                    neighbors++;
                    
                    if (distSq < 4.0f) {
                        sx += (b.x - other.x);
                        sy += (b.y - other.y);
                    }
                }
            }

            if (neighbors > 0) {
                cx /= neighbors; cy /= neighbors;
                ax /= neighbors; ay /= neighbors;
                
                b.vx += (cx - b.x) * 0.01f + ax * 0.05f + sx * 0.1f;
                b.vy += (cy - b.y) * 0.01f + ay * 0.05f + sy * 0.1f;
            }

            // Speed limit
            float speedSq = b.vx*b.vx + b.vy*b.vy;
            if (speedSq > 4.0f) {
                float speed = std::sqrt(speedSq);
                b.vx = (b.vx / speed) * 2.0f;
                b.vy = (b.vy / speed) * 2.0f;
            }

            // Move
            b.x += b.vx * (dtMs / 33.0f);
            b.y += b.vy * (dtMs / 33.0f);

            // Wrap around
            if (b.x < 0) b.x += ASCIIRenderer::kCols;
            if (b.x >= ASCIIRenderer::kCols) b.x -= ASCIIRenderer::kCols;
            if (b.y < 0) b.y += ASCIIRenderer::kRows;
            if (b.y >= ASCIIRenderer::kRows) b.y -= ASCIIRenderer::kRows;

            // Render
            char ch = '*';
            if (std::abs(b.vx) > std::abs(b.vy)) {
                ch = b.vx > 0 ? '>' : '<';
            } else {
                ch = b.vy > 0 ? 'v' : '^';
            }
            r.setCell((int)b.x, (int)b.y, ch, TFT_GREEN, TFT_BLACK);
        }
    }

    const char* name() const override { return "Boids Swarm"; }

private:
    struct Boid {
        float x, y;
        float vx, vy;
    };
    std::vector<Boid> _boids;
};

} // namespace idk

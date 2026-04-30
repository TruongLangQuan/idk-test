#pragma once
// ButterflyEntity.h - NPC butterfly that flies around in the outside scene

#include "Entity.h"
#include "Renderer.h"
#include "assets/nature_assets.h"

class ButterflyEntity : public Entity {
public:
    ButterflyEntity(float x, float y) : Entity(x, y),
        _animCounter(0.0f), _vx(0.5f), _vy(0.3f),
        _dirTimer(0.0f), _dirInterval(2.0f)
    {}

    void update(float dt) override {
        // Animate
        int fc = BUTTERFLY1.frame_count;
        _animCounter += dt * 8.0f;
        if (_animCounter >= fc) _animCounter -= fc;

        // AI direction change
        _dirTimer += dt;
        if (_dirTimer >= _dirInterval) {
            _dirTimer = 0;
            _pickDirection();
        }

        // Move
        x += _vx * dt * 20.0f;
        y += _vy * dt * 20.0f;

        // Bounce off world bounds (scaled for 240x display)
        float bl = 20.0f, br = 220.0f, bt = 20.0f, bb = 90.0f;
        if (x < bl) { x = bl; _vx =  fabsf(_vx); }
        if (x > br) { x = br; _vx = -fabsf(_vx); }
        if (y < bt) { y = bt; _vy =  fabsf(_vy); }
        if (y > bb) { y = bb; _vy = -fabsf(_vy); }
    }

    void draw(Renderer& r, int cameraOffset = 0) {
        if (!visible) return;
        int f = (int)_animCounter % max(1, BUTTERFLY1.frame_count);
        r.drawSpriteObj(&BUTTERFLY1, (int)x - cameraOffset, (int)y, f, false);
    }

private:
    float _animCounter;
    float _vx, _vy;
    float _dirTimer, _dirInterval;

    void _pickDirection() {
        if (random(100) < 50) _vy = (random(100) - 50) / 100.0f;
        if (random(100) < 30) _vx = (random(100) - 50) / 100.0f;
        _dirInterval = 1.0f + random(20) / 10.0f;
    }
};

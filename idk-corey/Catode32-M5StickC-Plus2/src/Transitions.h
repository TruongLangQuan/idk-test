#pragma once
// Transitions.h - Screen transition effects (fade, wipe, iris)

#include "Renderer.h"
#include <Arduino.h>
#include <math.h>

using TransitionCb = void(*)();

class TransitionManager {
public:
    bool active = false;

    TransitionManager(Renderer* r, const char* type = "fade", float duration = 0.4f)
        : _r(r), _type(type), _duration(duration),
          _progress(0.0f), _phase("out"),
          _midpointCb(nullptr), _midpointCalled(false) {}

    void start(TransitionCb onMidpoint = nullptr) {
        if (active) return;
        active          = true;
        _phase          = "out";
        _progress       = 0.0f;
        _midpointCb     = onMidpoint;
        _midpointCalled = false;
    }

    bool update(float dt) {
        if (!active) return false;
        _progress += dt / _duration;
        if (_progress >= 1.0f) {
            _progress = 1.0f;
            if (strcmp(_phase, "out") == 0) {
                // Midpoint
                if (!_midpointCalled && _midpointCb) {
                    _midpointCalled = true;
                    _midpointCb();
                }
                _phase    = "in";
                _progress = 0.0f;
            } else {
                // Done
                active = false;
            }
        }
        return true;
    }

    void draw() {
        if (!active) return;
        float t = (strcmp(_phase, "out") == 0) ? _progress : (1.0f - _progress);
        if (strcmp(_type, "fade") == 0)     _drawFade(t);
        else if (strcmp(_type, "wipe") == 0) _drawWipe(t);
        else if (strcmp(_type, "iris") == 0) _drawIris(t);
        else _drawFade(t);
    }

private:
    Renderer*   _r;
    const char* _type;
    float       _duration;
    float       _progress;
    const char* _phase;
    TransitionCb _midpointCb;
    bool        _midpointCalled;

    void _drawFade(float t) {
        // Simple fill with increasing opacity using dithering
        // t=0 = transparent, t=1 = fully black
        if (t <= 0) return;
        if (t >= 1.0f) {
            _r->drawRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_BLACK, true);
            return;
        }
        // Dither pattern: draw every N-th pixel
        int step = max(1, (int)(1.0f / t));
        for (int y = 0; y < DISPLAY_HEIGHT; y++)
            for (int x = (y % step); x < DISPLAY_WIDTH; x += step)
                _r->drawPixel(x, y, COLOR_BLACK);
    }

    void _drawWipe(float t) {
        // Horizontal wipe from left
        int w = (int)(t * DISPLAY_WIDTH);
        if (w > 0) _r->drawRect(0, 0, w, DISPLAY_HEIGHT, COLOR_BLACK, true);
    }

    void _drawIris(float t) {
        // Iris close: black corners expanding to center circle
        if (t <= 0) return;
        if (t >= 1.0f) {
            _r->drawRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_BLACK, true);
            return;
        }
        int cx = DISPLAY_WIDTH / 2;
        int cy = DISPLAY_HEIGHT / 2;
        float maxR = sqrtf(cx*cx + cy*cy);
        float r = maxR * (1.0f - t);
        int ir = (int)r;

        _r->drawRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_BLACK, true);
        // Clear the circle (draw it in a bright color — scene was drawn before)
        // Since we can't mask, we fill everything then cut out the circle
        // by re-drawing background... instead just draw a circle border effect
        if (ir > 0)
            _r->drawCircle(cx, cy, ir, COLOR_BLACK, true);
    }
};

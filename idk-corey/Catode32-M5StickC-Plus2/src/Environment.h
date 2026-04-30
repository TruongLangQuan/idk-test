#pragma once
// Environment.h - Parallax scrolling world with layered objects

#include "Renderer.h"
#include "config.h"

// Layer parallax factors
static const float PARALLAX_BG = 0.3f;
static const float PARALLAX_MG = 0.6f;
static const float PARALLAX_FG = 1.0f;

enum Layer { LAYER_BACKGROUND=0, LAYER_MIDGROUND=1, LAYER_FOREGROUND=2 };
static const int LAYER_COUNT = 3;
static const float PARALLAX[LAYER_COUNT] = { PARALLAX_BG, PARALLAX_MG, PARALLAX_FG };

// ── Env Object (static sprite placed in world) ─────────────────────────────

static const int MAX_ENV_OBJECTS = 32;

struct EnvObject {
    const Sprite* sprite  = nullptr;
    float         x       = 0;
    float         y       = 0;
    int           frame   = 0;      // current frame (can be animated externally)
    float         rotate  = 0.0f;  // degrees (unused for now, reserved)
    bool          mirror_h= false;
    bool          active  = false;
};

// ── Custom draw callback ───────────────────────────────────────────────────

using DrawCallback = void(*)(Renderer& r, float cameraX, float parallax, void* data);

static const int MAX_DRAW_CALLBACKS = 8;
struct DrawCb { DrawCallback fn; Layer layer; void* data; bool active; };

// ── Environment ────────────────────────────────────────────────────────────

class Environment {
public:
    float cameraX = 0.0f;
    float worldWidth;

    Environment(float worldWidth = 256.0f * SPRITE_SCALE)
        : worldWidth(worldWidth), cameraX(0.0f), _objCount(0), _cbCount(0) {}

    // ── Object management ──────────────────────────────────────────────
    EnvObject* addObject(Layer layer, const Sprite* sprite, float wx, float wy,
                         bool mirror_h = false) {
        if (_objCount >= MAX_ENV_OBJECTS) return nullptr;
        EnvObject& o = _objects[_objCount++];
        o.sprite   = sprite;
        o.x        = wx;
        o.y        = wy;
        o.frame    = 0;
        o.mirror_h = mirror_h;
        o.active   = true;
        o.rotate   = 0;
        _layerOf[_objCount - 1] = layer;
        return &o;
    }

    void removeObject(const EnvObject* ptr) {
        for (int i = 0; i < _objCount; i++) {
            if (&_objects[i] == ptr) {
                _objects[i].active = false;
                return;
            }
        }
    }

    void addCustomDraw(Layer layer, DrawCallback fn, void* data = nullptr) {
        if (_cbCount >= MAX_DRAW_CALLBACKS) return;
        _callbacks[_cbCount++] = { fn, layer, data, true };
    }

    // ── Camera ────────────────────────────────────────────────────────
    void pan(float dx) {
        float maxCam = max(0.0f, worldWidth - PLAY_WIDTH);
        cameraX = max(0.0f, min(maxCam, cameraX + dx));
    }

    void setCamera(float x) {
        float maxCam = max(0.0f, worldWidth - PLAY_WIDTH);
        cameraX = max(0.0f, min(maxCam, x));
    }

    // ── Draw ──────────────────────────────────────────────────────────
    void draw(Renderer& r) {
        for (int layer = 0; layer < LAYER_COUNT; layer++) {
            float par = PARALLAX[layer];
            float camOff = cameraX * par;

            // Custom callbacks for this layer
            for (int c = 0; c < _cbCount; c++) {
                if (_callbacks[c].active && _callbacks[c].layer == layer)
                    _callbacks[c].fn(r, cameraX, par, _callbacks[c].data);
            }

            // Static objects
            for (int i = 0; i < _objCount; i++) {
                if (!_objects[i].active) continue;
                if (_layerOf[i] != layer) continue;
                const EnvObject& o = _objects[i];
                if (!o.sprite) continue;

                int sx = (int)(o.x - camOff);
                int sy = (int)(o.y) + PLAY_Y;

                // Cull off-screen
                int sw = o.sprite->width * SPRITE_SCALE;
                if (sx + sw < 0 || sx > DISPLAY_WIDTH) continue;

                r.drawSpriteObj(o.sprite, sx, sy, o.frame, o.mirror_h);
            }
        }
    }

private:
    EnvObject _objects[MAX_ENV_OBJECTS];
    Layer     _layerOf[MAX_ENV_OBJECTS];
    int       _objCount;

    DrawCb    _callbacks[MAX_DRAW_CALLBACKS];
    int       _cbCount;
};

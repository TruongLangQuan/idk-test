#pragma once
// CharacterEntity.h - Main pet character with pose + behavior system

#include <Arduino.h>
#include "Entity.h"
#include "Renderer.h"
#include "GameContext.h"
#include "assets/character_assets.h"

class BaseBehavior;

// ── Pose part animation state ──────────────────────────────────────────────

struct PartAnim {
    float counter = 0.0f;
};

// ── CharacterEntity ────────────────────────────────────────────────────────

class CharacterEntity : public Entity {
public:
    GameContext* context;

    CharacterEntity(float x, float y, const char* pose = "sitting.forward.neutral",
                    GameContext* ctx = nullptr)
        : Entity(x, y), context(ctx),
          _poseName(pose), _poseEntry(nullptr),
          _currentBehavior(nullptr)
    {
        _poseEntry = findPose(pose);
    }

    ~CharacterEntity() {
        delete _currentBehavior;
    }

    // ── Pose ─────────────────────────────────────────────────────────────
    bool setPose(const char* name) {
        const PoseEntry* p = findPose(name);
        if (!p) return false;
        _poseName  = name;
        _poseEntry = p;
        return true;
    }

    const char* poseName() const { return _poseName; }

    // ── Behavior management ───────────────────────────────────────────────
    void setCurrentBehavior(BaseBehavior* b) {
        _currentBehavior = b;
    }

    BaseBehavior* currentBehavior() const { return _currentBehavior; }

    // Interrupt any current behavior and start the given one
    void trigger(BaseBehavior* newBehavior);

    // Called by BaseBehavior::_chainTo when it needs a fresh idle
    BaseBehavior* makeIdleBehavior();

    // ── Update / Draw ─────────────────────────────────────────────────────
    void update(float dt) override {
        if (!_poseEntry) return;

        const PoseEntry& p = *_poseEntry;
        _body.counter = _advCounter(_body.counter, dt, p.body->speed,
                                    p.body->frame_count + p.body->extra_frames);
        _head.counter = _advCounter(_head.counter, dt, p.head->speed,
                                    p.head->frame_count + p.head->extra_frames);
        _eyes.counter = _advCounter(_eyes.counter, dt, p.eyes->speed,
                                    p.eyes->frame_count + p.eyes->extra_frames);
        _tail.counter = _advCounter(_tail.counter, dt, p.tail->speed,
                                    p.tail->frame_count + p.tail->extra_frames);

        _updateBehavior(dt);
    }

    // Draw the character at its world position minus camera_offset
    void draw(Renderer& r, bool mirror = false, int cameraOffset = 0, int scale = SPRITE_SCALE) {
        if (!visible || !_poseEntry) return;

        const PoseEntry& p = *_poseEntry;
        int px = (int)x - cameraOffset;
        int py = (int)y;

        const Sprite* body = p.body;
        const Sprite* head = p.head;
        const Sprite* eyes = p.eyes;
        const Sprite* tail = p.tail;

        int bodyF  = _frameIdx(body, _body.counter);
        int headF  = _frameIdx(head, _head.counter);
        int eyeF   = _frameIdx(eyes, _eyes.counter);
        int tailF  = _frameIdx(tail, _tail.counter);

        int ancBX  = mirror ? (body->width - body->anchor_x) : body->anchor_x;
        int bodyX  = px - ancBX * scale;
        int bodyY  = py - body->anchor_y * scale;

        // Head attachment point on body
        int hx = body->head_x.get(bodyF);
        int hy = body->head_y.get(bodyF);
        if (mirror) hx = body->width - hx;
        int headRootX = bodyX + hx * scale;
        int headRootY = bodyY + hy * scale;
        int ancHX = mirror ? (head->width - head->anchor_x) : head->anchor_x;
        int headX = headRootX - ancHX * scale;
        int headY = headRootY - head->anchor_y * scale;

        // Eye attachment on head
        int ex = head->eye_x.get(headF);
        int ey = head->eye_y.get(headF);
        if (mirror) ex = head->width - ex;
        int ancEX = mirror ? (eyes->width - eyes->anchor_x) : eyes->anchor_x;
        int eyeX = headX + ex * scale - ancEX * scale;
        int eyeY = headY + ey * scale - eyes->anchor_y * scale;

        // Tail attachment on body
        int tx = body->tail_x.get(bodyF);
        int ty = body->tail_y.get(bodyF);
        if (mirror) tx = body->width - tx;
        int ancTX = mirror ? (tail->width - tail->anchor_x) : tail->anchor_x;
        int tailX = bodyX + tx * scale - ancTX * scale;
        int tailY = bodyY + ty * scale - tail->anchor_y * scale;

        // Draw order
        r.drawSpriteObj(tail, tailX, tailY, tailF, mirror, COLOR_WHITE, COLOR_CREAM, scale);

        if (p.head_first) {
            r.drawSpriteObj(head, headX, headY, headF, mirror, COLOR_WHITE, COLOR_CREAM, scale);
            r.drawSpriteObj(body, bodyX, bodyY, bodyF, mirror, COLOR_WHITE, COLOR_CREAM, scale);
        } else {
            r.drawSpriteObj(body, bodyX, bodyY, bodyF, mirror, COLOR_WHITE, COLOR_CREAM, scale);
            r.drawSpriteObj(head, headX, headY, headF, mirror, COLOR_WHITE, COLOR_CREAM, scale);
        }
        r.drawSpriteObj(eyes, eyeX, eyeY, eyeF, mirror, COLOR_WHITE, COLOR_CREAM, scale);

        // Draw behavior effects (bubbles, Z's, etc.)
        _drawBehavior(r, px, py, mirror);
    }

private:
    const char*      _poseName;
    const PoseEntry* _poseEntry;
    BaseBehavior*    _currentBehavior;

    PartAnim _body, _head, _eyes, _tail;

    static float _advCounter(float c, float dt, float speed, int totalFrames) {
        if (totalFrames <= 0) return 0;
        c += dt * speed;
        while (c >= totalFrames) c -= totalFrames;
        return c;
    }

    static int _frameIdx(const Sprite* s, float counter) {
        if (!s || s->frame_count == 0) return 0;
        int total = s->frame_count + s->extra_frames;
        int idx   = (int)counter % total;
        return (idx < s->frame_count) ? idx : 0;
    }

    // Bodies defined after IdleBehavior.h is included (below class)
    void _updateBehavior(float dt);
    void _drawBehavior(Renderer& r, int px, int py, bool mirror);
};

// ── Lazy-include of behaviors needed for makeIdleBehavior ────────────────
#include "behaviors/IdleBehavior.h"

inline BaseBehavior* CharacterEntity::makeIdleBehavior() {
    return new IdleBehavior(this);
}

inline void CharacterEntity::trigger(BaseBehavior* newBehavior) {
    if (_currentBehavior && _currentBehavior->active())
        _currentBehavior->stop(false);
    delete _currentBehavior;
    _currentBehavior = newBehavior;
    if (_currentBehavior) _currentBehavior->start();
}

inline void CharacterEntity::_updateBehavior(float dt) {
    if (_currentBehavior && context) {
        _currentBehavior->applyStatEffects(context, dt);
        _currentBehavior->update(dt);
    }
}

inline void CharacterEntity::_drawBehavior(Renderer& r, int px, int py, bool mirror) {
    if (_currentBehavior)
        _currentBehavior->draw(r, px, py, mirror);
}

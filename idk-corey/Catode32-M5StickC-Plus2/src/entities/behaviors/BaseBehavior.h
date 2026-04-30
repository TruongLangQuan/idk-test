#pragma once
// BaseBehavior.h - Abstract base class for all character behaviors

#include <Arduino.h>
#include "GameContext.h"

class Renderer;
class CharacterEntity;

// Stat effect entry: stat name + rate per second (or completion bonus)
struct StatEffect {
    const char* stat;
    float       rate;
};

class BaseBehavior {
public:
    explicit BaseBehavior(CharacterEntity* character)
        : _character(character), _active(false),
          _phase(nullptr), _phaseTimer(0.0f), _progress(0.0f),
          _poseBeforeName(nullptr), _onComplete(nullptr) {}

    virtual ~BaseBehavior() {}

    // ── Overridable class-level properties ────────────────────────────────
    virtual const char*    name()           const = 0;
    virtual const char*    triggerStat()    const { return nullptr; }
    virtual float          triggerThreshold() const { return 50.0f; }
    virtual bool           triggerBelow()   const { return true; }
    virtual int            priority()       const { return 50; }

    // Stat effects during behavior (override to return your array)
    virtual const StatEffect* statEffects(int* count) const { *count=0; return nullptr; }
    // Completion bonuses
    virtual const StatEffect* completionBonus(int* count) const { *count=0; return nullptr; }

    // ── Class-level trigger check ─────────────────────────────────────────
    virtual bool canTrigger(GameContext* ctx) const {
        if (!ctx || !triggerStat()) return false;
        float val = ctx->getStat(triggerStat());
        return triggerBelow() ? (val < triggerThreshold()) : (val > triggerThreshold());
    }

    // ── Accessors ─────────────────────────────────────────────────────────
    bool        active()   const { return _active; }
    float       progress() const { return _progress; }
    const char* phase()    const { return _phase; }

    // ── Lifecycle ─────────────────────────────────────────────────────────
    using CompleteCb = void(*)(bool completed, float progress, void* userData);

    virtual void start(CompleteCb cb = nullptr, void* userData = nullptr) {
        if (_active) return;
        _active       = true;
        _phaseTimer   = 0.0f;
        _progress     = 0.0f;
        _onComplete   = cb;
        _userData     = userData;
        _poseBeforeName = _getCurrentPoseName();
    }

    virtual void stop(bool completed = true) {
        if (!_active) return;
        _active    = false;
        _phase     = nullptr;
        _phaseTimer= 0.0f;

        if (completed && _poseBeforeName)
            _character->setPose(_poseBeforeName);
        _poseBeforeName = nullptr;

        CompleteCb cb = _onComplete;
        void*      ud = _userData;
        float      fp = _progress;
        _onComplete   = nullptr;
        _userData     = nullptr;

        if (cb) cb(completed, fp, ud);

        if (completed) {
            GameContext* ctx = _getContext();
            if (ctx) _applyCompletionBonus(ctx, fp);
            BaseBehavior* next = nextBehavior(ctx);
            _chainTo(next);
        }
    }

    virtual void update(float dt) {
        if (!_active) return;
        _phaseTimer += dt;
    }

    virtual void draw(Renderer& r, int charX, int charY, bool mirror = false) {}

    // Return the next behavior to chain to (nullptr = idle)
    virtual BaseBehavior* nextBehavior(GameContext* ctx) { return nullptr; }

    // Apply per-frame stat effects
    void applyStatEffects(GameContext* ctx, float dt) {
        if (!ctx) return;
        int count; const StatEffect* effects = statEffects(&count);
        for (int i = 0; i < count; i++)
            ctx->addStat(effects[i].stat, effects[i].rate * dt);
    }

protected:
    CharacterEntity* _character;
    bool             _active;
    const char*      _phase;
    float            _phaseTimer;
    float            _progress;
    const char*      _poseBeforeName;
    CompleteCb       _onComplete  = nullptr;
    void*            _userData    = nullptr;

    void _applyCompletionBonus(GameContext* ctx, float prog) {
        if (!ctx) return;
        int count; const StatEffect* bonus = completionBonus(&count);
        for (int i = 0; i < count; i++)
            ctx->addStat(bonus[i].stat, bonus[i].rate * prog);
    }

    // Platform-specific: get GameContext from character
    GameContext* _getContext();

    // Get current pose name from character
    const char* _getCurrentPoseName();

    // Chain to next behavior (install it on the character)
    void _chainTo(BaseBehavior* next);
};

// Forward-declare helpers implemented in Character.cpp
#include "entities/CharacterEntity.h"

inline GameContext* BaseBehavior::_getContext() {
    return _character ? _character->context : nullptr;
}

inline const char* BaseBehavior::_getCurrentPoseName() {
    return _character ? _character->poseName() : nullptr;
}

inline void BaseBehavior::_chainTo(BaseBehavior* next) {
    if (!_character) return;
    if (!next) {
        // Chain to idle
        next = _character->makeIdleBehavior();
    }
    _character->setCurrentBehavior(next);
    next->start();
}

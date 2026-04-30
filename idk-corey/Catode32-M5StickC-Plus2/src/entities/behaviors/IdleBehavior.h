#pragma once
#include "BaseBehavior.h"
#include <stdlib.h>

class IdleBehavior : public BaseBehavior {
public:
    IdleBehavior(CharacterEntity* c) : BaseBehavior(c),
        _timeUntilPoseChange(0.0f), _currentIdlePose(nullptr) {}

    const char* name() const override { return "idle"; }
    int         priority() const override { return 100; }

    static const StatEffect FX[4];
    const StatEffect* statEffects(int* n) const override { *n=4; return FX; }

    void start(CompleteCb cb=nullptr, void* ud=nullptr) override {
        if (_active) return;
        BaseBehavior::start(cb, ud);
        _phase = "idling";
        _pickNewPose();
    }

    void update(float dt) override {
        if (!_active) return;
        _phaseTimer           += dt;
        _timeUntilPoseChange  -= dt;
        if (_timeUntilPoseChange <= 0) _pickNewPose();
        if (_phaseTimer >= CHECK_INTERVAL) stop(true);
    }

    BaseBehavior* nextBehavior(GameContext* ctx) override;

private:
    static const float CHECK_INTERVAL;
    float       _timeUntilPoseChange;
    const char* _currentIdlePose;

    static const char* POSES[];
    static const int   POSE_COUNT;

    void _pickNewPose() {
        int idx = random(POSE_COUNT);
        _currentIdlePose = POSES[idx];
        if (_character) _character->setPose(_currentIdlePose);
        _timeUntilPoseChange = 10.0f + (random(20) * 1.0f);
    }
};

const float IdleBehavior::CHECK_INTERVAL = 15.0f;
const StatEffect IdleBehavior::FX[] = {
    {"curiosity",  0.10f},
    {"energy",    -0.10f},
    {"fullness",  -0.10f},
    {"affection", -0.05f},
};
const char* IdleBehavior::POSES[] = {
    "sitting.side.neutral",
    "sitting.side.happy",
    "sitting.side.aloof",
    "sitting.forward.neutral",
    "sitting.forward.happy",
    "sitting.forward.aloof",
    "standing.side.neutral",
    "standing.side.happy",
};
const int IdleBehavior::POSE_COUNT = 8;

// Must be below all behavior includes to avoid circular deps
// Call this from a .cpp or after all behaviors are included
#include "SleepingBehavior.h"
#include "NappingBehavior.h"
#include "PlayingBehavior.h"
#include "ZoomiesBehavior.h"
#include "VocalizingBehavior.h"
#include "InvestigatingBehavior.h"
#include "ObservingBehavior.h"
#include "StretchingBehavior.h"
#include "SelfGroomingBehavior.h"
#include "LoungeingBehavior.h"
#include "KneadingBehavior.h"

inline BaseBehavior* IdleBehavior::nextBehavior(GameContext* ctx) {
    if (!ctx) return nullptr;
    // Try all auto-triggerable behaviors
    BaseBehavior* candidates[16];
    int           prio[16];
    int           count = 0;
    CharacterEntity* ch = _character;

    #define TRY(CLS) { \
        CLS* b = new CLS(ch); \
        if (b->canTrigger(ctx)) { candidates[count]=b; prio[count]=b->priority(); count++; } \
        else delete b; \
    }
    TRY(SleepingBehavior)
    TRY(NappingBehavior)
    TRY(ZoomiesBehavior)
    TRY(VocalizingBehavior)
    TRY(PlayingBehavior)
    TRY(InvestigatingBehavior)
    TRY(ObservingBehavior)
    TRY(SelfGroomingBehavior)
    TRY(StretchingBehavior)
    TRY(LoungeingBehavior)
    #undef TRY

    if (count == 0) return nullptr;

    // Find minimum priority (highest importance)
    int best = prio[0];
    for (int i = 1; i < count; i++) if (prio[i] < best) best = prio[i];

    // Collect all with best priority
    BaseBehavior* top[16]; int topCount = 0;
    for (int i = 0; i < count; i++) {
        if (prio[i] == best) top[topCount++] = candidates[i];
        else delete candidates[i];
    }

    // Pick randomly from the top candidates
    int chosen = random(topCount);
    BaseBehavior* result = top[chosen];
    for (int i = 0; i < topCount; i++) if (i != chosen) delete top[i];
    return result;
}

inline BaseBehavior* SleepingBehavior::nextBehavior(GameContext*) {
    return new StretchingBehavior(_character);
}
inline BaseBehavior* StretchingBehavior::nextBehavior(GameContext*) {
    if (random(100) < 20) return new KneadingBehavior(_character);
    return nullptr; // -> idle
}
inline BaseBehavior* KneadingBehavior::nextBehavior(GameContext*) {
    return nullptr; // -> idle
}

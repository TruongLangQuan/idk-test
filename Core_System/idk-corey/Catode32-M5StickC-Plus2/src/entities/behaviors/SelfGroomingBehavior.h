#pragma once
#include "BaseBehavior.h"

class SelfGroomingBehavior : public BaseBehavior {
public:
    SelfGroomingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "self_grooming"; }
    int   priority() const override { return 45; }

    bool canTrigger(GameContext* ctx) const override {
        if(!ctx)return false;
        return ctx->getStat("cleanliness")<40.0f && ctx->getStat("energy")>30.0f;
    }

    static const StatEffect FX[4];
    static const StatEffect BONUS[4];
    const StatEffect* statEffects(int* n) const override { *n=4; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=4; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="preparing";
        if(_character) _character->setPose("sitting.forward.neutral");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"preparing")==0){
            if(_phaseTimer>=1.0f){_phase="grooming";_phaseTimer=0;if(_character)_character->setPose("sitting.side.aloof");}
        } else if(strcmp(_phase,"grooming")==0){
            _progress=min(1.0f,_phaseTimer/12.0f);
            if(_phaseTimer>=12.0f){_phase="finishing";_phaseTimer=0;if(_character)_character->setPose("sitting.side.happy");}
        } else if(strcmp(_phase,"finishing")==0){
            if(_phaseTimer>=1.5f) stop(true);
        }
    }
};
const StatEffect SelfGroomingBehavior::FX[]    = {{"cleanliness",0.5f},{"energy",-0.2f},{"comfort",-0.1f},{"focus",-0.2f}};
const StatEffect SelfGroomingBehavior::BONUS[] = {{"cleanliness",15.0f},{"grace",5.0f},{"sociability",3.0f},{"affection",2.0f}};

#pragma once
#include "BaseBehavior.h"

class ZoomiesBehavior : public BaseBehavior {
public:
    ZoomiesBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "zoomies"; }
    int   priority() const override { return 25; }

    bool canTrigger(GameContext* ctx) const override {
        if(!ctx)return false;
        return ctx->getStat("energy")>70.0f && ctx->getStat("playfulness")>70.0f;
    }

    static const StatEffect FX[2];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="winding_up";
        if(_character) _character->setPose("sitting.side.aloof");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"winding_up")==0){
            if(_phaseTimer>=1.0f){_phase="zooming";_phaseTimer=0;if(_character)_character->setPose("standing.side.happy");}
        } else if(strcmp(_phase,"zooming")==0){
            _progress=min(1.0f,_phaseTimer/10.0f);
            if(_phaseTimer>=10.0f){_phase="collapsing";_phaseTimer=0;if(_character)_character->setPose("sitting.side.neutral");}
        } else if(strcmp(_phase,"collapsing")==0){
            if(_phaseTimer>=2.0f) stop(true);
        }
    }
};
const StatEffect ZoomiesBehavior::FX[]    = {{"energy",-2.0f},{"playfulness",-3.0f}};
const StatEffect ZoomiesBehavior::BONUS[] = {{"energy",-10.0f},{"playfulness",-15.0f}};

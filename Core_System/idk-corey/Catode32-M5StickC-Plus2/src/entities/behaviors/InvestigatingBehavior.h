#pragma once
#include "BaseBehavior.h"

class InvestigatingBehavior : public BaseBehavior {
public:
    InvestigatingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "investigating"; }
    const char* triggerStat() const override { return "curiosity"; }
    float triggerThreshold() const override { return 70.0f; }
    bool  triggerBelow()     const override { return false; }
    int   priority()         const override { return 40; }

    static const StatEffect FX[1];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=1; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="approaching";
        if(_character) _character->setPose("standing.side.neutral");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"approaching")==0){
            if(_phaseTimer>=1.5f){_phase="sniffing";_phaseTimer=0;if(_character)_character->setPose("leaning_forward.side.neutral");}
        } else if(strcmp(_phase,"sniffing")==0){
            if(_phaseTimer>=4.0f){_phase="reacting";_phaseTimer=0;if(_character)_character->setPose("sitting.side.happy");}
        } else if(strcmp(_phase,"reacting")==0){
            if(_phaseTimer>=1.5f) stop(true);
        }
    }
};
const StatEffect InvestigatingBehavior::FX[]    = {{"curiosity",-1.0f}};
const StatEffect InvestigatingBehavior::BONUS[] = {{"curiosity",-20.0f},{"fulfillment",5.0f}};

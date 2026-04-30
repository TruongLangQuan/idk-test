#pragma once
#include "BaseBehavior.h"

class ObservingBehavior : public BaseBehavior {
public:
    ObservingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "observing"; }
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
        _phase="noticing";
        if(_character) _character->setPose("sitting.side.aloof");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"noticing")==0){
            if(_phaseTimer>=1.0f){_phase="watching";_phaseTimer=0;}
        } else if(strcmp(_phase,"watching")==0){
            if(_phaseTimer>=10.0f){_phase="losing_interest";_phaseTimer=0;if(_character)_character->setPose("sitting.side.neutral");}
        } else if(strcmp(_phase,"losing_interest")==0){
            if(_phaseTimer>=2.0f) stop(true);
        }
    }
};
const StatEffect ObservingBehavior::FX[]    = {{"curiosity",-0.5f}};
const StatEffect ObservingBehavior::BONUS[] = {{"curiosity",-10.0f},{"fulfillment",3.0f}};

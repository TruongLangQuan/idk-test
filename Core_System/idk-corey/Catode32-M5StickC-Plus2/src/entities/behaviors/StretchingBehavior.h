#pragma once
#include "BaseBehavior.h"

class StretchingBehavior : public BaseBehavior {
public:
    StretchingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "stretching"; }
    const char* triggerStat() const override { return "comfort"; }
    float triggerThreshold() const override { return 40.0f; }
    bool  triggerBelow()     const override { return true; }
    int   priority()         const override { return 50; }

    static const StatEffect FX[1];
    static const StatEffect BONUS[1];
    const StatEffect* statEffects(int* n) const override { *n=1; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=1; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="preparing";
        if(_character) _character->setPose("standing.side.neutral");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"preparing")==0){
            if(_phaseTimer>=0.5f){_phase="stretching";_phaseTimer=0;if(_character)_character->setPose("leaning_forward.side.neutral");}
        } else if(strcmp(_phase,"stretching")==0){
            if(_phaseTimer>=3.0f){_phase="relaxing";_phaseTimer=0;if(_character)_character->setPose("sitting.side.happy");}
        } else if(strcmp(_phase,"relaxing")==0){
            if(_phaseTimer>=1.5f) stop(true);
        }
    }

    BaseBehavior* nextBehavior(GameContext*) override;
};
const StatEffect StretchingBehavior::FX[]    = {{"comfort",1.5f}};
const StatEffect StretchingBehavior::BONUS[] = {{"comfort",15.0f}};

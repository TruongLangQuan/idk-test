#pragma once
#include "BaseBehavior.h"

class KneadingBehavior : public BaseBehavior {
public:
    KneadingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "kneading"; }
    int   priority() const override { return 50; }

    static const StatEffect FX[2];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="kneading";
        if(_character) _character->setPose("sitting.forward.happy");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"kneading")==0){
            if(_phaseTimer>=8.0f){_phase="settling";_phaseTimer=0;if(_character)_character->setPose("sitting.side.neutral");}
        } else if(strcmp(_phase,"settling")==0){
            if(_phaseTimer>=2.0f) stop(true);
        }
    }

    BaseBehavior* nextBehavior(GameContext*) override;
};
const StatEffect KneadingBehavior::FX[]    = {{"serenity",0.3f},{"comfort",0.2f}};
const StatEffect KneadingBehavior::BONUS[] = {{"serenity",5.0f},{"comfort",3.0f}};

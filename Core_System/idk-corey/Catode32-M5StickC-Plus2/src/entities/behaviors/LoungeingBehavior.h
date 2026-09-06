#pragma once
#include "BaseBehavior.h"

class LoungeingBehavior : public BaseBehavior {
public:
    LoungeingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "lounging"; }
    int   priority() const override { return 90; }
    bool  canTrigger(GameContext*) const override { return true; }

    static const StatEffect FX[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="settling";
        static const char* poses[]={"sleeping.side.sploot","sitting.side.aloof"};
        if(_character) _character->setPose(poses[random(2)]);
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"settling")==0){
            if(_phaseTimer>=1.0f){_phase="lounging";_phaseTimer=0;}
        } else if(strcmp(_phase,"lounging")==0){
            _progress=min(1.0f,_phaseTimer/20.0f);
            if(_phaseTimer>=20.0f){_phase="rousing";_phaseTimer=0;}
        } else if(strcmp(_phase,"rousing")==0){
            if(_phaseTimer>=1.5f) stop(true);
        }
    }
};
const StatEffect LoungeingBehavior::FX[] = {{"comfort",-0.1f},{"energy",-0.05f}};

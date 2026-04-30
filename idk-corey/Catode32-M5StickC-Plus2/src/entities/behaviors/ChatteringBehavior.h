#pragma once
#include "BaseBehavior.h"

class ChatteringBehavior : public BaseBehavior {
public:
    ChatteringBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "chattering"; }
    int   priority() const override { return 40; }

    static const StatEffect FX[2];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="chattering";
        if(_character) _character->setPose("sitting.side.aloof");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"chattering")==0){
            if(_phaseTimer>=4.0f){_phase="settling";_phaseTimer=0;if(_character)_character->setPose("sitting.side.neutral");}
        } else if(strcmp(_phase,"settling")==0){
            if(_phaseTimer>=1.0f) stop(true);
        }
    }

    void draw(Renderer& r, int cx, int cy, bool mirror) override {
        if(!_active||strcmp(_phase,"chattering")!=0)return;
        // rapid flicker effect
        if((int)(_phaseTimer*8)%2==0)
            r.drawText("...",cx+(mirror?5:-20),cy-20,COLOR_WHITE,COLOR_BLACK,1);
    }
};
const StatEffect ChatteringBehavior::FX[]    = {{"curiosity",-0.5f},{"playfulness",0.5f}};
const StatEffect ChatteringBehavior::BONUS[] = {{"curiosity",-5.0f},{"playfulness",5.0f}};

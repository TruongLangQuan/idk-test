#pragma once
#include "BaseBehavior.h"

class BeingGroomedBehavior : public BaseBehavior {
public:
    BeingGroomedBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "being_groomed"; }
    int   priority() const override { return 5; }

    static const StatEffect FX[4];
    static const StatEffect BONUS[4];
    const StatEffect* statEffects(int* n) const override { *n=4; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=4; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="accepting";
        if(_character) _character->setPose("sitting.forward.neutral");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"accepting")==0){
            if(_phaseTimer>=1.0f){_phase="enjoying";_phaseTimer=0;if(_character)_character->setPose("sitting.forward.happy");}
        } else if(strcmp(_phase,"enjoying")==0){
            _progress=min(1.0f,_phaseTimer/8.0f);
            if(_phaseTimer>=8.0f){_phase="satisfied";_phaseTimer=0;if(_character)_character->setPose("sitting.side.happy");}
        } else if(strcmp(_phase,"satisfied")==0){
            if(_phaseTimer>=1.5f) stop(true);
        }
    }

    void draw(Renderer& r, int cx, int cy, bool mirror) override {
        if(!_active||strcmp(_phase,"enjoying")!=0)return;
        r.drawText("<3",cx+(mirror?10:-20),cy-28,COLOR_RED,COLOR_BLACK,1);
    }
};
const StatEffect BeingGroomedBehavior::FX[]    = {{"cleanliness",0.5f},{"affection",0.3f},{"patience",0.2f},{"focus",-0.3f}};
const StatEffect BeingGroomedBehavior::BONUS[] = {{"cleanliness",15.0f},{"affection",8.0f},{"grace",3.0f},{"sociability",2.0f}};

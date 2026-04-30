#pragma once
#include "BaseBehavior.h"

class PlayingBehavior : public BaseBehavior {
public:
    PlayingBehavior(CharacterEntity* c) : BaseBehavior(c), _bubble(nullptr) {}
    const char* name() const override { return "playing"; }
    const char* triggerStat() const override { return "playfulness"; }
    float triggerThreshold() const override { return 70.0f; }
    bool  triggerBelow()     const override { return false; }
    int   priority()         const override { return 30; }

    static const StatEffect FX[2];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(const char* trigger=nullptr, CompleteCb cb=nullptr, void* ud=nullptr) {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="excited";
        _bubble=nullptr;
        if(_character) _character->setPose("sitting.side.happy");
        if(trigger&&_character&&_character->context){
            _bubble="!";
            _character->context->addStat("playfulness",15.0f);
            _character->context->addStat("energy",-5.0f);
        }
    }
    void start(CompleteCb cb=nullptr,void* ud=nullptr) override { start(nullptr,cb,ud); }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"excited")==0){
            if(_phaseTimer>=1.0f){_phase="playing";_phaseTimer=0;_bubble=nullptr;if(_character)_character->setPose("standing.side.happy");}
        } else if(strcmp(_phase,"playing")==0){
            _progress=min(1.0f,_phaseTimer/5.0f);
            if(_phaseTimer>=5.0f){_phase="tired";_phaseTimer=0;if(_character)_character->setPose("sitting.side.neutral");}
        } else if(strcmp(_phase,"tired")==0){
            if(_phaseTimer>=1.0f) stop(true);
        }
    }

    void draw(Renderer& r, int cx, int cy, bool mirror) override {
        if(!_active||!_bubble||strcmp(_phase,"excited")!=0)return;
        r.drawText(_bubble,cx+(mirror?10:-12),cy-28,COLOR_YELLOW,COLOR_BLACK,1);
    }

private:
    const char* _bubble;
};
const StatEffect PlayingBehavior::FX[]    = {{"playfulness",-2.0f},{"energy",-0.5f}};
const StatEffect PlayingBehavior::BONUS[] = {{"playfulness",-25.0f},{"fulfillment",10.0f}};

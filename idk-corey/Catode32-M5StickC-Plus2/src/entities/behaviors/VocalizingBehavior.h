#pragma once
#include "BaseBehavior.h"

class VocalizingBehavior : public BaseBehavior {
public:
    VocalizingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "vocalizing"; }
    int   priority() const override { return 25; }

    bool canTrigger(GameContext* ctx) const override {
        if(!ctx)return false;
        return ctx->getStat("energy")>60.0f && ctx->getStat("playfulness")>60.0f;
    }

    static const StatEffect FX[2];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="winding_up";
        if(_character) _character->setPose("sitting.forward.aloof");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"winding_up")==0){
            if(_phaseTimer>=0.5f){_phase="vocalizing";_phaseTimer=0;if(_character)_character->setPose("sitting.forward.happy");}
        } else if(strcmp(_phase,"vocalizing")==0){
            if(_phaseTimer>=4.0f){_phase="settling";_phaseTimer=0;if(_character)_character->setPose("sitting.side.neutral");}
        } else if(strcmp(_phase,"settling")==0){
            if(_phaseTimer>=1.0f) stop(true);
        }
    }

    void draw(Renderer& r, int cx, int cy, bool mirror) override {
        if(!_active||strcmp(_phase,"vocalizing")!=0)return;
        r.drawText("~",cx+(mirror?15:-15),cy-30,COLOR_WHITE,COLOR_BLACK,1);
    }
};
const StatEffect VocalizingBehavior::FX[]    = {{"energy",-1.0f},{"playfulness",-1.5f}};
const StatEffect VocalizingBehavior::BONUS[] = {{"energy",-5.0f},{"playfulness",-8.0f}};

#pragma once
#include "BaseBehavior.h"
#include <math.h>

class NappingBehavior : public BaseBehavior {
public:
    NappingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "napping"; }
    const char* triggerStat() const override { return "energy"; }
    float triggerThreshold() const override { return 45.0f; }
    bool  triggerBelow()     const override { return true; }
    int   priority()         const override { return 20; }

    static const StatEffect FX[2];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="settling";
        if(_character) _character->setPose("sleeping.side.modest");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"settling")==0){
            if(_phaseTimer>=1.5f){_phase="napping";_phaseTimer=0;}
        } else if(strcmp(_phase,"napping")==0){
            _progress=min(1.0f,_phaseTimer/20.0f);
            if(_phaseTimer>=20.0f){_phase="waking";_phaseTimer=0;}
        } else if(strcmp(_phase,"waking")==0){
            if(_phaseTimer>=2.0f) stop(true);
        }
    }

    void draw(Renderer& r, int cx, int cy, bool mirror) override {
        if(!_active || strcmp(_phase,"napping")!=0) return;
        int i=(int)(_phaseTimer*1.5f)%3;
        float wave=sin(_phaseTimer*2.5f)*2.5f;
        int zx=cx+(mirror?15:-15)+i*6;
        int zy=(int)(cy-25+wave);
        r.drawText("z",zx,zy,COLOR_WHITE,COLOR_BLACK,1);
    }
};
const StatEffect NappingBehavior::FX[]    = {{"energy",1.0f},{"focus",0.5f}};
const StatEffect NappingBehavior::BONUS[] = {{"energy",5.0f},{"focus",5.0f}};

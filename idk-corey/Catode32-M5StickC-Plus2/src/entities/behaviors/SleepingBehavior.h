#pragma once
#include "BaseBehavior.h"
#include <math.h>

class SleepingBehavior : public BaseBehavior {
public:
    SleepingBehavior(CharacterEntity* c) : BaseBehavior(c), _sleepPose(nullptr) {}
    const char* name() const override { return "sleeping"; }
    const char* triggerStat() const override { return "energy"; }
    float triggerThreshold() const override { return 30.0f; }
    bool  triggerBelow()     const override { return true; }
    int   priority()         const override { return 10; }

    static const StatEffect FX[2];
    static const StatEffect BONUS[2];
    const StatEffect* statEffects(int* n) const override { *n=2; return FX; }
    const StatEffect* completionBonus(int* n) const override { *n=2; return BONUS; }

    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        static const char* poses[]={"sleeping.side.sploot","sleeping.side.modest","sleeping.side.crossed"};
        _sleepPose=poses[random(3)];
        _phase="considering";
        if(_character) _character->setPose("sitting.side.looking_down");
    }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"considering")==0){
            if(_phaseTimer>=1.0f){_phase="settling";_phaseTimer=0;if(_character)_character->setPose("leaning_forward.side.neutral");}
        } else if(strcmp(_phase,"settling")==0){
            if(_phaseTimer>=2.5f){_phase="sleeping";_phaseTimer=0;if(_character)_character->setPose(_sleepPose);}
        } else if(strcmp(_phase,"sleeping")==0){
            _progress=min(1.0f,_phaseTimer/45.0f);
            if(_phaseTimer>=45.0f){_phase="waking";_phaseTimer=0;}
        } else if(strcmp(_phase,"waking")==0){
            if(_phaseTimer>=5.0f) stop(true);
        }
    }

    void draw(Renderer& r, int cx, int cy, bool mirror) override {
        if(!_active || strcmp(_phase,"sleeping")!=0) return;
        for(int i=0;i<4;i++){
            float wave=sin(_phaseTimer*3.0f - i*0.8f)*3.0f;
            int zx=cx+(mirror?20:-20)+i*8;
            int zy=(int)(cy-35+i*-2+wave);
            char buf[2]="z";
            r.drawText(buf,zx,zy,COLOR_WHITE,COLOR_BLACK,1);
        }
    }

    BaseBehavior* nextBehavior(GameContext*) override;

private:
    const char* _sleepPose;
};
const StatEffect SleepingBehavior::FX[]    = {{"energy",2.0f},{"comfort",0.2f}};
const StatEffect SleepingBehavior::BONUS[] = {{"energy",15.0f},{"comfort",10.0f}};

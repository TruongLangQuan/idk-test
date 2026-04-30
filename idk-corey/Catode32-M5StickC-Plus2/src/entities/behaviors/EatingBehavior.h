#pragma once
#include "BaseBehavior.h"
#include "Renderer.h"
#include "assets/items_assets.h"

class EatingBehavior : public BaseBehavior {
public:
    EatingBehavior(CharacterEntity* c)
        : BaseBehavior(c), _bowlSprite(nullptr), _bowlFrame(0.0f),
          _bowlYProg(0.0f), _mealType(nullptr) {}

    const char* name() const override { return "eating"; }
    int   priority() const override { return 10; }

    struct MealStat { const char* stat; float delta; };

    void start(const Sprite* bowl, const char* mealType,
               CompleteCb cb=nullptr, void* ud=nullptr) {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="lowering";
        _bowlSprite=bowl;
        _bowlFrame=0.0f;
        _bowlYProg=0.0f;
        _mealType=mealType;
        if(_character) _character->setPose("standing.side.happy");
    }
    void start(CompleteCb cb=nullptr,void* ud=nullptr) override {
        start(nullptr,nullptr,cb,ud);
    }

    void stop(bool completed=true) override {
        if(!_active)return;
        _bowlYProg=1.0f;
        if(completed) _applyMealStats();
        _bowlSprite=nullptr; _mealType=nullptr;
        BaseBehavior::stop(completed);
    }

    void update(float dt) override {
        if(!_active||!_bowlSprite)return;
        _phaseTimer+=dt;
        if(strcmp(_phase,"lowering")==0){
            _bowlYProg=min(1.0f,_phaseTimer/0.5f);
            if(_bowlYProg>=1.0f){_phase="pre_eating";_phaseTimer=0;if(_character)_character->setPose("leaning_forward.side.neutral");}
        } else if(strcmp(_phase,"pre_eating")==0){
            if(_phaseTimer>=1.5f){_phase="eating";_phaseTimer=0;if(_character)_character->setPose("leaning_forward.side.eating");}
        } else if(strcmp(_phase,"eating")==0){
            int nf=_bowlSprite->frame_count;
            _bowlFrame+=dt*0.4f;
            if(_bowlFrame>=nf){_phase="post_eating";_phaseTimer=0;if(_character)_character->setPose("leaning_forward.side.neutral");}
        } else if(strcmp(_phase,"post_eating")==0){
            if(_phaseTimer>=1.5f) stop(true);
        }
    }

    int  getBowlFrame() const { return min((int)_bowlFrame, _bowlSprite?_bowlSprite->frame_count-1:0); }
    float getBowlYProg() const { return _bowlYProg; }
    const Sprite* getBowlSprite() const { return _bowlSprite; }

    void getBowlPosition(float charX, float charY, bool mirror,
                         int& bx, int& by) const {
        int bw=_bowlSprite?_bowlSprite->width:22;
        int bh=_bowlSprite?_bowlSprite->height:8;
        int groundY=(int)charY - bh;
        int startY=groundY-40;
        by=(int)(startY+(groundY-startY)*_bowlYProg);
        bx=mirror ? (int)charX+30-bw/2 : (int)charX-30-bw/2;
    }

private:
    const Sprite* _bowlSprite;
    float         _bowlFrame;
    float         _bowlYProg;
    const char*   _mealType;

    void _applyMealStats() {
        GameContext* ctx=_getContext();
        if(!ctx||!_mealType)return;
        if(strcmp(_mealType,"chicken")==0){ ctx->addStat("fullness",30); ctx->addStat("energy",10); }
        else if(strcmp(_mealType,"fish")==0){ ctx->addStat("fullness",25); ctx->addStat("affection",5); }
        else { ctx->addStat("fullness",20); }
    }
};

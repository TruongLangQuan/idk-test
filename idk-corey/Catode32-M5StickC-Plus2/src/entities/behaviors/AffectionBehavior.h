#pragma once
#include "BaseBehavior.h"

class AffectionBehavior : public BaseBehavior {
public:
    AffectionBehavior(CharacterEntity* c) : BaseBehavior(c), _duration(2.0f) {}
    const char* name() const override { return "affection"; }
    int   priority() const override { return 5; }

    // variant: "kiss"=bigger stat bonus longer; "pets"=shorter smaller bonus
    void start(const char* variant=nullptr, CompleteCb cb=nullptr, void* ud=nullptr) {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="reacting";
        if(variant && strcmp(variant,"kiss")==0){
            _duration=2.5f;
            if(_character) _character->setPose("sitting.side.happy");
            if(_character&&_character->context) _character->context->addStat("affection",10.0f);
        } else {
            _duration=2.0f;
            if(_character) _character->setPose("sitting.forward.happy");
            if(_character&&_character->context) _character->context->addStat("affection",5.0f);
        }
    }
    void start(CompleteCb cb=nullptr,void* ud=nullptr) override { start(nullptr,cb,ud); }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        _progress=min(1.0f,_phaseTimer/_duration);
        if(_phaseTimer>=_duration) stop(true);
    }

    void draw(Renderer& r, int cx, int cy, bool mirror) override {
        if(!_active)return;
        r.drawText("<3",cx+(mirror?10:-20),cy-30,COLOR_RED,COLOR_BLACK,1);
    }

private:
    float _duration;
};

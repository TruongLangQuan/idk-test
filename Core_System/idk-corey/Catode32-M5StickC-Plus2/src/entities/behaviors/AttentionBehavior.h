#pragma once
#include "BaseBehavior.h"

class AttentionBehavior : public BaseBehavior {
public:
    AttentionBehavior(CharacterEntity* c) : BaseBehavior(c), _duration(1.5f) {}
    const char* name() const override { return "attention"; }
    int   priority() const override { return 5; }

    void start(const char* variant=nullptr, CompleteCb cb=nullptr, void* ud=nullptr) {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="reacting";
        if(variant && strcmp(variant,"point_bird")==0){
            _duration=2.0f;
            if(_character) _character->setPose("sitting.side.aloof");
            if(_character&&_character->context) _character->context->addStat("curiosity",10.0f);
        } else { // psst
            _duration=1.5f;
            if(_character) _character->setPose("sitting.forward.aloof");
            if(_character&&_character->context) _character->context->addStat("curiosity",3.0f);
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
        r.drawText("?",cx+(mirror?10:-12),cy-30,COLOR_YELLOW,COLOR_BLACK,1);
    }

private:
    float _duration;
};

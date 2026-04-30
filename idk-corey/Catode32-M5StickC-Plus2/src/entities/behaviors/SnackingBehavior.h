#pragma once
#include "BaseBehavior.h"

class SnackingBehavior : public BaseBehavior {
public:
    SnackingBehavior(CharacterEntity* c) : BaseBehavior(c) {}
    const char* name() const override { return "snacking"; }
    int   priority() const override { return 5; }

    void start(const char* variant=nullptr, CompleteCb cb=nullptr, void* ud=nullptr) {
        if(_active)return;
        BaseBehavior::start(cb,ud);
        _phase="reacting";
        if(_character) _character->setPose("sitting.forward.happy");
        if(_character&&_character->context){
            if(variant&&strcmp(variant,"treat")==0){
                _character->context->addStat("fullness",5.0f);
                _character->context->addStat("affection",3.0f);
            } else {
                _character->context->addStat("fullness",10.0f);
            }
        }
    }
    void start(CompleteCb cb=nullptr,void* ud=nullptr) override { start(nullptr,cb,ud); }

    void update(float dt) override {
        if(!_active)return;
        _phaseTimer+=dt;
        if(_phaseTimer>=2.0f) stop(true);
    }
};

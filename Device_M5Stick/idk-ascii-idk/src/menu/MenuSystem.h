#pragma once

#include "engine/ASCIIRenderer.h"
#include <vector>
#include <string>

namespace idk {

enum class MenuState {
    CategorySelect,
    AnimationSelect,
    RunningAnimation
};

struct MenuCategory {
    std::string name;
    std::vector<std::string> animations;
};

class MenuSystem {
public:
    MenuSystem();
    
    void init();
    void update(ASCIIRenderer& r);
    
    bool isAnimationSelected() const { return _state == MenuState::RunningAnimation; }
    int getSelectedCategory() const { return _categoryIdx; }
    int getSelectedAnimation() const { return _animIdx; }
    
    void exitAnimation() { _state = MenuState::AnimationSelect; }

private:
    void handleInput();
    void draw(ASCIIRenderer& r);
    void drawBattery(ASCIIRenderer& r);

    MenuState _state = MenuState::CategorySelect;
    int _categoryIdx = 0;
    int _animIdx = 0;
    
    std::vector<MenuCategory> _categories;
};

} // namespace idk

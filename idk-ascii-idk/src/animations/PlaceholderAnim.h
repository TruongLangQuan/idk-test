#pragma once

#include "../engine/IAnimation.h"

namespace idk {

class PlaceholderAnim : public IAnimation {
public:
    void init(ASCIIRenderer& r) override {
        // Red background for the whole grid
        r.clear(' ', TFT_WHITE, 0xF800); 
    }
    
    void update(ASCIIRenderer& r, float dtMs) override {
        // Blinking text to indicate it's running
        if (((int)millis() / 500) % 2 == 0) {
            r.print(10, 7, " ANIMATION NOT ", TFT_YELLOW, 0xF800);
            r.print(10, 8, " YET AVAILABLE ", TFT_YELLOW, 0xF800);
        } else {
            r.print(10, 7, " ANIMATION NOT ", TFT_WHITE, 0xF800);
            r.print(10, 8, " YET AVAILABLE ", TFT_WHITE, 0xF800);
        }
    }
    
    const char* name() const override { return "Coming Soon"; }
};

} // namespace idk

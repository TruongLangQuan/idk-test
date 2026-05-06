#pragma once

#include "ASCIIRenderer.h"

namespace idk {

class IAnimation {
public:
    virtual ~IAnimation() = default;

    // Called once when animation is selected
    virtual void init(ASCIIRenderer& r) = 0;

    // Called every frame
    virtual void update(ASCIIRenderer& r, float dtMs) = 0;

    // Return the name for the menu
    virtual const char* name() const = 0;
};

} // namespace idk

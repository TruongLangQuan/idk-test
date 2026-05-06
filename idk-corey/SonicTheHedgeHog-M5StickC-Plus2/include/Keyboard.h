#pragma once
#include "Settings.h"

class Keyboard {
  public:
    static bool isKeyPressedRight();  
    static bool isKeyPressedLeft();
    static bool isKeyPressedUp();
    static bool isKeyPressedDown();
    static bool isKeyPressedAction();  
	static bool isKeyPressedSpindash();
};
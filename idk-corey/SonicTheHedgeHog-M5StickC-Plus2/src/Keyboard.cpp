#include "Keyboard.h"

extern UNIT_JOYC joyc;

bool Keyboard::isKeyPressedRight() {
    #ifdef CORE
    uint16_t y = joyc.getADCValue(1);
    return (y < 1600);
    #endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_RIGHT);
	#endif
}

bool Keyboard::isKeyPressedLeft() {
    #ifdef CORE
    uint16_t y = joyc.getADCValue(1);
    return (y > 3000);
    #endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_LEFT);
	#endif
}

bool Keyboard::isKeyPressedDown() {
    #ifdef CORE
    uint16_t x = joyc.getADCValue(0);
    return (x < 1200);
    #endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_DOWN);
	#endif
}

bool Keyboard::isKeyPressedAction() {
    #ifdef CORE
    return (joyc.getButtonStatus() == 0);
    #endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_ACTION);
	#endif
}  

bool Keyboard::isKeyPressedSpindash() {
	#ifdef CORE
	return StickCP2.BtnA.isPressed() && (joyc.getButtonStatus() == 0);
	#endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_SPINDASH_PC);
	#endif
}

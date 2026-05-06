#include "Keyboard.h"

// 5-way tactile switch pins
#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

bool Keyboard::isKeyPressedRight() {
    #ifdef CORE
    return (digitalRead(PIN_RIGHT) == LOW);
    #endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_RIGHT_PC);
	#endif
}

bool Keyboard::isKeyPressedLeft() {
	#ifdef CORE
    return (digitalRead(PIN_LEFT) == LOW);
	#endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_LEFT_PC);
	#endif
}

bool Keyboard::isKeyPressedUp() {
	#ifdef CORE
    return (digitalRead(PIN_UP) == LOW);
	#endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_UP_PC);
	#endif
}

bool Keyboard::isKeyPressedDown() {
	#ifdef CORE
    return (digitalRead(PIN_DOWN) == LOW);
	#endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_DOWN_PC);
	#endif
}

bool Keyboard::isKeyPressedAction() {
	#ifdef CORE
	return StickCP2.BtnA.isPressed() || (digitalRead(PIN_CENTER) == LOW);
	#endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_ACTION_PC);
	#endif
}  

bool Keyboard::isKeyPressedSpindash() {
	#ifdef CORE
	return StickCP2.BtnA.isPressed() && (digitalRead(PIN_DOWN) == LOW);
	#endif

	#ifdef PC
	return sf::Keyboard::isKeyPressed(KEY_SPINDASH_PC);
	#endif
}

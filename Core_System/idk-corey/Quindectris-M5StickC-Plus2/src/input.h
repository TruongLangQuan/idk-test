// input.h - Input handling for M5StickC Plus2 with 5-way tactile switch
#ifndef INPUT_H
#define INPUT_H

#include "config.h"
#include <M5StickCPlus2.h>

#define PIN_UP 32
#define PIN_DOWN 33
#define PIN_LEFT 25
#define PIN_RIGHT 26
#define PIN_CENTER 0

struct ButtonState {
  bool up;
  bool down;
  bool left;
  bool right;
  bool btnA;
  bool btnB;
  bool joyBtn;
  bool upPressed;
  bool downPressed;
  bool leftPressed;
  bool rightPressed;
  bool btnAPressed;
  bool btnBPressed;
  bool joyBtnPressed;
};

extern ButtonState buttons;

void initInput();
void updateInput();
bool isButtonPressed(int button);
bool wasButtonJustPressed(int button);

#endif

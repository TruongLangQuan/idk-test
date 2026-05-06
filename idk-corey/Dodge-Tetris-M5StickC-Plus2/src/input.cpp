// input.cpp - Input implementation for M5StickC Plus2
#include "input.h"

ButtonState buttons;
ButtonState lastButtons;

void initInput() {
  M5.begin();
  
  // Initialize 5-way switch pins
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_CENTER, INPUT_PULLUP);
  
  // Initialize button states
  buttons = {false, false, false, false, false, false, false, false, false, false, false, false, false, false};
  lastButtons = buttons;
}

void updateInput() {
  M5.update();
  
  // Store previous state
  lastButtons = buttons;
  
  // Read 5-way tactile switch
  // Mapping for Landscape 3: 32=UP, 33=DOWN, 25=LEFT, 26=RIGHT
  buttons.up = (digitalRead(PIN_UP) == LOW);
  buttons.down = (digitalRead(PIN_DOWN) == LOW);
  buttons.left = (digitalRead(PIN_LEFT) == LOW);
  buttons.right = (digitalRead(PIN_RIGHT) == LOW);
  
  // Read M5StickC buttons
  buttons.btnA = M5.BtnA.isPressed();
  buttons.btnB = M5.BtnB.isPressed();
  buttons.joyBtn = (digitalRead(PIN_CENTER) == LOW);
  
  // Detect button press events (just pressed)
  buttons.upPressed = buttons.up && !lastButtons.up;
  buttons.downPressed = buttons.down && !lastButtons.down;
  buttons.leftPressed = buttons.left && !lastButtons.left;
  buttons.rightPressed = buttons.right && !lastButtons.right;
  buttons.btnAPressed = buttons.btnA && !lastButtons.btnA;
  buttons.btnBPressed = buttons.btnB && !lastButtons.btnB;
  buttons.joyBtnPressed = buttons.joyBtn && !lastButtons.joyBtn;
}

bool isButtonPressed(int button) {
  return false;
}

bool wasButtonJustPressed(int button) {
  return false;
}

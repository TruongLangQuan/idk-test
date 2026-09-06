#include <M5StickCPlus2.h>
#include "Level.h"
#include "Screen.h"

Level level;
Screen scr;

void setup() {
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    
    // Initialize 5-way tactile switch pins
    pinMode(32, INPUT_PULLUP); // UP
    pinMode(33, INPUT_PULLUP); // DOWN
    pinMode(25, INPUT_PULLUP); // LEFT
    pinMode(26, INPUT_PULLUP); // RIGHT
    pinMode(0, INPUT_PULLUP);  // CENTER
    
    StickCP2.Display.setRotation(1);
    scr.create(Size(SCREEN_WIDTH, SCREEN_HEIGHT));
    level.create();
}

void loop() {
    StickCP2.update();
    level.update();
    
    scr.beginDraw();
    level.draw();
    scr.endDraw();
}

/*
  === M5Stacks Sonic The Hedgehog port ===

  !!! WARNING !!!
  This is an unofficial port of the game, this product was created exclusively for educational purposes and
  it should NOT be sold. As for code, it can be used as you like, but in that case, please give me a credit.

  All characters and graphics are copyright of SEGA.

  You can configure the game in a file "GameSettings.h"
  
  --- Controls ---
  Left   button - left
  Right  button - right
  Middle button - jump
  Left and Right button when moving   - roll
  Left and Right button when standing - spindash

  Special thanks to:
  Sonic Physics Guide - http://info.sonicretro.org/Sonic_Physics_Guide
  Sprite Resource     - https://www.spriters-resource.com/

  Author: GalaxyShad, 2020
  GitHub: https://github.com/GalaxyShad
*/

// ========================== //
#include <Arduino.h>
#include "gameLogo.h"

#include "Settings.h"
#include "Screen.h"

#include "structs/Geometry.h"
#include "Level.h"

Screen scr;
Level lv;
int startFrame = 0;
int fps = 0;
float frameFrame = 0;
UNIT_JOYC joyc;

void setup() {
	auto cfg = M5.config();
	StickCP2.begin(cfg);
	StickCP2.Display.setRotation(1);
	StickCP2.Display.fillScreen(TFT_BLACK);
	
	StickCP2.Display.setTextSize(2);
	StickCP2.Display.setCursor(20, 50);
	StickCP2.Display.println("M5Stick Sonic");
	StickCP2.Display.setCursor(20, 80);
	StickCP2.Display.println("Init JoyC...");
	
	while (!joyc.begin(&Wire, 0x54, 0, 26, 100000L)) {
		StickCP2.Display.println("JoyC not found!");
		delay(500);
	}
	
	StickCP2.Display.println("Ready!");
	delay(1000);
	
	scr.create(Size(SCREEN_WIDTH, SCREEN_HEIGHT));
}

void loop() {
  lv = Level();
  lv.create();
  while(lv.isLevelPlaying()) {
    startFrame = millis();
    lv.update();
    if (1.0 / frameFrame <= 36)
      lv.update();
    scr.beginDraw();
    lv.draw();
    scr.endDraw();
    
    frameFrame = (millis() - startFrame) / 1000.0;
    fps = 1.0 / frameFrame;
  }
  
}


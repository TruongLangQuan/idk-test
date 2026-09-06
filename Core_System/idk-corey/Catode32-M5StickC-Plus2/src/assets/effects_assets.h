#pragma once
// effects_assets.h - Auto-converted from effects.py
#include "Renderer.h"

static const uint8_t SPEECH_BUBBLE_F0[] PROGMEM = { 0x3f, 0xfe, 0x00, 0x7f, 0xff, 0x00, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0x7f, 0xff, 0x00, 0x3f, 0xfe, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x07, 0x80 };
static const Sprite SPEECH_BUBBLE = {
  17, 17, 8, 16,
  1.0f, 0,
  makeStatic(0), makeStatic(0),
  makeStatic(0), makeStatic(0),
  makeStatic(0),  makeStatic(0),
  false,
  1, { SPEECH_BUBBLE_F0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
  0, { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const uint8_t BUBBLE_HEART_F0[] PROGMEM = { 0x36, 0x00, 0x7f, 0x00, 0xff, 0x80, 0xff, 0x80, 0xff, 0x80, 0x7f, 0x00, 0x3e, 0x00, 0x1c, 0x00, 0x08, 0x00 };
static const Sprite BUBBLE_HEART = {
  9, 9, 4, 8,
  1.0f, 0,
  makeStatic(0), makeStatic(0),
  makeStatic(0), makeStatic(0),
  makeStatic(0),  makeStatic(0),
  false,
  1, { BUBBLE_HEART_F0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
  0, { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const uint8_t BUBBLE_QUESTION_F0[] PROGMEM = { 0x3e, 0x00, 0x7f, 0x00, 0x73, 0x00, 0x67, 0x00, 0x0e, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1c, 0x00 };
static const Sprite BUBBLE_QUESTION = {
  9, 9, 4, 8,
  1.0f, 0,
  makeStatic(0), makeStatic(0),
  makeStatic(0), makeStatic(0),
  makeStatic(0),  makeStatic(0),
  false,
  1, { BUBBLE_QUESTION_F0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
  0, { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const uint8_t BUBBLE_EXCLAIM_F0[] PROGMEM = { 0x1c, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0x18, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00 };
static const Sprite BUBBLE_EXCLAIM = {
  9, 9, 4, 8,
  1.0f, 0,
  makeStatic(0), makeStatic(0),
  makeStatic(0), makeStatic(0),
  makeStatic(0),  makeStatic(0),
  false,
  1, { BUBBLE_EXCLAIM_F0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
  0, { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const uint8_t BUBBLE_NOTE_F0[] PROGMEM = { 0x0c, 0x00, 0x0e, 0x00, 0x0b, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x38, 0x00, 0x78, 0x00, 0x30, 0x00 };
static const Sprite BUBBLE_NOTE = {
  9, 9, 4, 8,
  1.0f, 0,
  makeStatic(0), makeStatic(0),
  makeStatic(0), makeStatic(0),
  makeStatic(0),  makeStatic(0),
  false,
  1, { BUBBLE_NOTE_F0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
  0, { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const uint8_t BUBBLE_STAR_F0[] PROGMEM = { 0x08, 0x00, 0x1c, 0x00, 0x1c, 0x00, 0xff, 0x80, 0x7f, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x63, 0x00, 0x41, 0x00 };
static const Sprite BUBBLE_STAR = {
  9, 9, 4, 8,
  1.0f, 0,
  makeStatic(0), makeStatic(0),
  makeStatic(0), makeStatic(0),
  makeStatic(0),  makeStatic(0),
  false,
  1, { BUBBLE_STAR_F0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
  0, { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

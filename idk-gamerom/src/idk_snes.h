#pragma once

#include <M5Unified.h>
#include <Arduino.h>

bool idk_snes_begin(const String& path, String& error);
void idk_snes_run_frame(M5Canvas& canvas);
void idk_snes_end();
bool idk_snes_active();
uint32_t idk_snes_max_rom_kb();

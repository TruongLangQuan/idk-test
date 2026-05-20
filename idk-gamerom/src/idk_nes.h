#pragma once

#include <Arduino.h>

bool idk_nes_begin(const String& path, String& error);
bool idk_nes_active();
void idk_nes_end();
uint32_t idk_nes_max_rom_kb();
const char* idk_nes_last_error();

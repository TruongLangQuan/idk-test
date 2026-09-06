#ifndef PREFS_H
#define PREFS_H

#include <Arduino.h>

void prefs_init();

String prefs_get_note();
void prefs_set_note(String note);

String prefs_get_timetable();
void prefs_set_timetable(String json);

int prefs_get_ss_type();
void prefs_set_ss_type(int type);

int prefs_get_ss_speed();
void prefs_set_ss_speed(int speed);

uint32_t prefs_get_ss_color();
void prefs_set_ss_color(uint32_t color);

String prefs_get_media_path();
void prefs_set_media_path(String path);

int prefs_get_brightness();
void prefs_set_brightness(int br);

#endif // PREFS_H
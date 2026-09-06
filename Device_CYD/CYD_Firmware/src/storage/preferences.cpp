#include "preferences.h"
#include <Preferences.h>

static Preferences prefs;

void prefs_init() {
    prefs.begin("cyd_app", false);
}

String prefs_get_note() { return prefs.getString("note", ""); }
void prefs_set_note(String note) { prefs.putString("note", note); }

String prefs_get_timetable() { return prefs.getString("timetable", "{\"table\":[]}"); }
void prefs_set_timetable(String json) { prefs.putString("timetable", json); }

int prefs_get_ss_type() { return prefs.getInt("ss_type", 0); }
void prefs_set_ss_type(int type) { prefs.putInt("ss_type", type); }

int prefs_get_ss_speed() { return prefs.getInt("ss_speed", 50); }
void prefs_set_ss_speed(int speed) { prefs.putInt("ss_speed", speed); }

uint32_t prefs_get_ss_color() { return prefs.getUInt("ss_color", 0x07FF); }
void prefs_set_ss_color(uint32_t color) { prefs.putUInt("ss_color", color); }

String prefs_get_media_path() { return prefs.getString("media_path", ""); }
void prefs_set_media_path(String path) { prefs.putString("media_path", path); }

int prefs_get_brightness() { return prefs.getInt("brightness", 255); }
void prefs_set_brightness(int br) { prefs.putInt("brightness", br); }
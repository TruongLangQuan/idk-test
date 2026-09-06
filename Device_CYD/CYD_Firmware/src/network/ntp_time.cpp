#include "ntp_time.h"
#include "../config.h"
#include <ESP32Time.h>

static ESP32Time rtc(0); // Offset 0 because configTime already offsets system time
static bool ntp_synced = false;

void ntp_init() {
    configTime(TZ_OFFSET, 0, NTP_SERVER, "time.google.com", "time.windows.com");
    DEBUG_PRINTLN("NTP Initialized.");
}

void ntp_update() {
    if (!ntp_synced && time(nullptr) > 100000) {
        ntp_synced = true;
        DEBUG_PRINTLN("NTP Synced: " + get_current_time_string());
    }
}

String get_current_time_string() {
    if (ntp_synced || time(nullptr) > 100000) {
        return rtc.getTime("%H:%M:%S");
    }
    return "00:00:00";
}

String get_current_date_string() {
    if (ntp_synced || time(nullptr) > 100000) {
        return rtc.getTime("%a, %d/%m/%Y");
    }
    return "Thu, 01/01/1970";
}
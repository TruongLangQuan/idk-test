#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <Arduino.h>

void ntp_init();
void ntp_update();
String get_current_time_string();
String get_current_date_string();

#endif // NTP_TIME_H
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

void wifi_init();
void wifi_update();
bool wifi_is_connected();
bool wifi_is_ap_mode();
String wifi_get_ssid();
String wifi_get_ip();

#endif // WIFI_MANAGER_H
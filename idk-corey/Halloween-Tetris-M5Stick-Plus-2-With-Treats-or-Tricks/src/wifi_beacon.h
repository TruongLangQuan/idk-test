#ifndef WIFI_BEACON_H
#define WIFI_BEACON_H

#include <WiFi.h>
#include "esp_wifi.h"

// WiFi beacon functionality
void beaconSpam();
void generateRandomWiFiMac(uint8_t *mac);
char* randomSSID();
void beaconSpamList(const char list[]);
void nextChannel();

extern const char Beacons[] PROGMEM;
extern const uint8_t channels[];
extern uint8_t wifi_channel;
extern uint8_t channelIndex;

#endif
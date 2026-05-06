#include "wifi_manager.h"
#include "../config.h"
#include <WiFi.h>
#include <ESPmDNS.h>

static unsigned long last_wifi_check = 0;
static bool ap_mode = false;

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        delay(100); // Only blocking during initial boot
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        // Fallback to AP
        WiFi.mode(WIFI_AP);
        WiFi.softAP("CYD_Setup", "12345678");
        ap_mode = true;
    } else {
        MDNS.begin("cyd");
    }
}

void wifi_update() {
    if (ap_mode) return;
    
    unsigned long currentMillis = millis();
    if (currentMillis - last_wifi_check >= 5000) {
        last_wifi_check = currentMillis;
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.reconnect();
        }
    }
}

bool wifi_is_connected() {
    return ap_mode ? true : (WiFi.status() == WL_CONNECTED);
}

bool wifi_is_ap_mode() {
    return ap_mode;
}

String wifi_get_ssid() {
    return ap_mode ? "CYD_Setup" : String(WIFI_SSID);
}

String wifi_get_ip() {
    return ap_mode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}
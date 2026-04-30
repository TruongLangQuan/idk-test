#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <Preferences.h>

struct NetworkInfo {
    String ssid;
    int rssi;
    wifi_auth_mode_t encryption;
    String encType;
};

class WiFiScanner {
private:
    Preferences prefs;
    NetworkInfo networks[20];
    int networkCount;
    int selectedIndex;
    String inputBuffer;
    
public:
    WiFiScanner();
    void begin();
    bool scanAndConnect();
    void scanNetworks();
    void displayNetworks();
    bool selectNetwork();
    String inputPassword(String ssid);
    bool connectToNetwork(String ssid, String password);
    String getSavedPassword(String ssid);
    void savePassword(String ssid, String password);
    String getEncryptionType(wifi_auth_mode_t encryption);
    void showStatus(String message, uint16_t color = 0xFFFF);
};

#endif
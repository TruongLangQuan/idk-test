#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <Preferences.h>

struct WiFiNetwork {
    String ssid;
    int encryption;
    int rssi;
    String encryptionType;
};

#define MAX_WIFI_NETWORKS 10

class WiFiManager {
private:
    Preferences preferences;
    WiFiNetwork networks[MAX_WIFI_NETWORKS];
    int networkCount;
    int selectedNetwork;
    bool scanCompleted;
    
public:
    WiFiManager();
    
    // Core functions
    void begin();
    bool showConnectionMenu();
    bool connectToNetwork(String ssid, String password = "");
    void disconnect();
    
    // Network scanning
    void scanNetworks();
    void displayNetworkList();
    void handleNetworkSelection();
    
    // Password management
    String getSavedPassword(String ssid);
    void savePassword(String ssid, String password);
    
    // Status functions
    bool isConnected();
    String getIP();
    String getSSID();
    int getRSSI();
    
    // Display functions
    void displayScanning();
    void displayConnecting(String ssid);
    void displayConnected();
    void displayError(String error);
    
private:
    String getEncryptionString(int encryption);
    bool connectWithRetry(String ssid, String password, int maxRetries = 3);
};

#endif
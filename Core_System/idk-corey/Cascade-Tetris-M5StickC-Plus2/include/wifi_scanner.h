#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <M5StickCPlus2.h>
#include <WiFi.h>

// WiFi scanner functionality
void wifiScanner();
void displayNetworks();
void updateSelectionDisplay();
void drawSignalBar(int x, int y, int width, int height, int rssi);
String getSignalQuality(int rssi);
String getEncryptionType(wifi_auth_mode_t encType);
void drawNetworkItem(int index, String ssid, int rssi, wifi_auth_mode_t encryption, bool selected);

// Scanner configuration
#define MAX_NETWORKS_DISPLAY 6  // Fit on small screen
#define SCAN_INTERVAL 2000      // 2 second intervals
#define RSSI_EXCELLENT -30
#define RSSI_GOOD -50  
#define RSSI_FAIR -70
#define RSSI_WEAK -80

// Colors for signal strength
#define COLOR_EXCELLENT 0x07E0  // Green
#define COLOR_GOOD      0x87E0  // Light green  
#define COLOR_FAIR      0xFFE0  // Yellow
#define COLOR_WEAK      0xFC00  // Orange
#define COLOR_POOR      0xF800  // Red

#endif
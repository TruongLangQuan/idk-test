#include "wifi_manager.h"

WiFiManager::WiFiManager() {
    selectedNetwork = 0;
    scanCompleted = false;
    networkCount = 0;
}

void WiFiManager::begin() {
    preferences.begin("wifi_manager", false);
    WiFi.mode(WIFI_STA);
}

bool WiFiManager::showConnectionMenu() {
    if (isConnected()) {
        M5.Lcd.fillScreen(0x0000);
        M5.Lcd.setTextColor(0x07E0); // Green
        M5.Lcd.setTextSize(1);
        M5.Lcd.drawString("Already Connected", 10, 50);
        M5.Lcd.drawString("SSID: " + WiFi.SSID(), 10, 70);
        M5.Lcd.drawString("IP: " + WiFi.localIP().toString(), 10, 90);
        M5.Lcd.drawString("A: Disconnect", 10, 130);
        M5.Lcd.drawString("B: Back", 10, 150);
        
        while (true) {
            M5.update();
            if (M5.BtnA.wasClicked()) {
                disconnect();
                break;
            }
            if (M5.BtnB.wasClicked()) {
                return true; // Already connected
            }
            delay(50);
        }
    }
    
    // Scan for networks
    scanNetworks();
    
    // Show network list and handle selection
    while (!isConnected()) {
        displayNetworkList();
        handleNetworkSelection();
        
        if (M5.BtnB.wasClicked()) {
            return false; // User cancelled
        }
        
        delay(50);
    }
    
    return isConnected();
}

void WiFiManager::scanNetworks() {
    displayScanning();
    
    networkCount = 0;
    int n = WiFi.scanNetworks();
    
    for (int i = 0; i < n && networkCount < MAX_WIFI_NETWORKS; i++) {
        String ssid = WiFi.SSID(i);
        
        // Skip duplicates
        bool duplicate = false;
        for (int j = 0; j < networkCount; j++) {
            if (networks[j].ssid == ssid) {
                duplicate = true;
                break;
            }
        }
        
        if (!duplicate) {
            networks[networkCount].ssid = ssid;
            networks[networkCount].encryption = WiFi.encryptionType(i);
            networks[networkCount].rssi = WiFi.RSSI(i);
            networks[networkCount].encryptionType = getEncryptionString(networks[networkCount].encryption);
            networkCount++;
        }
    }
    
    scanCompleted = true;
    selectedNetwork = 0;
}

void WiFiManager::displayNetworkList() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF); // Cyan
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("SELECT WIFI", 25, 10);
    
    if (networkCount == 0) {
        M5.Lcd.setTextColor(0xF800); // Red
        M5.Lcd.drawString("No networks found", 10, 50);
        M5.Lcd.drawString("PWR: Rescan", 10, 70);
        M5.Lcd.drawString("B: Back", 10, 90);
        return;
    }
    
    // Display networks (max 6 visible at once)
    int startIndex = max(0, selectedNetwork - 3);
    int endIndex = min(networkCount, startIndex + 6);
    
    for (int i = startIndex; i < endIndex; i++) {
        int y = 30 + (i - startIndex) * 15;
        String displayText = networks[i].ssid;
        
        // Truncate long SSIDs
        if (displayText.length() > 15) {
            displayText = displayText.substring(0, 12) + "...";
        }
        
        // Add encryption indicator
        if (networks[i].encryption != WIFI_AUTH_OPEN) {
            displayText = "* " + displayText;
        }
        
        // Add signal strength
        displayText += " (" + String(networks[i].rssi) + ")";
        
        uint16_t color = (i == selectedNetwork) ? 0xFFE0 : 0xFFFF; // Yellow for selected
        M5.Lcd.setTextColor(color);
        
        if (i == selectedNetwork) {
            M5.Lcd.drawString("> " + displayText, 5, y);
        } else {
            M5.Lcd.drawString("  " + displayText, 5, y);
        }
    }
    
    // Controls
    M5.Lcd.setTextColor(0x8410); // Gray
    M5.Lcd.drawString("PWR: Up  A: Down", 5, 180);
    M5.Lcd.drawString("M5: Connect  B: Back", 5, 195);
}

void WiFiManager::handleNetworkSelection() {
    static unsigned long lastBtnPress = 0;
    
    M5.update();
    
    // Handle button navigation
    if (millis() - lastBtnPress > 200) {
        if (M5.BtnPWR.wasClicked() && selectedNetwork > 0) {
            selectedNetwork--;
            lastBtnPress = millis();
        }
        else if (M5.BtnA.wasClicked() && selectedNetwork < networkCount - 1) {
            selectedNetwork++;
            lastBtnPress = millis();
        }
        
        // Rescan with long press of PWR
        static unsigned long pwrPressStart = 0;
        if (M5.BtnPWR.isPressed()) {
            if (pwrPressStart == 0) pwrPressStart = millis();
            if (millis() - pwrPressStart > 2000) {
                scanNetworks();
                pwrPressStart = 0;
                lastBtnPress = millis();
            }
        } else {
            pwrPressStart = 0;
        }
    }
    
    // Connect to selected network with M5 button (middle button)
    if (M5.BtnA.wasClicked()) {
        if (networkCount > 0 && selectedNetwork < networkCount) {
            WiFiNetwork& network = networks[selectedNetwork];
            String password = "";
            
            if (network.encryption != WIFI_AUTH_OPEN) {
                // Try saved password first
                password = getSavedPassword(network.ssid);
                // For open networks or if no saved password, try connecting without password first
            }
            
            if (connectToNetwork(network.ssid, password)) {
                if (password != "") savePassword(network.ssid, password);
                displayConnected();
                delay(2000);
                return;
            }
        }
    }
}

bool WiFiManager::connectToNetwork(String ssid, String password) {
    displayConnecting(ssid);
    return connectWithRetry(ssid, password, 3);
}

bool WiFiManager::connectWithRetry(String ssid, String password, int maxRetries) {
    for (int attempt = 1; attempt <= maxRetries; attempt++) {
        WiFi.begin(ssid.c_str(), password.c_str());
        
        // Show connection progress
        for (int i = 0; i < 20; i++) {
            if (WiFi.status() == WL_CONNECTED) {
                return true;
            }
            
            // Update display with dots
            M5.Lcd.fillRect(0, 100, 135, 20, 0x0000);
            M5.Lcd.setTextColor(0xFFFF);
            String dots = "Attempt " + String(attempt) + "/3 ";
            for (int j = 0; j <= i % 4; j++) {
                dots += ".";
            }
            M5.Lcd.drawString(dots, 10, 100);
            
            delay(500);
        }
        
        WiFi.disconnect();
        delay(1000);
    }
    
    displayError("Connection Failed");
    delay(2000);
    return false;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

String WiFiManager::getSavedPassword(String ssid) {
    return preferences.getString(("pwd_" + ssid).c_str(), "");
}

void WiFiManager::savePassword(String ssid, String password) {
    preferences.putString(("pwd_" + ssid).c_str(), password);
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIP() {
    return WiFi.localIP().toString();
}

String WiFiManager::getSSID() {
    return WiFi.SSID();
}

int WiFiManager::getRSSI() {
    return WiFi.RSSI();
}

void WiFiManager::displayScanning() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF); // Cyan
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("SCANNING WIFI", 20, 50);
    M5.Lcd.drawString("Please wait...", 25, 80);
}

void WiFiManager::displayConnecting(String ssid) {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0xFFE0); // Yellow
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("CONNECTING", 30, 30);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString("SSID:", 10, 60);
    M5.Lcd.drawString(ssid, 10, 80);
}

void WiFiManager::displayConnected() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07E0); // Green
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("CONNECTED!", 30, 30);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString("SSID: " + getSSID(), 10, 60);
    M5.Lcd.drawString("IP: " + getIP(), 10, 80);
    M5.Lcd.drawString("Signal: " + String(getRSSI()) + " dBm", 10, 100);
}

void WiFiManager::displayError(String error) {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0xF800); // Red
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("ERROR", 45, 50);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString(error, 10, 80);
}

String WiFiManager::getEncryptionString(int encryption) {
    switch (encryption) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-E";
        default: return "Unknown";
    }
}
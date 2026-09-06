#include "wifi_scanner.h"

WiFiScanner::WiFiScanner() {
    networkCount = 0;
    selectedIndex = 0;
    inputBuffer = "";
}

void WiFiScanner::begin() {
    prefs.begin("wifi_creds", false);
    WiFi.mode(WIFI_STA);
}

bool WiFiScanner::scanAndConnect() {
    showStatus("Scanning WiFi...", 0x07FF);
    scanNetworks();
    
    if (networkCount == 0) {
        showStatus("No networks found!", 0xF800);
        delay(2000);
        return false;
    }
    
    return selectNetwork();
}

void WiFiScanner::scanNetworks() {
    networkCount = 0;
    int n = WiFi.scanNetworks();
    
    // Filter and deduplicate
    for (int i = 0; i < n && networkCount < 20; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) continue; // Skip empty SSIDs
        
        // Check for duplicates
        bool duplicate = false;
        for (int j = 0; j < networkCount; j++) {
            if (networks[j].ssid == ssid) {
                // Keep the one with better signal
                if (WiFi.RSSI(i) > networks[j].rssi) {
                    networks[j].rssi = WiFi.RSSI(i);
                    networks[j].encryption = WiFi.encryptionType(i);
                    networks[j].encType = getEncryptionType(WiFi.encryptionType(i));
                }
                duplicate = true;
                break;
            }
        }
        
        if (!duplicate) {
            networks[networkCount].ssid = ssid;
            networks[networkCount].rssi = WiFi.RSSI(i);
            networks[networkCount].encryption = WiFi.encryptionType(i);
            networks[networkCount].encType = getEncryptionType(WiFi.encryptionType(i));
            networkCount++;
        }
    }
    
    // Sort by signal strength
    for (int i = 0; i < networkCount - 1; i++) {
        for (int j = i + 1; j < networkCount; j++) {
            if (networks[i].rssi < networks[j].rssi) {
                NetworkInfo temp = networks[i];
                networks[i] = networks[j];
                networks[j] = temp;
            }
        }
    }
}

bool WiFiScanner::selectNetwork() {
    selectedIndex = 0;
    
    while (true) {
        displayNetworks();
        M5.update();
        
        // Navigation
        if (M5.BtnPWR.wasClicked() && selectedIndex > 0) {
            selectedIndex--;
        }
        else if (M5.BtnA.wasClicked() && selectedIndex < networkCount - 1) {
            selectedIndex++;
        }
        else if (M5.BtnB.wasClicked()) {
            // Connect to selected network
            String ssid = networks[selectedIndex].ssid;
            String password = "";
            
            if (networks[selectedIndex].encryption != WIFI_AUTH_OPEN) {
                password = getSavedPassword(ssid);
                if (password == "") {
                    password = inputPassword(ssid);
                    if (password == "CANCEL") {
                        continue; // Go back to network list
                    }
                }
            }
            
            if (connectToNetwork(ssid, password)) {
                if (password != "") {
                    savePassword(ssid, password);
                }
                return true;
            } else {
                showStatus("Connection failed!", 0xF800);
                delay(2000);
                continue;
            }
        }
        
        delay(50);
    }
}

void WiFiScanner::displayNetworks() {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("WiFi Networks", 25, 5);
    
    // Show current selection info
    M5.Lcd.setTextColor(0xFFE0);
    M5.Lcd.drawString("PWR:Up A:Down B:Connect", 5, 220);
    
    // Display networks (show 8 at a time)
    int startIdx = max(0, selectedIndex - 4);
    int endIdx = min(networkCount, startIdx + 8);
    
    for (int i = startIdx; i < endIdx; i++) {
        int y = 25 + (i - startIdx) * 22;
        
        // Network name (truncate if too long)
        String displayName = networks[i].ssid;
        if (displayName.length() > 18) {
            displayName = displayName.substring(0, 15) + "...";
        }
        
        // Selection indicator and encryption
        String prefix = (i == selectedIndex) ? "> " : "  ";
        if (networks[i].encryption != WIFI_AUTH_OPEN) {
            prefix += "*";
        }
        
        uint16_t color = (i == selectedIndex) ? 0xFFE0 : 0xFFFF;
        M5.Lcd.setTextColor(color);
        M5.Lcd.drawString(prefix + displayName, 5, y);
        
        // Signal strength and encryption type
        M5.Lcd.setTextColor(0x8410); // Gray
        String info = String(networks[i].rssi) + "dB " + networks[i].encType;
        M5.Lcd.drawString(info, 5, y + 11);
    }
}

String WiFiScanner::inputPassword(String ssid) {
    inputBuffer = "";
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(0x07FF);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Enter Password", 25, 20);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString("SSID: " + ssid, 10, 40);
    
    // Simple character input using buttons
    String charset = "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{}|;:,.<>?";
    int charIndex = 0;
    
    while (true) {
        M5.update();
        
        // Display current character and input buffer
        M5.Lcd.fillRect(0, 60, 135, 120, 0x0000);
        M5.Lcd.setTextColor(0xFFFF);
        M5.Lcd.drawString("Password:", 10, 60);
        M5.Lcd.drawString(inputBuffer + "_", 10, 80);
        
        M5.Lcd.setTextColor(0xFFE0);
        M5.Lcd.drawString("Current: " + String(charset[charIndex]), 10, 110);
        
        M5.Lcd.setTextColor(0x8410);
        M5.Lcd.drawString("PWR:Next A:Add B:Done", 5, 140);
        M5.Lcd.drawString("Hold PWR:Back", 25, 160);
        
        // Button handling
        if (M5.BtnPWR.wasClicked()) {
            charIndex = (charIndex + 1) % charset.length();
        }
        else if (M5.BtnA.wasClicked()) {
            inputBuffer += charset[charIndex];
            if (inputBuffer.length() > 20) inputBuffer = inputBuffer.substring(0, 20); // Limit length
        }
        else if (M5.BtnB.wasClicked()) {
            if (inputBuffer.length() > 0) {
                return inputBuffer;
            }
        }
        
        // Hold PWR to go back
        static unsigned long pwrStart = 0;
        if (M5.BtnPWR.isPressed()) {
            if (pwrStart == 0) pwrStart = millis();
            if (millis() - pwrStart > 2000) {
                return "CANCEL";
            }
        } else {
            pwrStart = 0;
        }
        
        // Hold A to backspace
        static unsigned long aStart = 0;
        if (M5.BtnA.isPressed()) {
            if (aStart == 0) aStart = millis();
            if (millis() - aStart > 1000) {
                if (inputBuffer.length() > 0) {
                    inputBuffer = inputBuffer.substring(0, inputBuffer.length() - 1);
                }
                aStart = millis() - 500; // Repeat backspace
            }
        } else {
            aStart = 0;
        }
        
        delay(100);
    }
}

bool WiFiScanner::connectToNetwork(String ssid, String password) {
    showStatus("Connecting to:", 0x07FF);
    M5.Lcd.setTextColor(0xFFFF);
    M5.Lcd.drawString(ssid, 10, 80);
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    for (int i = 0; i < 30; i++) { // 15 second timeout
        if (WiFi.status() == WL_CONNECTED) {
            showStatus("Connected!", 0x07E0);
            M5.Lcd.setTextColor(0xFFFF);
            M5.Lcd.drawString("IP: " + WiFi.localIP().toString(), 10, 80);
            delay(2000);
            return true;
        }
        
        // Show progress dots
        M5.Lcd.fillRect(10, 100, 115, 20, 0x0000);
        M5.Lcd.setTextColor(0xFFE0);
        String dots = "Connecting";
        for (int j = 0; j <= (i % 4); j++) dots += ".";
        M5.Lcd.drawString(dots, 10, 100);
        
        delay(500);
    }
    
    WiFi.disconnect();
    return false;
}

String WiFiScanner::getSavedPassword(String ssid) {
    return prefs.getString(ssid.c_str(), "");
}

void WiFiScanner::savePassword(String ssid, String password) {
    prefs.putString(ssid.c_str(), password);
}

String WiFiScanner::getEncryptionType(wifi_auth_mode_t encryption) {
    switch (encryption) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2E";
        default: return "?";
    }
}

void WiFiScanner::showStatus(String message, uint16_t color) {
    M5.Lcd.fillScreen(0x0000);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString(message, 10, 60);
}
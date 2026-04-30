#include "wifi_scanner.h"
#include "UNIT_MiniJoyC.h"

extern UNIT_JOYC Joystick;

struct NetworkInfo {
  String ssid;
  int32_t rssi;
  wifi_auth_mode_t encryption;
  uint8_t channel;
};

NetworkInfo networks[MAX_NETWORKS_DISPLAY];
int networkCount = 0;
int selectedNetwork = 0;
unsigned long lastScan = 0;
unsigned long lastNavigation = 0;
bool scannerActive = true;
bool upPressed = false;
bool downPressed = false;

void wifiScanner() {
  // Initialize WiFi in station mode for scanning
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  
  // Header
  M5.Lcd.drawString("WiFi Signal Scanner", 2, 2);
  M5.Lcd.drawLine(0, 12, 135, 12, WHITE);
  M5.Lcd.drawString("Up/Dn:Select M5:Exit", 2, 225);
  
  // Initial scan
  displayNetworks();
  lastScan = millis();
  
  while(scannerActive) {
    // Handle input
    if(digitalRead(37) == 0) { // M5 button pressed
      delay(100);
      break;
    }
    
    // Improved joystick navigation with debouncing
    bool currentUp = (Joystick.getADCValue(1) > 2950);
    bool currentDown = (Joystick.getADCValue(1) < 1600);
    
    // Handle Up navigation
    if(currentUp && !upPressed && millis() - lastNavigation > 80) {
      if(selectedNetwork > 0) {
        selectedNetwork--;
        updateSelectionDisplay();
        lastNavigation = millis();
      }
    }
    upPressed = currentUp;
    
    // Handle Down navigation  
    if(currentDown && !downPressed && millis() - lastNavigation > 80) {
      if(selectedNetwork < networkCount - 1) {
        selectedNetwork++;
        updateSelectionDisplay();
        lastNavigation = millis();
      }
    }
    downPressed = currentDown;
    
    // Auto-refresh scan (less frequent to avoid interrupting navigation)
    if(millis() - lastScan > SCAN_INTERVAL) {
      displayNetworks();
      lastScan = millis();
    }
    
    delay(20); // Much faster loop
  }
  
  // Clean up
  WiFi.scanDelete();
  scannerActive = true; // Reset for next time
  upPressed = false;
  downPressed = false;
}

void displayNetworks() {
  // Scan for networks
  int numNetworks = WiFi.scanNetworks(false, true, false, 300);
  
  // Clear network display area
  M5.Lcd.fillRect(0, 15, 135, 207, BLACK);
  
  if(numNetworks == 0) {
    M5.Lcd.drawString("No networks found", 10, 50);
    networkCount = 0;
    return;
  }
  
  // Sort networks by signal strength (simple bubble sort for small array)
  NetworkInfo tempNetworks[numNetworks];
  for(int i = 0; i < numNetworks && i < MAX_NETWORKS_DISPLAY; i++) {
    tempNetworks[i].ssid = WiFi.SSID(i);
    tempNetworks[i].rssi = WiFi.RSSI(i);
    tempNetworks[i].encryption = WiFi.encryptionType(i);
    tempNetworks[i].channel = WiFi.channel(i);
  }
  
  // Simple sort by RSSI (strongest first)
  int sortCount = min(numNetworks, MAX_NETWORKS_DISPLAY);
  for(int i = 0; i < sortCount - 1; i++) {
    for(int j = 0; j < sortCount - i - 1; j++) {
      if(tempNetworks[j].rssi < tempNetworks[j + 1].rssi) {
        NetworkInfo temp = tempNetworks[j];
        tempNetworks[j] = tempNetworks[j + 1];
        tempNetworks[j + 1] = temp;
      }
    }
  }
  
  // Copy sorted networks
  networkCount = sortCount;
  for(int i = 0; i < networkCount; i++) {
    networks[i] = tempNetworks[i];
  }
  
  // Constrain selection
  if(selectedNetwork >= networkCount) selectedNetwork = networkCount - 1;
  if(selectedNetwork < 0) selectedNetwork = 0;
  
  // Display networks
  for(int i = 0; i < networkCount; i++) {
    drawNetworkItem(i, networks[i].ssid, networks[i].rssi, 
                   networks[i].encryption, i == selectedNetwork);
  }
  
  // Display scan count
  M5.Lcd.setTextColor(DARKGREY);
  M5.Lcd.drawString("Found: " + String(numNetworks), 80, 210);
  
  WiFi.scanDelete(); // Free memory
}

void updateSelectionDisplay() {
  // Fast update - only redraw the selection highlights
  for(int i = 0; i < networkCount; i++) {
    int yPos = 18 + (i * 32);
    
    if(i == selectedNetwork) {
      // Draw selection background
      M5.Lcd.fillRect(0, yPos - 2, 135, 30, 0x0208); // Dark blue
    } else {
      // Clear selection background  
      M5.Lcd.fillRect(0, yPos - 2, 135, 30, BLACK);
    }
    
    // Redraw this network item
    drawNetworkItem(i, networks[i].ssid, networks[i].rssi, 
                   networks[i].encryption, i == selectedNetwork);
  }
}

void drawNetworkItem(int index, String ssid, int rssi, wifi_auth_mode_t encryption, bool selected) {
  int yPos = 18 + (index * 32);
  
  // Background for selected item
  if(selected) {
    M5.Lcd.fillRect(0, yPos - 2, 135, 30, 0x0208); // Dark blue
  }
  
  // Truncate long SSID names
  if(ssid.length() > 16) {
    ssid = ssid.substring(0, 13) + "...";
  }
  if(ssid.length() == 0) {
    ssid = "[Hidden]";
  }
  
  // SSID name
  M5.Lcd.setTextColor(selected ? WHITE : LIGHTGREY);
  M5.Lcd.drawString(ssid, 4, yPos);
  
  // Signal strength bar
  drawSignalBar(4, yPos + 12, 80, 8, rssi);
  
  // RSSI value
  M5.Lcd.setTextColor(selected ? YELLOW : WHITE);
  M5.Lcd.drawString(String(rssi) + "dBm", 90, yPos);
  
  // Security indicator
  String security = getEncryptionType(encryption);
  uint16_t secColor = (encryption == WIFI_AUTH_OPEN) ? COLOR_WEAK : COLOR_GOOD;
  M5.Lcd.setTextColor(secColor);
  M5.Lcd.drawString(security, 90, yPos + 12);
}

void drawSignalBar(int x, int y, int width, int height, int rssi) {
  // Background
  M5.Lcd.fillRect(x, y, width, height, DARKGREY);
  
  // Calculate signal percentage
  int percentage;
  uint16_t color;
  
  if(rssi >= RSSI_EXCELLENT) {
    percentage = 100;
    color = COLOR_EXCELLENT;
  } else if(rssi >= RSSI_GOOD) {
    percentage = 80;
    color = COLOR_GOOD;
  } else if(rssi >= RSSI_FAIR) {
    percentage = 60;
    color = COLOR_FAIR;
  } else if(rssi >= RSSI_WEAK) {
    percentage = 40;
    color = COLOR_WEAK;
  } else {
    percentage = 20;
    color = COLOR_POOR;
  }
  
  // Draw signal bar
  int fillWidth = (width * percentage) / 100;
  if(fillWidth > 0) {
    M5.Lcd.fillRect(x, y, fillWidth, height, color);
  }
  
  // Add border
  M5.Lcd.drawRect(x, y, width, height, WHITE);
}

String getSignalQuality(int rssi) {
  if(rssi >= RSSI_EXCELLENT) return "Excellent";
  if(rssi >= RSSI_GOOD) return "Good";
  if(rssi >= RSSI_FAIR) return "Fair";  
  if(rssi >= RSSI_WEAK) return "Weak";
  return "Poor";
}

String getEncryptionType(wifi_auth_mode_t encType) {
  switch(encType) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA12";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2E";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA23";
    default: return "UNK";
  }
}
#include <M5StickCPlus2.h>
#include "WiFi.h"
#include "esp_wifi.h"
#include "WebServer.h"
#include "DNSServer.h"

#define Disp StickCP2.Display
#define SCREEN_WIDTH 135
#define SCREEN_HEIGHT 240

// Colors
#define BLACK      0x0000
#define WHITE      0xFFFF
#define RED        0xF800
#define GREEN      0x07E0
#define BLUE       0x001F
#define YELLOW     0xFFE0
#define ORANGE     0xFC00
#define PURPLE     0x8010
#define CYAN       0x07FF
#define MAGENTA    0xF81F

// Simple state machine
enum AppState {
  MAIN_MENU,
  SCANNING,
  RESULTS,
  COUNTER_ATTACK_ACTIVE
};

AppState currentState = MAIN_MENU;
int menuSelection = 0;
int threatCount = 0;
int networksFound = 0;
bool scanComplete = false;
unsigned long lastScanTime = 0;

// Counter-attack server
WebServer server(80);
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);
bool counterAttackActive = false;
// Portal killer variables
bool portalKillerActive = false;
int currentTargetIndex = 0;
unsigned long lastConnectionAttempt = 0;
String targetSSIDs[10];  // Store up to 10 target networks
int targetCount = 0;
bool isConnectedToTarget = false;
int spamCount = 0;

// Forward declarations
void showMainMenu();
void handleMainMenu();
void handleScanning();
void handleResults();
void handleCounterAttack();
void startThreatScan();
void startCounterAttack();
void activateShieldMode();
void performThreatScan();
void showResults();
void handleCaptivePortal();
void stopCounterAttack();
void spamPortal(String ssid);

void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  
  StickCP2.Display.setRotation(0);
  StickCP2.Display.setTextSize(1);
  
  Serial.println("Portal Killer Ready");
  showMainMenu();
}

void loop() {
  M5.update();
  
  switch(currentState) {
    case MAIN_MENU:
      handleMainMenu();
      break;
    case SCANNING:
      handleScanning();
      break;
    case RESULTS:
      handleResults();
      break;
    case COUNTER_ATTACK_ACTIVE:
      handleCounterAttack();
      break;
  }
  
  delay(50);
}

void showMainMenu() {
  Disp.fillScreen(BLACK);
  Disp.setTextColor(CYAN);
  Disp.setTextSize(2);
  Disp.setCursor(10, 10);
  Disp.println("PORTAL");
  Disp.setCursor(10, 30);
  Disp.println("KILLER");
  
  Disp.setTextSize(1);
  Disp.setCursor(10, 60);
  
  // Menu options
  if (menuSelection == 0) {
    Disp.setTextColor(YELLOW);
    Disp.println("> SCAN FOR THREATS");
  } else {
    Disp.setTextColor(WHITE);
    Disp.println("  SCAN FOR THREATS");
  }
  
  Disp.setCursor(10, 80);
  if (menuSelection == 1) {
    Disp.setTextColor(YELLOW);
    Disp.println("> COUNTER ATTACK");
  } else {
    Disp.setTextColor(WHITE);
    Disp.println("  COUNTER ATTACK");
  }
  
  Disp.setCursor(10, 100);
  if (menuSelection == 2) {
    Disp.setTextColor(YELLOW);
    Disp.println("> SHIELD MODE");
  } else {
    Disp.setTextColor(WHITE);
    Disp.println("  SHIELD MODE");
  }
  
  // Controls
  Disp.setTextColor(GREEN);
  Disp.setCursor(10, 140);
  Disp.println("A = SELECT");
  Disp.setCursor(10, 155);
  Disp.println("B = UP");
  Disp.setCursor(10, 170);
  Disp.println("C = DOWN");
  
  // Status
  Disp.setTextColor(CYAN);
  Disp.setCursor(10, 200);
  if (scanComplete) {
    Disp.println("Last: " + String(networksFound) + " nets, " + String(threatCount) + " threats");
  } else {
    Disp.println("Ready to scan");
  }
}

void handleMainMenu() {
  // Navigation
  if (M5.BtnB.wasClicked()) {
    menuSelection = (menuSelection > 0) ? menuSelection - 1 : 2;
    showMainMenu();
  }
  
  if (M5.BtnC.wasClicked()) {
    menuSelection = (menuSelection < 2) ? menuSelection + 1 : 0;
    showMainMenu();
  }
  
  // Selection
  if (M5.BtnA.wasClicked()) {
    if (menuSelection == 0) {
      currentState = SCANNING;
      startThreatScan();
    } else if (menuSelection == 1) {
      currentState = COUNTER_ATTACK_ACTIVE;
      startCounterAttack();
    } else if (menuSelection == 2) {
      activateShieldMode();
    }
  }
}

void startThreatScan() {
  Disp.fillScreen(BLACK);
  Disp.setTextColor(YELLOW);
  Disp.setTextSize(2);
  Disp.setCursor(10, 40);
  Disp.println("SCANNING");
  
  Disp.setTextSize(1);
  Disp.setTextColor(WHITE);
  Disp.setCursor(10, 80);
  Disp.println("Searching for threats...");
  
  Disp.setTextColor(RED);
  Disp.setCursor(10, 180);
  Disp.println("PRESS B TO EXIT");
  
  lastScanTime = millis();
}

void handleScanning() {
  // Exit button check - ALWAYS works!
  if (M5.BtnB.wasClicked()) {
    currentState = MAIN_MENU;
    showMainMenu();
    return;
  }
  
  // Simulate scan progress (you can replace with real WiFi scan)
  if (millis() - lastScanTime > 3000) { // 3 second scan
    performThreatScan();
    currentState = RESULTS;
    showResults();
  } else {
    // Show scanning animation
    int dots = ((millis() - lastScanTime) / 500) % 4;
    Disp.fillRect(10, 100, 120, 20, BLACK);
    Disp.setCursor(10, 100);
    Disp.setTextColor(GREEN);
    Disp.print("Scanning");
    for(int i = 0; i < dots; i++) {
      Disp.print(".");
    }
  }
}

void performThreatScan() {
  WiFi.mode(WIFI_MODE_STA);
  networksFound = WiFi.scanNetworks();
  threatCount = 0;
  
  for(int i = 0; i < networksFound; i++) {
    String ssid = WiFi.SSID(i);
    
    // Detect potential threats
    if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
      if(ssid.indexOf("Free") >= 0 || ssid.indexOf("Guest") >= 0 || 
         ssid.indexOf("Public") >= 0 || ssid == "Free WiFi" ||
         ssid.indexOf("Portal") >= 0 || ssid.indexOf("Login") >= 0) {
        threatCount++;
      }
    }
    
    // Check for suspicious names
    if(ssid.length() == 0 || ssid.length() < 3) {
      threatCount++;
    }
  }
  
  scanComplete = true;
}

void showResults() {
  Disp.fillScreen(BLACK);
  Disp.setTextColor(CYAN);
  Disp.setTextSize(2);
  Disp.setCursor(10, 10);
  Disp.println("RESULTS");
  
  Disp.setTextSize(1);
  Disp.setTextColor(WHITE);
  Disp.setCursor(10, 40);
  Disp.println("Networks found: " + String(networksFound));
  
  Disp.setCursor(10, 60);
  if(threatCount == 0) {
    Disp.setTextColor(GREEN);
    Disp.println("AREA CLEAN");
    Disp.setTextColor(WHITE);
    Disp.println("No threats detected");
  } else {
    Disp.setTextColor(RED);
    Disp.println("THREATS FOUND: " + String(threatCount));
    Disp.setTextColor(ORANGE);
    Disp.println("Suspicious networks detected!");
  }
  
  // Action buttons
  Disp.setTextColor(GREEN);
  Disp.setCursor(10, 120);
  Disp.println("A = RESCAN");
  Disp.setCursor(10, 135);
  Disp.println("C = COUNTER ATTACK");
  
  Disp.setTextColor(RED);
  Disp.setCursor(10, 180);
  Disp.println("B = EXIT TO MENU");
}

void handleResults() {
  if (M5.BtnB.wasClicked()) {
    // EXIT BUTTON - return to main menu
    currentState = MAIN_MENU;
    showMainMenu();
  }
  
  if (M5.BtnA.wasClicked()) {
    // RESCAN BUTTON
    currentState = SCANNING;
    startThreatScan();
  }
  
  if (M5.BtnC.wasClicked()) {
    // COUNTER ATTACK
    currentState = COUNTER_ATTACK_ACTIVE;
    startCounterAttack();
  }
}

void startCounterAttack() {
  Disp.fillScreen(BLACK);
  Disp.setTextColor(RED);
  Disp.setTextSize(2);
  Disp.setCursor(10, 10);
  Disp.println("PORTAL");
  Disp.setCursor(10, 30);
  Disp.println("KILLER");
  
  Disp.setTextSize(1);
  Disp.setTextColor(WHITE);
  Disp.setCursor(10, 60);
  Disp.println("Scanning for evil portals...");
  
  // Scan for open networks (potential evil portals)
  WiFi.mode(WIFI_MODE_STA);
  int networks = WiFi.scanNetworks();
  targetCount = 0;
  
  // Find ALL open networks (any open network is a potential target)
  for(int i = 0; i < networks && targetCount < 10; i++) {
    if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
      String ssid = WiFi.SSID(i);
      if(ssid.length() > 0) { // Only non-empty SSIDs
        targetSSIDs[targetCount] = ssid;
        targetCount++;
      }
    }
  }
  
  Disp.setCursor(10, 80);
  Disp.setTextColor(GREEN);
  Disp.println("TARGETS FOUND: " + String(targetCount));
  
  if (targetCount > 0) {
    portalKillerActive = true;
    currentTargetIndex = 0;
    isConnectedToTarget = false;
    
    Disp.setTextColor(WHITE);
    Disp.println("Starting portal attacks...");
    Disp.println("Will spam each portal 50x");
    
    // Show first few targets
    for(int i = 0; i < min(3, targetCount); i++) {
      Disp.println("-> " + targetSSIDs[i]);
    }
  } else {
    Disp.setTextColor(YELLOW);
    Disp.println("No evil portals detected");
    Disp.println("Area appears clean!");
  }
  
  Disp.setTextColor(RED);
  Disp.setCursor(10, 180);
  Disp.println("B = EXIT");
}

void handleCounterAttack() {
  if (M5.BtnB.wasClicked()) {
    // EXIT BUTTON - stop portal killer and return to menu
    portalKillerActive = false;
    if (isConnectedToTarget) {
      WiFi.disconnect();
      isConnectedToTarget = false;
    }
    currentState = MAIN_MENU;
    showMainMenu();
    return;
  }
  
  if (portalKillerActive && targetCount > 0) {
    // Try to connect to next target if not connected
    if (!isConnectedToTarget && millis() - lastConnectionAttempt > 5000) {
      if (currentTargetIndex < targetCount) {
        String currentTarget = targetSSIDs[currentTargetIndex];
        
        Disp.fillRect(10, 140, 120, 40, BLACK);
        Disp.setCursor(10, 140);
        Disp.setTextColor(YELLOW);
        Disp.println("Attacking:");
        Disp.println(currentTarget.substring(0, 12) + "...");
        
        WiFi.begin(currentTarget.c_str());
        lastConnectionAttempt = millis();
        
        // Check if connected after 3 seconds
        delay(3000);
        if (WiFi.status() == WL_CONNECTED) {
          isConnectedToTarget = true;
          spamPortal(currentTarget);
        } else {
          // Failed to connect, try next target
          currentTargetIndex++;
          if (currentTargetIndex >= targetCount) {
            // Completed all targets, restart cycle
            currentTargetIndex = 0;
          }
        }
      }
    }
    
    // Show status
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
      Disp.fillRect(10, 160, 120, 20, BLACK);
      Disp.setCursor(10, 160);
      if (isConnectedToTarget) {
        Disp.setTextColor(GREEN);
        Disp.println("SPAMMING PORTAL!");
      } else {
        Disp.setTextColor(CYAN);
        Disp.println("Target " + String(currentTargetIndex + 1) + "/" + String(targetCount));
      }
      lastUpdate = millis();
    }
  }
}

void spamPortal(String ssid) {
  Disp.fillRect(10, 140, 120, 40, BLACK);
  Disp.setCursor(10, 140);
  Disp.setTextColor(RED);
  Disp.println("CONNECTED!");
  Disp.setTextColor(GREEN);
  Disp.println("Spamming portal...");
  
  // Spam the portal 50 times
  for (int i = 0; i < 50; i++) {
    if (M5.BtnB.wasClicked()) break; // Allow exit during spam
    
    // Send HTTP POST to the portal gateway
    WiFiClient client;
    if (client.connect("192.168.4.1", 80)) {
      
      String postData = "username=Caught Ya Slippin&password=Ya Damn Fool&submit=login";
      
      client.println("POST /login HTTP/1.1");
      client.println("Host: 192.168.4.1");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Content-Length: " + String(postData.length()));
      client.println();
      client.println(postData);
      client.stop();
      
      // Also try common portal paths
      if (client.connect("192.168.4.1", 80)) {
        client.println("POST /post HTTP/1.1");
        client.println("Host: 192.168.4.1");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.println("Content-Length: " + String(postData.length()));
        client.println();
        client.println(postData);
        client.stop();
      }
    }
    
    // Show progress
    if (i % 10 == 0) {
      Disp.fillRect(10, 160, 120, 10, BLACK);
      Disp.setCursor(10, 160);
      Disp.setTextColor(CYAN);
      Disp.println("Spam: " + String(i + 1) + "/50");
    }
    
    delay(500); // Delay between requests - keeps device stable
  }
  
  Disp.fillRect(10, 140, 120, 30, BLACK);
  Disp.setCursor(10, 140);
  Disp.setTextColor(GREEN);
  Disp.println("PORTAL NEUTRALIZED!");
  
  // Disconnect and move to next target
  WiFi.disconnect();
  isConnectedToTarget = false;
  currentTargetIndex++;
  
  if (currentTargetIndex >= targetCount) {
    currentTargetIndex = 0; // Restart cycle
  }
  
  delay(2000);
}

void handleCaptivePortal() {
  // Not used in active portal killer mode
}

void stopCounterAttack() {
  portalKillerActive = false;
  if (isConnectedToTarget) {
    WiFi.disconnect();
    isConnectedToTarget = false;
  }
  WiFi.mode(WIFI_MODE_STA);
}

void activateShieldMode() {
  Disp.fillScreen(BLACK);
  Disp.setTextColor(BLUE);
  Disp.setTextSize(2);
  Disp.setCursor(10, 40);
  Disp.println("SHIELD");
  Disp.setCursor(10, 60);
  Disp.println("ACTIVE");
  
  WiFi.mode(WIFI_OFF);
  
  Disp.setTextSize(1);
  Disp.setTextColor(GREEN);
  Disp.setCursor(10, 100);
  Disp.println("All radios disabled");
  Disp.println("Device is now invisible");
  
  Disp.setTextColor(RED);
  Disp.setCursor(10, 180);
  Disp.println("B = EXIT SHIELD MODE");
  
  // Wait for exit
  while(true) {
    M5.update();
    if (M5.BtnB.wasClicked()) {
      WiFi.mode(WIFI_MODE_STA);
      showMainMenu();
      return;
    }
    delay(50);
  }
}
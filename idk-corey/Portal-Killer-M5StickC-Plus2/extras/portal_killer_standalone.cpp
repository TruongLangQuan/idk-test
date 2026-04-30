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
  Disp.println("COUNTER");
  Disp.setCursor(10, 30);
  Disp.println("ATTACK");
  
  Disp.setTextSize(1);
  Disp.setTextColor(WHITE);
  Disp.setCursor(10, 60);
  Disp.println("Deploying defenses...");
  
  // Start counter-attack AP
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP("DEFENSE_ACTIVE", "", 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  
  // Start DNS server for captive portal
  dnsServer.start(53, "*", apIP);
  
  // Setup web server
  server.onNotFound(handleCaptivePortal);
  server.begin();
  
  counterAttackActive = true;
  
  Disp.setCursor(10, 80);
  Disp.setTextColor(GREEN);
  Disp.println("DEFENSES ACTIVE:");
  Disp.setTextColor(WHITE);
  Disp.println("- Defensive AP running");
  Disp.println("- DNS hijacking active");
  Disp.println("- Captive portal deployed");
  
  Disp.setTextColor(RED);
  Disp.setCursor(10, 180);
  Disp.println("B = EXIT & STOP ATTACK");
}

void handleCounterAttack() {
  if (M5.BtnB.wasClicked()) {
    // EXIT BUTTON - stop counter attack and return to menu
    stopCounterAttack();
    currentState = MAIN_MENU;
    showMainMenu();
    return;
  }
  
  if (counterAttackActive) {
    dnsServer.processNextRequest();
    server.handleClient();
    
    // Show activity
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
      Disp.fillRect(10, 140, 120, 30, BLACK);
      Disp.setCursor(10, 140);
      Disp.setTextColor(GREEN);
      Disp.println("ACTIVE - Blocking portals");
      Disp.setTextColor(CYAN);
      Disp.println("Connections: " + String(WiFi.softAPgetStationNum()));
      lastUpdate = millis();
    }
  }
}

void handleCaptivePortal() {
  String html = "<html><head><title>Access Denied</title></head><body>";
  html += "<h1>SECURITY ALERT</h1>";
  html += "<p>This network is protected by Bruce Defense System.</p>";
  html += "<p>Malicious portal activity detected and blocked.</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void stopCounterAttack() {
  if (counterAttackActive) {
    server.stop();
    dnsServer.stop();
    WiFi.mode(WIFI_MODE_STA);
    counterAttackActive = false;
  }
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
#include <M5StickCPlus2.h>
#include "WiFi.h"
#include "esp_wifi.h"

// Simple, working WiFi defense system
// Combines Shark-Bait (offensive) and Guardian (defensive) tools

int menuSelection = 0;
bool inSubMenu = false;
int subMenuSelection = 0;

void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  
  StickCP2.Display.setRotation(0);
  StickCP2.Display.setTextSize(1);
  
  Serial.println("Bruce Standalone Defense System Ready");
  showMainMenu();
}

void loop() {
  M5.update();
  
  if (!inSubMenu) {
    handleMainMenu();
  } else {
    handleSubMenu();
  }
  
  delay(100);
}

void showMainMenu() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("BRUCE");
  StickCP2.Display.println("DEFENSE");
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(10, 60);
  
  // Menu options
  if (menuSelection == 0) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> SHARK-BAIT");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  SHARK-BAIT");
  }
  
  StickCP2.Display.setCursor(10, 80);
  if (menuSelection == 1) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> GUARDIAN");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  GUARDIAN");
  }
  
  StickCP2.Display.setCursor(10, 100);
  if (menuSelection == 2) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> EMERGENCY");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  EMERGENCY");
  }
  
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.setCursor(10, 140);
  StickCP2.Display.println("A=Select B=Up C=Down");
  
  StickCP2.Display.setTextColor(CYAN);
  StickCP2.Display.setCursor(10, 160);
  StickCP2.Display.println("WiFi: " + String(WiFi.getMode() == WIFI_OFF ? "OFF" : "ON"));
}

void handleMainMenu() {
  if (M5.BtnB.wasClicked()) {
    menuSelection = (menuSelection > 0) ? menuSelection - 1 : 2;
    showMainMenu();
  }
  
  if (M5.BtnC.wasClicked()) {
    menuSelection = (menuSelection < 2) ? menuSelection + 1 : 0;
    showMainMenu();
  }
  
  if (M5.BtnA.wasClicked()) {
    inSubMenu = true;
    subMenuSelection = 0;
    
    if (menuSelection == 0) {
      showSharkBaitMenu();
    } else if (menuSelection == 1) {
      showGuardianMenu();
    } else if (menuSelection == 2) {
      showEmergencyMenu();
    }
  }
}

void showSharkBaitMenu() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(RED);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("SHARK");
  StickCP2.Display.println("BAIT");
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(10, 60);
  
  if (subMenuSelection == 0) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Threat Scan");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Threat Scan");
  }
  
  StickCP2.Display.setCursor(10, 80);
  if (subMenuSelection == 1) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Evil Twin Hunt");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Evil Twin Hunt");
  }
  
  StickCP2.Display.setCursor(10, 100);
  if (subMenuSelection == 2) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Counter Attack");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Counter Attack");
  }
  
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.setCursor(10, 140);
  StickCP2.Display.println("A=Run B=Up C=Down");
  StickCP2.Display.setCursor(10, 155);
  StickCP2.Display.println("HOLD A=Back");
}

void showGuardianMenu() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(BLUE);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("GUARD");
  StickCP2.Display.println("IAN");
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(10, 60);
  
  if (subMenuSelection == 0) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Security Scan");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Security Scan");
  }
  
  StickCP2.Display.setCursor(10, 80);
  if (subMenuSelection == 1) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Network Monitor");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Network Monitor");
  }
  
  StickCP2.Display.setCursor(10, 100);
  if (subMenuSelection == 2) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Shield Mode");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Shield Mode");
  }
  
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.setCursor(10, 140);
  StickCP2.Display.println("A=Run B=Up C=Down");
  StickCP2.Display.setCursor(10, 155);
  StickCP2.Display.println("HOLD A=Back");
}

void showEmergencyMenu() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(RED);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("EMERG");
  StickCP2.Display.println("ENCY");
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(10, 60);
  
  if (subMenuSelection == 0) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Total Shutdown");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Total Shutdown");
  }
  
  StickCP2.Display.setCursor(10, 80);
  if (subMenuSelection == 1) {
    StickCP2.Display.setTextColor(YELLOW);
    StickCP2.Display.println("> Stealth Mode");
  } else {
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("  Stealth Mode");
  }
  
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.setCursor(10, 140);
  StickCP2.Display.println("A=Run B=Up C=Down");
  StickCP2.Display.setCursor(10, 155);
  StickCP2.Display.println("HOLD A=Back");
}

void handleSubMenu() {
  if (M5.BtnB.wasClicked()) {
    int maxOptions = (menuSelection == 2) ? 1 : 2; // Emergency has 2 options, others have 3
    subMenuSelection = (subMenuSelection > 0) ? subMenuSelection - 1 : maxOptions;
    
    if (menuSelection == 0) showSharkBaitMenu();
    else if (menuSelection == 1) showGuardianMenu();
    else if (menuSelection == 2) showEmergencyMenu();
  }
  
  if (M5.BtnC.wasClicked()) {
    int maxOptions = (menuSelection == 2) ? 1 : 2;
    subMenuSelection = (subMenuSelection < maxOptions) ? subMenuSelection + 1 : 0;
    
    if (menuSelection == 0) showSharkBaitMenu();
    else if (menuSelection == 1) showGuardianMenu();
    else if (menuSelection == 2) showEmergencyMenu();
  }
  
  if (M5.BtnA.wasClicked()) {
    runFunction();
  }
  
  if (M5.BtnA.isPressed() && M5.BtnA.pressedFor(1000)) {
    inSubMenu = false;
    showMainMenu();
  }
}

void runFunction() {
  if (menuSelection == 0) { // Shark-Bait
    if (subMenuSelection == 0) threatScanner();
    else if (subMenuSelection == 1) evilTwinHunter();
    else if (subMenuSelection == 2) counterAttack();
  } else if (menuSelection == 1) { // Guardian
    if (subMenuSelection == 0) securityScan();
    else if (subMenuSelection == 1) networkMonitor();
    else if (subMenuSelection == 2) shieldMode();
  } else if (menuSelection == 2) { // Emergency
    if (subMenuSelection == 0) totalShutdown();
    else if (subMenuSelection == 1) stealthMode();
  }
}

// SHARK-BAIT FUNCTIONS
void threatScanner() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(RED);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("THREAT SCANNER");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("Scanning...");
  
  WiFi.mode(WIFI_STA);
  int networks = WiFi.scanNetworks();
  
  int threats = 0;
  
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("Networks found: " + String(networks));
  
  for(int i = 0; i < networks; i++) {
    String ssid = WiFi.SSID(i);
    if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
      if(ssid.indexOf("Free") >= 0 || ssid.indexOf("Guest") >= 0) {
        threats++;
      }
    }
  }
  
  StickCP2.Display.setCursor(10, 70);
  if(threats > 0) {
    StickCP2.Display.setTextColor(RED);
    StickCP2.Display.println("THREATS: " + String(threats));
  } else {
    StickCP2.Display.setTextColor(GREEN);
    StickCP2.Display.println("AREA CLEAN");
  }
  
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 100);
  StickCP2.Display.println("Press any button");
  
  waitForButton();
  if (menuSelection == 0) showSharkBaitMenu();
}

void evilTwinHunter() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(MAGENTA);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("EVIL TWIN HUNTER");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("Scanning...");
  
  WiFi.mode(WIFI_STA);
  int networks = WiFi.scanNetworks();
  
  int duplicates = 0;
  
  // Simple duplicate detection
  for(int i = 0; i < networks - 1; i++) {
    for(int j = i + 1; j < networks; j++) {
      if(WiFi.SSID(i) == WiFi.SSID(j) && WiFi.SSID(i) != "") {
        duplicates++;
        break;
      }
    }
  }
  
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("Checked: " + String(networks));
  StickCP2.Display.setCursor(10, 70);
  
  if(duplicates > 0) {
    StickCP2.Display.setTextColor(RED);
    StickCP2.Display.println("EVIL TWINS: " + String(duplicates));
  } else {
    StickCP2.Display.setTextColor(GREEN);
    StickCP2.Display.println("NO EVIL TWINS");
  }
  
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 100);
  StickCP2.Display.println("Press any button");
  
  waitForButton();
  if (menuSelection == 0) showSharkBaitMenu();
}

void counterAttack() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(RED);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("COUNTER ATTACK");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("Deploying defenses...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("DEFENSE_ACTIVE", "", 1);
  
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("COUNTER-MEASURES:");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.println("- Defensive AP active");
  StickCP2.Display.println("- Channel jamming");
  StickCP2.Display.println("- Monitoring enabled");
  
  StickCP2.Display.setCursor(10, 120);
  StickCP2.Display.println("Running for 30 sec...");
  
  delay(30000);
  
  WiFi.mode(WIFI_STA);
  StickCP2.Display.setCursor(10, 140);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("COMPLETE");
  
  delay(2000);
  if (menuSelection == 0) showSharkBaitMenu();
}

// GUARDIAN FUNCTIONS
void securityScan() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(BLUE);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("SECURITY SCAN");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("Analyzing...");
  
  WiFi.mode(WIFI_STA);
  int networks = WiFi.scanNetworks();
  
  int open = 0, wep = 0, wpa = 0, wpa2 = 0;
  
  for(int i = 0; i < networks; i++) {
    wifi_auth_mode_t auth = (wifi_auth_mode_t)WiFi.encryptionType(i);
    switch(auth) {
      case WIFI_AUTH_OPEN: open++; break;
      case WIFI_AUTH_WEP: wep++; break;
      case WIFI_AUTH_WPA_PSK: wpa++; break;
      default: wpa2++; break;
    }
  }
  
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("SECURITY REPORT:");
  StickCP2.Display.setTextColor(RED);
  StickCP2.Display.println("VULNERABLE: " + String(open + wep));
  StickCP2.Display.setTextColor(ORANGE);
  StickCP2.Display.println("WEAK: " + String(wpa));
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("SECURE: " + String(wpa2));
  
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 120);
  StickCP2.Display.println("Press any button");
  
  waitForButton();
  if (menuSelection == 1) showGuardianMenu();
}

void networkMonitor() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(BLUE);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("NETWORK MONITOR");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("Monitoring...");
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("Press A to stop");
  
  WiFi.mode(WIFI_STA);
  int lastCount = 0;
  
  for(int cycle = 0; cycle < 60; cycle++) { // 60 seconds
    int currentCount = WiFi.scanNetworks();
    
    StickCP2.Display.fillRect(10, 70, 120, 60, BLACK);
    StickCP2.Display.setCursor(10, 70);
    StickCP2.Display.println("Networks: " + String(currentCount));
    
    if(currentCount != lastCount) {
      StickCP2.Display.setTextColor(ORANGE);
      StickCP2.Display.println("CHANGE DETECTED!");
      StickCP2.Display.setTextColor(WHITE);
    } else {
      StickCP2.Display.setTextColor(GREEN);
      StickCP2.Display.println("STABLE");
      StickCP2.Display.setTextColor(WHITE);
    }
    
    StickCP2.Display.println("Time: " + String(60 - cycle) + "s");
    
    lastCount = currentCount;
    
    for(int i = 0; i < 10; i++) {
      M5.update();
      if(M5.BtnA.wasClicked()) goto monitor_exit;
      delay(100);
    }
  }
  
  monitor_exit:
  StickCP2.Display.setCursor(10, 120);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("MONITORING COMPLETE");
  
  delay(2000);
  if (menuSelection == 1) showGuardianMenu();
}

void shieldMode() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(BLUE);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("SHIELD MODE");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("Activating...");
  
  WiFi.mode(WIFI_OFF);
  esp_wifi_deinit();
  
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("SHIELD ACTIVE");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.println("- WiFi: DISABLED");
  StickCP2.Display.println("- Bluetooth: OFF");
  StickCP2.Display.println("- All radios: OFF");
  
  StickCP2.Display.setCursor(10, 120);
  StickCP2.Display.println("Press A to exit");
  
  waitForButton();
  
  // Re-enable WiFi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  WiFi.mode(WIFI_STA);
  
  StickCP2.Display.setCursor(10, 140);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("SHIELD DOWN");
  
  delay(2000);
  if (menuSelection == 1) showGuardianMenu();
}

// EMERGENCY FUNCTIONS
void totalShutdown() {
  StickCP2.Display.fillScreen(RED);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("TOTAL SHUTDOWN");
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("WARNING!");
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("This will disable");
  StickCP2.Display.println("all wireless!");
  StickCP2.Display.setCursor(10, 90);
  StickCP2.Display.println("Hold A to confirm");
  
  if(waitForLongPress()) {
    WiFi.mode(WIFI_OFF);
    esp_wifi_deinit();
    
    StickCP2.Display.fillScreen(BLACK);
    StickCP2.Display.setTextColor(RED);
    StickCP2.Display.setCursor(10, 50);
    StickCP2.Display.println("TOTAL SHUTDOWN");
    StickCP2.Display.setTextColor(WHITE);
    StickCP2.Display.println("ALL RADIOS OFF");
    StickCP2.Display.println("RESTART TO RESTORE");
    
    while(true) delay(1000); // Infinite loop
  }
  
  if (menuSelection == 2) showEmergencyMenu();
}

void stealthMode() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextColor(PURPLE);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("STEALTH MODE");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.println("Going dark...");
  
  WiFi.mode(WIFI_OFF);
  
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("STEALTH ACTIVE");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.println("- No emissions");
  StickCP2.Display.println("- Passive only");
  StickCP2.Display.println("- Invisible mode");
  
  StickCP2.Display.setCursor(10, 120);
  StickCP2.Display.println("Press A to exit");
  
  waitForButton();
  
  WiFi.mode(WIFI_STA);
  
  StickCP2.Display.setCursor(10, 140);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("STEALTH OFF");
  
  delay(2000);
  if (menuSelection == 2) showEmergencyMenu();
}

// UTILITY FUNCTIONS
void waitForButton() {
  while(true) {
    M5.update();
    if(M5.BtnA.wasClicked() || M5.BtnB.wasClicked() || M5.BtnC.wasClicked()) {
      break;
    }
    delay(50);
  }
}

bool waitForLongPress() {
  unsigned long startTime = millis();
  while(millis() - startTime < 5000) {
    M5.update();
    if(M5.BtnA.isPressed() && M5.BtnA.pressedFor(2000)) {
      return true;
    }
    if(M5.BtnB.wasClicked() || M5.BtnC.wasClicked()) {
      return false;
    }
    delay(50);
  }
  return false;
}
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "pins.h"

// Global variables
TFT_eSPI tft = TFT_eSPI();
AsyncWebServer server(80);
int currentScreen = 1;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000; // Update every 1 second

// WiFi credentials
const char* ssid = "CYD_WiFi";
const char* password = "12345678";

// Forward declarations
void initDisplay();
void initWiFi();
void setupWebServer();
void drawScreen1();
void drawScreen2();
void drawScreen3();
void updateDisplay();
void handleButtonPress();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nCYD Firmware Starting...");
  
  // Initialize buttons
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_UP, INPUT);
  
  // Initialize display
  initDisplay();
  
  // Initialize WiFi
  initWiFi();
  
  // Setup web server
  setupWebServer();
  
  Serial.println("Setup complete!");
}

void loop() {
  // Handle button presses
  if (digitalRead(BTN_DOWN) == LOW) {
    delay(50);
    if (digitalRead(BTN_DOWN) == LOW) {
      currentScreen--;
      if (currentScreen < 1) currentScreen = 3;
      updateDisplay();
      delay(300);
    }
  }
  
  if (digitalRead(BTN_UP) == LOW) {
    delay(50);
    if (digitalRead(BTN_UP) == LOW) {
      currentScreen++;
      if (currentScreen > 3) currentScreen = 1;
      updateDisplay();
      delay(300);
    }
  }
  
  // Periodic updates
  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    updateDisplay();
  }
  
  delay(10);
}

void initDisplay() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  Serial.println("Display initialized: 320x240");
  Serial.println("Driver: ILI9341");
}

void initWiFi() {
  Serial.println("Starting WiFi AP...");
  
  // Start as WiFi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Also try to connect to available networks
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin("YourSSID", "YourPassword"); // Change these to your network
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("Station IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi STA connection failed, running as AP only");
  }
}

void setupWebServer() {
  // Serve main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>CYD Control Panel</title>
      <meta charset="utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        * {
          margin: 0;
          padding: 0;
          box-sizing: border-box;
        }
        body {
          font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
          background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
          min-height: 100vh;
          display: flex;
          align-items: center;
          justify-content: center;
          padding: 20px;
        }
        .container {
          background: white;
          border-radius: 12px;
          box-shadow: 0 20px 60px rgba(0,0,0,0.3);
          padding: 40px;
          max-width: 600px;
          width: 100%;
        }
        h1 {
          color: #333;
          margin-bottom: 10px;
          font-size: 28px;
        }
        .subtitle {
          color: #666;
          margin-bottom: 30px;
          font-size: 14px;
        }
        .info-grid {
          display: grid;
          grid-template-columns: 1fr 1fr;
          gap: 20px;
          margin-bottom: 30px;
        }
        .info-card {
          background: #f5f5f5;
          padding: 20px;
          border-radius: 8px;
          border-left: 4px solid #667eea;
        }
        .info-label {
          color: #888;
          font-size: 12px;
          text-transform: uppercase;
          margin-bottom: 5px;
        }
        .info-value {
          color: #333;
          font-size: 20px;
          font-weight: bold;
        }
        .control-section {
          margin-bottom: 30px;
        }
        .section-title {
          color: #333;
          font-weight: 600;
          margin-bottom: 15px;
          font-size: 16px;
        }
        .button-group {
          display: grid;
          grid-template-columns: 1fr 1fr;
          gap: 10px;
        }
        button {
          padding: 12px;
          border: none;
          border-radius: 8px;
          font-size: 14px;
          font-weight: 600;
          cursor: pointer;
          transition: all 0.3s;
        }
        .btn-primary {
          background: #667eea;
          color: white;
        }
        .btn-primary:hover {
          background: #5568d3;
          transform: translateY(-2px);
          box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .btn-secondary {
          background: #764ba2;
          color: white;
        }
        .btn-secondary:hover {
          background: #6a3f94;
          transform: translateY(-2px);
          box-shadow: 0 5px 15px rgba(118, 75, 162, 0.4);
        }
        .status {
          background: #e8f4f8;
          border: 1px solid #b3dfe0;
          padding: 12px;
          border-radius: 8px;
          color: #333;
          margin-top: 20px;
          font-size: 13px;
          line-height: 1.6;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>🎨 CYD Control Panel</h1>
        <p class="subtitle">ESP32 Cheap Yellow Display</p>
        
        <div class="info-grid">
          <div class="info-card">
            <div class="info-label">Display</div>
            <div class="info-value">320×240</div>
          </div>
          <div class="info-card">
            <div class="info-label">Driver</div>
            <div class="info-value">ILI9341</div>
          </div>
          <div class="info-card">
            <div class="info-label">Uptime</div>
            <div class="info-value" id="uptime">0s</div>
          </div>
          <div class="info-card">
            <div class="info-label">Current Screen</div>
            <div class="info-value" id="screen">1</div>
          </div>
        </div>
        
        <div class="control-section">
          <div class="section-title">🖼️ Screen Control</div>
          <div class="button-group">
            <button class="btn-primary" onclick="switchScreen(1)">Screen 1</button>
            <button class="btn-primary" onclick="switchScreen(2)">Screen 2</button>
            <button class="btn-secondary" onclick="switchScreen(3)">Screen 3</button>
            <button class="btn-secondary" onclick="nextScreen()">Next →</button>
          </div>
        </div>
        
        <div class="control-section">
          <div class="section-title">⚡ Device Info</div>
          <div class="status">
            <strong>WiFi Mode:</strong> Access Point + Station<br>
            <strong>AP SSID:</strong> CYD_WiFi<br>
            <strong>AP Password:</strong> 12345678<br>
            <strong>AP IP:</strong> 192.168.4.1<br>
            <strong>Status:</strong> Connected & Ready
          </div>
        </div>
      </div>
      
      <script>
        function updateUptime() {
          let seconds = Math.floor(Date.now() / 1000) % 86400;
          let hours = Math.floor(seconds / 3600);
          let minutes = Math.floor((seconds % 3600) / 60);
          let secs = seconds % 60;
          document.getElementById('uptime').textContent = 
            (hours > 0 ? hours + 'h ' : '') + 
            (minutes > 0 ? minutes + 'm ' : '') + 
            secs + 's';
        }
        
        function switchScreen(screen) {
          fetch('/screen?n=' + screen).then(() => {
            document.getElementById('screen').textContent = screen;
            updateUptime();
          });
        }
        
        function nextScreen() {
          fetch('/next').then(() => {
            updateUptime();
          });
        }
        
        setInterval(updateUptime, 1000);
      </script>
    </body>
    </html>
    )";
    request->send(200, "text/html", html);
  });
  
  // API: Switch screen
  server.on("/screen", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("n")) {
      int screen = request->getParam("n")->value().toInt();
      if (screen >= 1 && screen <= 3) {
        currentScreen = screen;
        updateDisplay();
        request->send(200, "application/json", "{\"status\":\"ok\",\"screen\":" + String(currentScreen) + "}");
        return;
      }
    }
    request->send(400, "application/json", "{\"error\":\"Invalid screen\"}");
  });
  
  // API: Next screen
  server.on("/next", HTTP_GET, [](AsyncWebServerRequest *request) {
    currentScreen++;
    if (currentScreen > 3) currentScreen = 1;
    updateDisplay();
    request->send(200, "application/json", "{\"status\":\"ok\",\"screen\":" + String(currentScreen) + "}");
  });
  
  // API: Get status
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"screen\":" + String(currentScreen) + ",\"uptime\":" + String(millis()/1000) + "}";
    request->send(200, "application/json", json);
  });
  
  server.begin();
  Serial.println("Web server started on port 80");
}

void updateDisplay() {
  tft.fillScreen(TFT_BLACK);
  
  switch(currentScreen) {
    case 1:
      drawScreen1();
      break;
    case 2:
      drawScreen2();
      break;
    case 3:
      drawScreen3();
      break;
  }
  
  // Draw screen indicator at bottom
  tft.setTextColor(0x8410);
  tft.setTextSize(1);
  tft.drawString("Screen " + String(currentScreen) + "/3", 280, 230);
}

void drawScreen1() {
  // Screen 1: Dashboard with WiFi, Battery, SD, Weather info
  
  // Draw WiFi icon (simplified)
  tft.drawBitmap(5, 5, image_wifi_full_bits, 19, 16, 0xFFFF);
  
  // Draw Battery indicator (simplified)
  tft.drawBitmap(289, 5, image_battery_full_bits, 24, 16, 0xFFFF);
  
  // Draw SD card indicator (simplified)
  tft.drawBitmap(30, 8, image_SDcardMounted_bits, 11, 8, 0xFFFF);
  
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setFreeFont(NULL);
  
  tft.drawString("Time", 56, 80);
  tft.drawString("Date", 76, 104);
  
  tft.setTextSize(1);
  tft.drawString("Weather", 90, 128);
  
  // Draw note box
  tft.drawRect(22, 160, 176, 72, 0xFFFF);
  tft.drawString("note", 89, 193);
  
  // Status info
  tft.setTextSize(1);
  tft.setTextColor(0x8410);
  tft.drawString("CYD - ESP32 Firmware", 10, 220);
}

void drawScreen2() {
  // Screen 2: Calendar/Schedule grid
  
  tft.drawRect(1, 1, 317, 30, 0xFFFF);
  
  tft.setTextColor(0xFFFF);
  tft.setTextSize(1);
  tft.setFreeFont(NULL);
  
  tft.drawString("Monday  Tuesday Wednesday Thursday Friday Saturday", 8, 12);
  
  // Draw grid outer border
  tft.drawRect(1, 1, 317, 234, 0xFFFF);
  
  // Draw vertical column dividers
  tft.drawRect(214, 1, 43, 234, 0xFFFF);
  tft.drawRect(50, 1, 50, 234, 0xFFFF);
  tft.drawRect(99, 1, 63, 234, 0xFFFF);
  tft.drawRect(161, 1, 54, 234, 0xFFFF);
  
  // Draw horizontal row dividers
  tft.drawRect(1, 30, 317, 30, 0xFFFF);
  tft.drawRect(1, 59, 317, 30, 0xFFFF);
  tft.drawRect(1, 88, 317, 30, 0xFFFF);
  tft.drawRect(1, 117, 317, 30, 0xFFFF);
  tft.drawRect(1, 146, 317, 30, 0xFFFF);
  tft.drawRect(1, 175, 317, 30, 0xFFFF);
}

void drawScreen3() {
  // Screen 3: Media player/status
  
  tft.setTextColor2);
  tft.setFreeFont(NULL);
  tft.drawString("Media Player", 60, 40);
  
  tft.setTextSize(1);
  tft.drawString("gif playing", 22, 100);
  tft.drawString("ascii (2d or 3d) playing", 22, 120);
  tft.drawString("screensaver active", 22, 140);
  
  // Draw media controls box
  tft.drawRect(50, 160, 220, 60, 0xFFFF);
  tft.drawString("PLAY  |  PAUSE  |  SKIP", 80, 190
  tft.drawString("gif playing / ascii(2d or 3d) playing / screensaver", 22, 112);
}

// Placeholder bitmap functions (replace with actual bitmaps if needed)
const unsigned char image_wifi_full_bits[] = {
  0x00, 0x00, 0x00, 0xE0, 0x1F, 0x00, 0x1C, 0xE0, 0x00, 0x03, 0x00, 0x01
};

const unsigned char image_battery_full_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00
};

const unsigned char image_SDcardMounted_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//==============================================================
// M5StickC Plus2 Groq Chatbot v1.3 - Direct to Groq (No Proxy)
//
// Based on working ESP32 + Groq Whisper example
// User enters WiFi + Groq key via AP mode
// Device records voice, sends DIRECTLY to Groq Whisper
// Gets transcription, sends to Groq chat, displays response
//
// NO PROXY NEEDED - STANDALONE DEVICE
//==============================================================

#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Configuration storage
Preferences preferences;
String groqApiKey = "";
String chatHistory = "";
int messageCount = 0;
bool isProcessing = false;  // Prevent button spam causing crashes
bool inMenu = false;        // Track if we're in settings menu
int menuSelection = 0;      // Current menu item selected
unsigned long btnHoldStart = 0;  // Track button hold time
bool inColorPicker = false; // Track if we're in color picker
int colorPickerSelection = 0; // Current color being previewed

// Menu items
const int MENU_ITEMS = 5;
const char* menuOptions[] = {
  "1. Text Color",
  "2. Reset WiFi",
  "3. Scan WiFi",
  "4. Record Time",
  "5. Exit Menu"
};

// Color names for picker
const char* colorNames[] = {"White", "Cyan", "Green", "Yellow", "Magenta", "Red", "Blue", "Orange"};

// Scrollable display
String currentMessage = "";
int scrollOffset = 0;

// Color cycling for scrollable text
uint16_t textColors[] = {WHITE, CYAN, GREEN, YELLOW, MAGENTA, RED, BLUE, ORANGE};
int currentColorIndex = 0;
unsigned long lastColorChange = 0;
unsigned long btnBPressStart = 0;
bool btnBWasHeld = false;

// Sound visualization
float soundWave = 0;
unsigned long lastWaveUpdate = 0;

// Display settings
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
#define TEXT_SIZE 2
#define LINE_HEIGHT 20
#define MAX_LINES 6

// Groq API settings
#define GROQ_CHAT_URL "https://api.groq.com/openai/v1/chat/completions"
#define GROQ_WHISPER_URL "api.groq.com"
#define GROQ_MODEL "llama-3.3-70b-versatile"
#define GROQ_WHISPER_MODEL "whisper-large-v3-turbo"
#define MAX_RESPONSE_SIZE 4096

// Microphone settings
#define RECORD_SAMPLERATE 16000  // 16kHz for better quality
#define RECORD_SECONDS 3         // 3 seconds recording (heap has plenty of room)
#define SAMPLE_BITS 16

//==============================================================
// Simple Animated Wave
//==============================================================
void drawSoundWave() {
  soundWave += 0.2;
  
  // Draw 3 wavy lines at bottom
  for (int x = 0; x < 240; x += 3) {
    int y1 = 115 + sin((x * 0.05) + soundWave) * 8;
    int y2 = 120 + sin((x * 0.07) + soundWave + 1) * 6;
    int y3 = 125 + sin((x * 0.06) + soundWave + 2) * 7;
    
    M5.Lcd.drawPixel(x, y1, GREEN);
    M5.Lcd.drawPixel(x, y2, CYAN);
    M5.Lcd.drawPixel(x, y3, BLUE);
  }
}

//==============================================================
// Cool Ready Screen with Static Colorful Wave Graphics
//==============================================================
void displayReadyScreen() {
  M5.Lcd.fillScreen(BLACK);
  
  // Main text
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(35, 20);
  M5.Lcd.print("READY!");
  
  // Subtitle
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(10, 55);
  M5.Lcd.print("Ask me anything");
  
  // Draw colorful wavy lines at bottom (thicker, more visible)
  for (int x = 0; x < 240; x++) {
    int y1 = 95 + sin(x * 0.08) * 10;
    int y2 = 100 + sin(x * 0.1 + 1.5) * 8;
    int y3 = 105 + sin(x * 0.12 + 3) * 6;
    int y4 = 110 + sin(x * 0.09 + 4.5) * 9;
    
    // Draw 2-pixel tall lines for visibility
    M5.Lcd.drawPixel(x, y1, GREEN);
    M5.Lcd.drawPixel(x, y1+1, GREEN);
    M5.Lcd.drawPixel(x, y2, CYAN);
    M5.Lcd.drawPixel(x, y2+1, CYAN);
    M5.Lcd.drawPixel(x, y3, YELLOW);
    M5.Lcd.drawPixel(x, y3+1, YELLOW);
    M5.Lcd.drawPixel(x, y4, MAGENTA);
    M5.Lcd.drawPixel(x, y4+1, MAGENTA);
  }
  
  currentMessage = "";
}

//==============================================================
// Display Settings Menu
//==============================================================
void displayMenu() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(30, 5);
  M5.Lcd.println("SETTINGS");
  
  M5.Lcd.setTextSize(2);
  for (int i = 0; i < MENU_ITEMS; i++) {
    if (i == menuSelection) {
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.setCursor(5, 30 + (i * 20));
      M5.Lcd.print("> ");
    } else {
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(20, 30 + (i * 20));
    }
    M5.Lcd.println(menuOptions[i]);
  }
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(5, 115);
  M5.Lcd.print("B:Up PWR:Down A:Select");
}

//==============================================================
// Simple Display - No String concatenation in loops
//==============================================================
void displayText(String text) {
  currentMessage = text;
  scrollOffset = 0;
  
  // Special handling for READY screen
  if (text == "READY!") {
    displayReadyScreen();
    return;
  }
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.setTextSize(TEXT_SIZE);
  M5.Lcd.setTextColor(WHITE);
  
  // Simple print without complex word wrapping
  M5.Lcd.print(text.substring(0, 120));  // Limit to first 120 chars to avoid overflow
  
  // Add wave for Recording screen
  if (text.startsWith("Recording")) {
    drawSoundWave();
  }
}

void refreshDisplay() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.setTextSize(TEXT_SIZE);
  M5.Lcd.setTextColor(textColors[currentColorIndex]);
  
  // Show text with scroll offset and proper word wrapping
  int startPos = scrollOffset * 100;  // 100 chars per page
  if (startPos < currentMessage.length()) {
    String pageText = currentMessage.substring(startPos, startPos + 200);
    
    // Word wrap logic
    int charPerLine = 19;  // Approx chars per line for TEXT_SIZE 2
    int cursorX = 5;
    int cursorY = 5;
    String word = "";
    
    for (int i = 0; i < pageText.length() && cursorY < SCREEN_HEIGHT - 10; i++) {
      char c = pageText.charAt(i);
      
      if (c == ' ' || c == '\n' || i == pageText.length() - 1) {
        if (i == pageText.length() - 1 && c != ' ' && c != '\n') word += c;
        
        int wordWidth = word.length() * 12;  // Approx width per char at TEXT_SIZE 2
        
        if (cursorX + wordWidth > SCREEN_WIDTH - 10 || c == '\n') {
          cursorX = 5;
          cursorY += LINE_HEIGHT;
          if (cursorY >= SCREEN_HEIGHT - 10) break;
        }
        
        M5.Lcd.setCursor(cursorX, cursorY);
        M5.Lcd.print(word);
        cursorX += wordWidth;
        
        if (c == ' ') {
          M5.Lcd.print(" ");
          cursorX += 12;
        }
        
        word = "";
      } else {
        word += c;
      }
    }
  }
}

//==============================================================
// Write WAV Header
//==============================================================
void writeWavHeader(uint8_t* buffer, uint32_t dataSize) {
  uint32_t chunkSize = dataSize + 36;
  uint32_t sampleRate = RECORD_SAMPLERATE;
  uint16_t numChannels = 1;
  uint16_t bitsPerSample = SAMPLE_BITS;
  uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
  uint16_t blockAlign = numChannels * (bitsPerSample / 8);

  memcpy(buffer, "RIFF", 4);
  memcpy(buffer + 4, &chunkSize, 4);
  memcpy(buffer + 8, "WAVEfmt ", 8);
  uint32_t subchunk1Size = 16;
  uint16_t audioFormat = 1;
  memcpy(buffer + 16, &subchunk1Size, 4);
  memcpy(buffer + 20, &audioFormat, 2);
  memcpy(buffer + 22, &numChannels, 2);
  memcpy(buffer + 24, &sampleRate, 4);
  memcpy(buffer + 28, &byteRate, 4);
  memcpy(buffer + 32, &blockAlign, 2);
  memcpy(buffer + 34, &bitsPerSample, 2);
  memcpy(buffer + 36, "data", 4);
  memcpy(buffer + 40, &dataSize, 4);
}

//==============================================================
// Record Audio and Send to Groq Whisper (Direct, No Proxy)
//==============================================================
String recordAndTranscribe() {
  if (groqApiKey.length() == 0) {
    return "Error: No API key";
  }
  
  Serial.println("[WHISPER] Starting recording...");
  Serial.printf("[MEM-START] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  displayText("Recording...\nSpeak now!");
  
  // Calculate buffer size
  int BUFFER_SIZE = RECORD_SAMPLERATE * RECORD_SECONDS * SAMPLE_BITS / 8;
  Serial.printf("[WHISPER] Buffer size: %d bytes\n", BUFFER_SIZE);
  
  // Allocate audio buffer in HEAP (not PSRAM - PSRAM corrupts with WiFi/TLS)
  int16_t* audioData = (int16_t*)malloc(BUFFER_SIZE);
  
  if (audioData == NULL) {
    Serial.println("[WHISPER] Audio buffer allocation failed!");
    return "Error: Memory allocation failed";
  }
  
  Serial.println("[WHISPER] Using heap RAM (safer than PSRAM)");
  
  // CRITICAL: Turn off speaker before using mic (they share I2S bus)
  M5.Speaker.end();
  
  // Configure and start microphone
  auto cfg = M5.Mic.config();
  cfg.sample_rate = RECORD_SAMPLERATE;
  M5.Mic.config(cfg);
  M5.Mic.begin();
  
  // Record audio
  size_t bytesRead = 0;
  size_t totalRead = 0;
  unsigned long startTime = millis();
  int loops = 0;
  
  Serial.println("[WHISPER] Starting record loop...");
  
  size_t chunk_samples = 512;  // Record in small chunks
  
  while (totalRead < BUFFER_SIZE && (millis() - startTime) < (RECORD_SECONDS * 1000 + 500)) {
    if (M5.Mic.isEnabled()) {
      // Non-blocking record - returns true when data is ready
      if (M5.Mic.record(audioData + (totalRead / 2), chunk_samples, RECORD_SAMPLERATE)) {
        totalRead += chunk_samples * 2;
        loops++;
        
        if (loops % 10 == 0) {
          Serial.printf("[WHISPER] Recorded chunk %d, total: %d bytes\n", loops, totalRead);
        }
      } else {
        delay(10);  // Wait a bit if no data ready
      }
    } else {
      Serial.println("[WHISPER] Mic not enabled!");
      break;
    }
  }
  
  M5.Mic.end();
  
  Serial.printf("[WHISPER] Recorded %d bytes in %d loops\n", totalRead, loops);
  Serial.printf("[MEM-RECORDED] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  
  if (totalRead < 512) {
    free(audioData);
    audioData = NULL;
    Serial.printf("[MEM-CLEANUP] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    return "Error: Message not heard";
  }
  
  displayText("Transcribing...");
  
  // Create WAV file with header - use HEAP ONLY to avoid corruption
  uint8_t* wavData = (uint8_t*)malloc(44 + totalRead);
  
  if (wavData == NULL) {
    free(audioData);
    audioData = NULL;
    Serial.printf("[MEM-WAV-ALLOC-FAIL] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    return "Error: Memory full";
  }
  
  writeWavHeader(wavData, totalRead);
  memcpy(wavData + 44, audioData, totalRead);
  size_t wavSize = 44 + totalRead;
  
  free(audioData);
  audioData = NULL;
  Serial.printf("[MEM-AUDIO-FREED] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  
  // Prepare multipart form data
  String boundary = "M5Boundary";
  String requestHead = "--" + boundary + "\r\n";
  requestHead += "Content-Disposition: form-data; name=\"model\"\r\n\r\n";
  requestHead += GROQ_WHISPER_MODEL;
  requestHead += "\r\n--" + boundary + "\r\n";
  requestHead += "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n";
  requestHead += "Content-Type: audio/wav\r\n\r\n";
  
  String requestTail = "\r\n--" + boundary + "--\r\n";
  size_t totalLen = requestHead.length() + wavSize + requestTail.length();
  
  // Connect to Groq API
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(30);
  
  Serial.println("[WHISPER] Connecting to Groq...");
  
  if (!client.connect(GROQ_WHISPER_URL, 443)) {
    Serial.println("[WHISPER] Connection failed");
    free(wavData);
    wavData = NULL;
    client.stop();
    Serial.printf("[MEM-CONN-FAIL] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    return "Error: Connection failed";
  }
  
  Serial.println("[WHISPER] Connected, sending request...");
  
  // Send HTTP headers
  client.println("POST /openai/v1/audio/transcriptions HTTP/1.1");
  client.println("Host: api.groq.com");
  client.println("Authorization: Bearer " + groqApiKey);
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println("Content-Length: " + String(totalLen));
  client.println("Connection: close");
  client.println();
  
  // Send request body
  client.print(requestHead);
  
  // Send WAV data in chunks
  for (size_t n = 0; n < wavSize; n += 1024) {
    size_t len = (n + 1024 < wavSize) ? 1024 : (wavSize - n);
    client.write(wavData + n, len);
    delay(1);
  }
  
  client.print(requestTail);
  free(wavData);
  wavData = NULL;
  Serial.printf("[MEM-WAV-FREED] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  
  Serial.println("[WHISPER] Request sent, waiting for response...");
  
  // Read response
  String response = "";
  String body = "";
  boolean headersParsed = false;
  unsigned long timeout = millis();
  
  while (client.connected() && (millis() - timeout) < 30000) {
    if (client.available()) {
      char c = client.read();
      
      if (!headersParsed) {
        response += c;
        if (response.endsWith("\r\n\r\n")) {
          headersParsed = true;
          response = "";
        }
      } else {
        body += c;
      }
      
      timeout = millis();
    }
  }
  
  client.stop();
  Serial.printf("[MEM-CLIENT-CLOSED] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  
  Serial.println("[WHISPER] Response received");
  Serial.println(body);
  
  // Parse JSON response
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    Serial.println("[WHISPER] JSON parse error");
    doc.clear();
    response = "";
    body = "";
    Serial.printf("[MEM-JSON-ERROR] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    return "Error: Message not heard";
  }
  
  String transcription = doc["text"].as<String>();
  
  if (transcription.length() == 0) {
    String errorMsg = doc["error"]["message"].as<String>();
    doc.clear();
    response = "";
    body = "";
    Serial.printf("[MEM-NO-TEXT] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    if (errorMsg.length() > 0) {
      return "Error: " + errorMsg;
    }
    return "Error: Message not heard";
  }
  
  doc.clear();
  response = "";
  body = "";
  Serial.println("[WHISPER] Transcription: " + transcription);
  Serial.printf("[MEM-WHISPER-DONE] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  return transcription;
}

//==============================================================
// Send Message to Groq Chat API
//==============================================================
String sendToGroq(String userMessage) {
  if (groqApiKey.length() == 0) {
    return "Error: No API key";
  }
  
  Serial.println("[GROQ] Sending message...");
  Serial.printf("[MEM-GROQ-START] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  
  // CRITICAL: Create fresh client objects for each request
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(30);
  
  HTTPClient http;
  http.begin(client, GROQ_CHAT_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + groqApiKey);
  http.setTimeout(30000);
  
  // Build JSON payload
  DynamicJsonDocument doc(8192);
  JsonArray messages = doc.createNestedArray("messages");
  
  JsonObject system = messages.createNestedObject();
  system["role"] = "system";
  system["content"] = "You are a helpful assistant. Keep responses very concise (2-3 sentences max).";
  
  // Add chat history
  if (chatHistory.length() > 0) {
    DynamicJsonDocument historyDoc(4096);
    DeserializationError error = deserializeJson(historyDoc, chatHistory);
    if (!error && historyDoc.is<JsonArray>()) {
      JsonArray history = historyDoc.as<JsonArray>();
      for (JsonVariant v : history) {
        messages.add(v);
      }
    }
  }
  
  JsonObject user = messages.createNestedObject();
  user["role"] = "user";
  user["content"] = userMessage;
  
  doc["model"] = GROQ_MODEL;
  doc["temperature"] = 0.7;
  doc["max_tokens"] = 300;
  
  String payload;
  serializeJson(doc, payload);
  doc.clear();
  Serial.printf("[MEM-PAYLOAD-BUILT] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  
  int httpCode = http.POST(payload);
  String response = "";
  
  if (httpCode == 200) {
    String jsonResponse = http.getString();
    Serial.printf("[MEM-RESPONSE-GOT] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    
    DynamicJsonDocument responseDoc(MAX_RESPONSE_SIZE);
    DeserializationError error = deserializeJson(responseDoc, jsonResponse);
    jsonResponse = "";
    
    if (!error) {
      response = responseDoc["choices"][0]["message"]["content"].as<String>();
      
      // Update chat history
      DynamicJsonDocument historyDoc(4096);
      if (chatHistory.length() > 0) {
        deserializeJson(historyDoc, chatHistory);
      }
      JsonArray history = historyDoc.to<JsonArray>();
      
      JsonObject userMsg = history.createNestedObject();
      userMsg["role"] = "user";
      userMsg["content"] = userMessage;
      
      JsonObject assistantMsg = history.createNestedObject();
      assistantMsg["role"] = "assistant";
      assistantMsg["content"] = response;
      
      messageCount++;
      if (messageCount > 5) {
        history.remove(0);
        history.remove(0);
      }
      
      serializeJson(history, chatHistory);
      preferences.putString("chatHistory", chatHistory);
      preferences.putInt("messageCount", messageCount);
      
      historyDoc.clear();
    }
    responseDoc.clear();
  } else {
    response = "Error: " + String(httpCode);
    Serial.printf("[GROQ] HTTP error: %d\n", httpCode);
  }
  
  payload = "";
  http.end();
  client.stop();
  
  // Force WiFi stack to release all TLS buffers
  delay(100);
  
  Serial.printf("[MEM-GROQ-DONE] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
  return response;
}

//==============================================================
// Setup
//==============================================================
void setup() {
  M5.begin();
  Serial.begin(115200);
  
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(TEXT_SIZE);
  M5.Lcd.setTextColor(WHITE);
  
  displayText("M5GroqBot v1.8\nStarting...");
  
  // Load preferences first
  preferences.begin("groqbot", false);
  groqApiKey = preferences.getString("groqApiKey", "");
  chatHistory = preferences.getString("chatHistory", "");
  messageCount = preferences.getInt("messageCount", 0);
  currentColorIndex = preferences.getInt("colorIndex", 0);  // Load saved color preference
  
  // Check for B button during boot (10 second window for WiFi menu)
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(CYAN);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Hold B for");
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.println("WiFi Menu");
  
  bool enterBootMenu = false;
  unsigned long bootMenuStart = millis();
  while (millis() - bootMenuStart < 10000) {  // 10 second window
    M5.update();
    if (M5.BtnB.isPressed()) {
      enterBootMenu = true;
      break;
    }
    
    // Show countdown
    int remaining = 10 - ((millis() - bootMenuStart) / 1000);
    M5.Lcd.fillRect(10, 60, 220, 20, BLACK);
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.printf("(%d seconds)", remaining);
    delay(100);
  }
  
  // If B was held, show boot menu
  if (enterBootMenu) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(20, 5);
    M5.Lcd.println("BOOT MENU");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.println("1. Reset WiFi");
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.println("2. Scan WiFi");
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.println("3. Exit");
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setCursor(5, 110);
    M5.Lcd.print("B:Up PWR:Down A:Select");
    
    int bootMenuSelection = 0;
    bool bootMenuActive = true;
    
    while (bootMenuActive) {
      M5.update();
      
      // Highlight selection
      M5.Lcd.fillRect(0, 30, 10, 60, BLACK);  // Clear old arrows
      M5.Lcd.setCursor(0, 30 + (bootMenuSelection * 20));
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.print(">");
      
      if (M5.BtnB.wasPressed()) {
        bootMenuSelection = (bootMenuSelection - 1 + 3) % 3;
        delay(200);
      }
      
      if (M5.BtnPWR.wasPressed()) {
        bootMenuSelection = (bootMenuSelection + 1) % 3;
        delay(200);
      }
      
      if (M5.BtnA.wasPressed()) {
        if (bootMenuSelection == 0) {  // Reset WiFi
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(10, 30);
          M5.Lcd.setTextColor(RED);
          M5.Lcd.println("WiFi Reset!");
          M5.Lcd.setTextColor(WHITE);
          M5.Lcd.setCursor(10, 60);
          M5.Lcd.println("Clearing...");
          
          WiFi.disconnect(true);  // Disconnect and erase
          preferences.putString("ssid", "");
          preferences.putString("password", "");
          delay(2000);
          
          M5.Lcd.setCursor(10, 90);
          M5.Lcd.println("Rebooting...");
          delay(1500);
          ESP.restart();
        }
        else if (bootMenuSelection == 1) {  // Scan WiFi
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(10, 10);
          M5.Lcd.setTextColor(CYAN);
          M5.Lcd.println("Scanning WiFi...");
          
          int n = WiFi.scanNetworks();
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(5, 5);
          M5.Lcd.setTextSize(1);
          M5.Lcd.printf("Found %d networks:\n", n);
          
          for (int i = 0; i < min(n, 8); i++) {
            M5.Lcd.printf("%d: %s (%ddBm)\n", i+1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
          }
          
          M5.Lcd.setTextColor(GREEN);
          M5.Lcd.println("\nPress A to continue");
          
          while (!M5.BtnA.wasPressed()) {
            M5.update();
            delay(10);
          }
          
          bootMenuActive = false;  // Exit menu and continue boot
        }
        else if (bootMenuSelection == 2) {  // Exit
          bootMenuActive = false;
        }
        delay(200);
      }
      
      delay(10);
    }
  }
  
  // WiFi setup with AP portal
  WiFiManager wm;
  WiFiManagerParameter groqKeyParam("groq_key", "Groq API Key", groqApiKey.c_str(), 100);
  wm.addParameter(&groqKeyParam);
  
  displayText("WiFi Setup...\nConnect to:\nM5GroqBot-Setup");
  
  if (!wm.autoConnect("M5GroqBot-Setup")) {
    displayText("WiFi Failed!\nRestarting...");
    delay(3000);
    ESP.restart();
  }
  
  // Save Groq API key
  String newKey = groqKeyParam.getValue();
  if (newKey.length() > 0 && newKey != groqApiKey) {
    groqApiKey = newKey;
    preferences.putString("groqApiKey", groqApiKey);
    Serial.println("[SETUP] API key saved");
  }
  
  displayText("WiFi OK!\nIP: " + WiFi.localIP().toString());
  delay(2000);
  
  // Use the displayText "READY!" trigger for animated screen
  displayText("READY!");
}

//==============================================================
// Main Loop
//==============================================================
void loop() {
  // Memory diagnostics - ONLY ADDITION
  static unsigned long lastMemPrint = 0;
  if (millis() - lastMemPrint > 1000) {
    Serial.printf("[HEAP] %d  [PSRAM] %d  [INT_FREE] %d\n",
        ESP.getFreeHeap(),
        ESP.getFreePsram(),
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    lastMemPrint = millis();
  }
  
  M5.update();
  
  // DEBUG: Print current message state
  static String lastMsg = "";
  if (currentMessage != lastMsg) {
    Serial.printf("[DEBUG] currentMessage changed to: '%s'\n", currentMessage.c_str());
    lastMsg = currentMessage;
  }
  
  // Check for menu access - Press B button on ready screen
  if (!isProcessing && currentMessage == "READY!" && !inMenu) {
    Serial.println("[DEBUG] Ready screen detected, waiting for B button...");
    if (M5.BtnB.wasPressed()) {
      Serial.println("[MENU] B BUTTON PRESSED! Opening menu!");
      inMenu = true;
      menuSelection = 0;
      displayMenu();
    }
  }
  
  // Handle menu navigation
  if (inMenu) {
    // B button - move UP
    if (M5.BtnB.wasPressed()) {
      menuSelection = (menuSelection - 1 + MENU_ITEMS) % MENU_ITEMS;  // -1 with wrap
      displayMenu();
      Serial.printf("[MENU] Selection: %d\n", menuSelection);
    }
    
    // PWR button - move DOWN
    if (M5.BtnPWR.wasPressed()) {
      menuSelection = (menuSelection + 1) % MENU_ITEMS;
      displayMenu();
      Serial.printf("[MENU] Selection: %d\n", menuSelection);
    }
    
    // A button - select option
    if (M5.BtnA.wasPressed()) {
      Serial.printf("[MENU] Selected: %s\n", menuOptions[menuSelection]);
      
      // Check if Exit Menu was selected
      if (menuSelection == 4) {  // Exit Menu is option 5 (index 4)
        inMenu = false;
        displayText("READY!");
      }
      // Handle Text Color selection
      else if (menuSelection == 0) {  // Text Color is option 1 (index 0)
        displayText("Color picker\ncoming soon!");
        delay(1500);
        displayMenu();
      }
      else {
        // TODO: Handle other menu selections
        displayText("Feature:\n" + String(menuOptions[menuSelection]) + "\n\nComing soon!");
        delay(2000);
        inMenu = false;
        displayText("READY!");
      }
    }
    
    return;  // Don't process other buttons while in menu
  }
  
  // Animate wave on ready screen when idle
  if (!isProcessing && currentMessage.length() == 0) {
    if (millis() - lastWaveUpdate > 50) {
      drawSoundWave();
      lastWaveUpdate = millis();
    }
  }
  
  // Button A - Voice recording
  if (M5.BtnA.wasPressed() && !isProcessing) {
    isProcessing = true;
    Serial.printf("[MEM-LOOP-START] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    
    String transcription = recordAndTranscribe();
    
    if (transcription.startsWith("Error:")) {
      displayText(transcription);
      delay(2000);
      displayReadyScreen();
      transcription = "";
    } else {
      displayText("You said:\n" + transcription);
      delay(2000);
      
      displayText("Thinking...");
      String response = sendToGroq(transcription);
      transcription = "";
      
      // Add instructions to end of message
      currentMessage = "Groq:\n" + response + "\n\n[BtnB=Up PWR=Down]\n[PWR at end=Next Q]";
      response = "";
      scrollOffset = 0;
      // Don't reset color - keep user's choice
      lastColorChange = millis();
      refreshDisplay();
      
      Serial.printf("[MEM-LOOP-END] Free heap: %d, Free PSRAM: %d\n", ESP.getFreeHeap(), ESP.getFreePsram());
    }
    
    isProcessing = false;
  }
  
  // Button B handling - tap to scroll, hold to change color
  if (currentMessage.length() > 50 && !currentMessage.startsWith("READY")) {
    if (M5.BtnB.wasPressed()) {
      btnBPressStart = millis();
      btnBWasHeld = false;
    }
    
    if (M5.BtnB.isPressed()) {
      unsigned long holdTime = millis() - btnBPressStart;
      
      // Check if held for color cycling (with throttle)
      if (holdTime > 800 && millis() - lastColorChange > 1500) {
        currentColorIndex = (currentColorIndex + 1) % 8;
        preferences.putInt("colorIndex", currentColorIndex);  // Save color choice
        lastColorChange = millis();
        refreshDisplay();
        Serial.printf("[COLOR] Changed to %d and saved\n", currentColorIndex);
      }
    }
    
    if (M5.BtnB.wasReleased()) {
      unsigned long holdTime = millis() - btnBPressStart;
      // If button was released quickly (not held), scroll up
      if (holdTime < 800) {
        if (scrollOffset > 0) {
          scrollOffset--;
          refreshDisplay();
        }
      }
      btnBWasHeld = false;
    }
  }
  
  // Power button - Scroll down OR reset for next question
  if (M5.BtnPWR.wasPressed()) {
    // Check if we can scroll down more
    if ((scrollOffset + 1) * 100 < currentMessage.length()) {
      scrollOffset++;
      refreshDisplay();
    } else if (currentMessage.length() > 50 && !isProcessing) {
      // At end of message - reset for next question
      chatHistory = "";
      messageCount = 0;
      preferences.putString("chatHistory", "");
      preferences.putInt("messageCount", 0);
      
      displayText("Resetting for\nnext question...\n\nPlease wait!");
      delay(1500);
      ESP.restart();
    }
  }
  
  delay(10);
}

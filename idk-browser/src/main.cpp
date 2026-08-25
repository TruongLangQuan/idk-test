#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// 5-Way Tactile Switch Pins (GPIO G1, G2, G3, G8, G43)
constexpr int kPinSW_UP     = 1;
constexpr int kPinSW_DOWN   = 2;
constexpr int kPinSW_LEFT   = 3;
constexpr int kPinSW_RIGHT  = 8;
constexpr int kPinSW_CENTER = 43;

// Application States
enum AppState {
  STATE_SCANNING_WIFI,
  STATE_SELECTING_WIFI,
  STATE_WIFI_KEYBOARD,
  STATE_CONNECTING_WIFI,
  STATE_BROWSER_HOME,
  STATE_URL_KEYBOARD,
  STATE_FETCHING_PAGE,
  STATE_VIEWING_PAGE
};

AppState g_state = STATE_SCANNING_WIFI;

// Wi-Fi Variables
int g_wifi_count = 0;
String g_selected_ssid = "";
String g_wifi_password = "";
String g_wifi_error = "";
unsigned long g_wifi_timer = 0;

// Browser Variables
String g_input_url = "http://neverssl.com";
String g_page_title = "";
String g_page_content = "";
String g_raw_html = "";
bool g_view_raw = false;
int g_scroll_offset = 0;
int g_max_scroll = 0;
std::vector<String> g_wrapped_lines;

// History & Bookmarks
std::vector<String> g_bookmarks = {
  "http://neverssl.com",
  "http://retro.wtf",
  "http://httpbin.org/ip",
  "https://www.google.com"
};
int g_selected_bookmark = 0;

// Keyboard Layout
const char* kKeyboardChars[5][10] = {
  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
  {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
  {"A", "S", "D", "F", "G", "H", "J", "K", "L", "_"},
  {"Z", "X", "C", "V", "B", "N", "M", ".", "/", ":"},
  {"CAPS", "SPACE", "DEL", "OK", "BACK", "", "", "", "", ""}
};
int g_kb_x = 0;
int g_kb_y = 0;
bool g_kb_caps = false;
String* g_kb_target_string = nullptr;

// 5-way D-Pad states (for debouncing)
struct SwitchState {
  int pin;
  bool is_pressed;
  unsigned long last_press_ms;
};

SwitchState g_switches[] = {
  {kPinSW_UP,     false, 0},
  {kPinSW_DOWN,   false, 0},
  {kPinSW_LEFT,   false, 0},
  {kPinSW_RIGHT,  false, 0},
  {kPinSW_CENTER, false, 0}
};

// Functions declaration
void scanWifi();
void drawUI();
void wrapText(const String& text, int max_width);
void fetchWebPage(const String& url);
String parseHTML(const String& html);
bool checkSwitchPressed(int pin);

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  M5.Display.setRotation(1); // Landscape mode (135x240)
  M5.Display.setBrightness(180);
  M5.Display.fillScreen(TFT_BLACK);
  
  Serial.begin(115200);
  
  // Set up D-pad buttons
  for (auto& sw : g_switches) {
    pinMode(sw.pin, INPUT_PULLUP);
  }

  // Auto-connect to "Quan Le" Wi-Fi on boot
  Serial.println("[WiFi] Attempting auto-connect to 'Quan Le'...");
  WiFi.mode(WIFI_STA);
  WiFi.begin("Quan Le", "15032011");
  
  M5.Display.startWrite();
  M5.Display.fillScreen(0x0F0F1A);
  M5.Display.fillRect(0, 0, 240, 18, 0x1A1D3A);
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.drawString(" STICKS3 MINI BROWSER", 4, 3);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("AUTO CONNECTING...", 120, 55);
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.drawString("WiFi: Quan Le", 120, 75);
  M5.Display.drawRoundRect(40, 95, 160, 8, 4, TFT_CYAN);
  M5.Display.endWrite();

  unsigned long auto_wifi_start = millis();
  bool auto_success = false;
  while (millis() - auto_wifi_start < 6000) {
    if (WiFi.status() == WL_CONNECTED) {
      auto_success = true;
      break;
    }
    M5.Display.startWrite();
    M5.Display.fillRoundRect(42, 97, (millis() / 20) % 156, 4, 2, TFT_CYAN);
    M5.Display.endWrite();
    delay(50);
  }

  if (auto_success) {
    Serial.println("[WiFi] Auto-connect succeeded!");
    g_state = STATE_BROWSER_HOME;
    g_selected_bookmark = 0;
  } else {
    Serial.println("[WiFi] Auto-connect failed. Falling back to Wi-Fi scan.");
    WiFi.disconnect();
    scanWifi();
  }
  drawUI();
}

void loop() {
  M5.update();
  bool state_changed = false;

  // Poll 5-way tactile switches
  bool press_up = checkSwitchPressed(kPinSW_UP);
  bool press_down = checkSwitchPressed(kPinSW_DOWN);
  bool press_left = checkSwitchPressed(kPinSW_LEFT);
  bool press_right = checkSwitchPressed(kPinSW_RIGHT);
  bool press_center = checkSwitchPressed(kPinSW_CENTER);

  if (press_up || press_down || press_left || press_right || press_center) {
    state_changed = true;
  }

  // State Machine handling
  switch (g_state) {
    case STATE_SCANNING_WIFI:
      // Done automatically during setup, transit to selection
      break;

    case STATE_SELECTING_WIFI:
      if (press_down) {
        g_selected_bookmark++;
        if (g_selected_bookmark >= g_wifi_count) g_selected_bookmark = 0;
      } else if (press_up) {
        g_selected_bookmark--;
        if (g_selected_bookmark < 0) g_selected_bookmark = g_wifi_count - 1;
      } else if (press_center) {
        if (g_wifi_count > 0) {
          g_selected_ssid = WiFi.SSID(g_selected_bookmark);
          g_wifi_password = "";
          g_kb_target_string = &g_wifi_password;
          g_state = STATE_WIFI_KEYBOARD;
          g_kb_x = 0;
          g_kb_y = 0;
        }
      }
      break;

    case STATE_WIFI_KEYBOARD:
    case STATE_URL_KEYBOARD: {
      if (press_down) {
        g_kb_y++;
        if (g_kb_y > 4) g_kb_y = 0;
      } else if (press_up) {
        g_kb_y--;
        if (g_kb_y < 0) g_kb_y = 4;
      } else if (press_right) {
        g_kb_x++;
        if (g_kb_y == 4) {
          if (g_kb_x > 4) g_kb_x = 0;
        } else {
          if (g_kb_x > 9) g_kb_x = 0;
        }
      } else if (press_left) {
        g_kb_x--;
        if (g_kb_x < 0) {
          g_kb_x = (g_kb_y == 4) ? 4 : 9;
        }
      } else if (press_center) {
        const char* selected_char = kKeyboardChars[g_kb_y][g_kb_x];
        if (strcmp(selected_char, "CAPS") == 0) {
          g_kb_caps = !g_kb_caps;
        } else if (strcmp(selected_char, "SPACE") == 0) {
          *g_kb_target_string += " ";
        } else if (strcmp(selected_char, "DEL") == 0) {
          if (g_kb_target_string->length() > 0) {
            g_kb_target_string->remove(g_kb_target_string->length() - 1);
          }
        } else if (strcmp(selected_char, "BACK") == 0) {
          if (g_state == STATE_WIFI_KEYBOARD) {
            g_state = STATE_SELECTING_WIFI;
            g_selected_bookmark = 0;
          } else {
            g_state = STATE_BROWSER_HOME;
          }
        } else if (strcmp(selected_char, "OK") == 0) {
          if (g_state == STATE_WIFI_KEYBOARD) {
            g_state = STATE_CONNECTING_WIFI;
            WiFi.begin(g_selected_ssid.c_str(), g_wifi_password.c_str());
            g_wifi_timer = millis();
          } else {
            g_state = STATE_FETCHING_PAGE;
            state_changed = true;
            drawUI(); // Show loading immediately
            fetchWebPage(g_input_url);
          }
        } else {
          String val = String(selected_char);
          if (!g_kb_caps) {
            val.toLowerCase();
          }
          *g_kb_target_string += val;
        }
      }
      break;
    }

    case STATE_CONNECTING_WIFI:
      if (WiFi.status() == WL_CONNECTED) {
        g_state = STATE_BROWSER_HOME;
        g_selected_bookmark = 0;
        state_changed = true;
      } else if (millis() - g_wifi_timer > 15000) { // 15 seconds timeout
        WiFi.disconnect();
        g_wifi_error = "Connect Failed";
        g_state = STATE_SELECTING_WIFI;
        state_changed = true;
      }
      break;

    case STATE_BROWSER_HOME:
      if (press_down) {
        g_selected_bookmark++;
        if (g_selected_bookmark > (int)g_bookmarks.size()) g_selected_bookmark = 0; // Bookmarks + 1 for input box
      } else if (press_up) {
        g_selected_bookmark--;
        if (g_selected_bookmark < 0) g_selected_bookmark = g_bookmarks.size();
      } else if (press_center) {
        if (g_selected_bookmark == 0) { // URL input box selected
          g_kb_target_string = &g_input_url;
          g_state = STATE_URL_KEYBOARD;
          g_kb_x = 0;
          g_kb_y = 0;
        } else { // Bookmark selected
          g_input_url = g_bookmarks[g_selected_bookmark - 1];
          g_state = STATE_FETCHING_PAGE;
          state_changed = true;
          drawUI();
          fetchWebPage(g_input_url);
        }
      }
      break;

    case STATE_FETCHING_PAGE:
      // Handled synchronously or in a helper, returns immediately
      break;

    case STATE_VIEWING_PAGE:
      if (press_down) {
        if (g_scroll_offset < g_max_scroll) {
          g_scroll_offset++;
        }
      } else if (press_up) {
        if (g_scroll_offset > 0) {
          g_scroll_offset--;
        }
      } else if (press_left) { // Go back to Home
        g_state = STATE_BROWSER_HOME;
        g_selected_bookmark = 0;
      } else if (press_right) { // Toggle raw HTML view
        g_view_raw = !g_view_raw;
        wrapText(g_view_raw ? g_raw_html : g_page_content, 230);
        g_scroll_offset = 0;
      }
      break;
  }

  if (state_changed || g_state == STATE_CONNECTING_WIFI) {
    drawUI();
    delay(10);
  }
}

// Check switch pressed with software debouncing
bool checkSwitchPressed(int pin) {
  for (auto& sw : g_switches) {
    if (sw.pin == pin) {
      bool is_low = (digitalRead(pin) == LOW);
      if (is_low && !sw.is_pressed && (millis() - sw.last_press_ms > 220)) {
        sw.is_pressed = true;
        sw.last_press_ms = millis();
        return true;
      } else if (!is_low) {
        sw.is_pressed = false;
      }
    }
  }
  return false;
}

// Wi-Fi network scanning
void scanWifi() {
  g_state = STATE_SCANNING_WIFI;
  drawUI();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  g_wifi_count = WiFi.scanNetworks();
  g_state = STATE_SELECTING_WIFI;
  g_selected_bookmark = 0;
}

// Wrap text to fit the 135x240 screen boundary
void wrapText(const String& text, int max_width) {
  g_wrapped_lines.clear();
  int start = 0;
  int text_len = text.length();

  M5.Display.setTextSize(1);
  
  while (start < text_len) {
    int end = start;
    int last_space = -1;
    
    // Find how many characters fit on the screen width
    while (end < text_len) {
      char c = text[end];
      if (c == '\n') {
        end++;
        break;
      }
      
      String line = text.substring(start, end + 1);
      int w = M5.Display.textWidth(line.c_str());
      if (w > max_width) {
        // We exceeded line width, wrap at space if found
        if (last_space != -1) {
          end = last_space + 1;
        }
        break;
      }
      
      if (c == ' ') {
        last_space = end;
      }
      end++;
    }
    
    if (end == start) {
      // Single character is wider than screen, force wrap
      end = start + 1;
    }
    
    String final_line = text.substring(start, end);
    final_line.replace("\n", "");
    g_wrapped_lines.push_back(final_line);
    start = end;
  }
  
  g_max_scroll = g_wrapped_lines.size() - 11; // 11 lines fit on display comfortably
  if (g_max_scroll < 0) g_max_scroll = 0;
}

// Fetch web page over HTTP/HTTPS
void fetchWebPage(const String& url) {
  HTTPClient http;
  WiFiClientSecure secure_client;
  WiFiClient standard_client;
  
  // Set insecure mode to bypass SSL certification errors on ESP32
  secure_client.setInsecure();
  
  Serial.println("[Browser] Fetching: " + url);
  bool is_https = url.startsWith("https://");
  
  bool begin_ok = false;
  if (is_https) {
    begin_ok = http.begin(secure_client, url);
  } else {
    begin_ok = http.begin(standard_client, url);
  }
  
  if (!begin_ok) {
    g_page_title = "ERROR";
    g_page_content = "Failed to parse URL/Establish Connection.";
    g_raw_html = g_page_content;
    g_state = STATE_VIEWING_PAGE;
    g_view_raw = false;
    wrapText(g_page_content, 230);
    g_scroll_offset = 0;
    return;
  }
  
  http.setTimeout(8000); // 8 seconds timeout
  int http_code = http.GET();
  
  if (http_code > 0) {
    g_raw_html = http.getString();
    g_page_title = url;
    g_page_content = parseHTML(g_raw_html);
  } else {
    g_page_title = "HTTP ERROR";
    g_page_content = "Error Code: " + String(http_code) + "\n\nDetail: " + http.errorToString(http_code);
    g_raw_html = g_page_content;
  }
  
  http.end();
  g_state = STATE_VIEWING_PAGE;
  g_view_raw = false;
  wrapText(g_page_content, 230);
  g_scroll_offset = 0;
}

// Simple HTML text parser to strip out tags and render text
String parseHTML(const String& html) {
  String text = "";
  bool in_tag = false;
  bool in_script = false;
  bool in_style = false;
  
  int len = html.length();
  for (int i = 0; i < len; i++) {
    char c = html[i];
    
    // Detect tags
    if (c == '<') {
      in_tag = true;
      String tag_content = "";
      int next_i = i + 1;
      while (next_i < len && html[next_i] != '>') {
        tag_content += html[next_i];
        next_i++;
      }
      tag_content.toLowerCase();
      
      // Handle tag checks
      if (tag_content.startsWith("script")) in_script = true;
      else if (tag_content.startsWith("/script")) in_script = false;
      else if (tag_content.startsWith("style")) in_style = true;
      else if (tag_content.startsWith("/style")) in_style = false;
      else if (tag_content == "p" || tag_content == "div" || tag_content == "br" || tag_content == "tr" || tag_content == "/h1" || tag_content == "/h2" || tag_content == "/h3") {
        if (!in_script && !in_style) text += "\n";
      } else if (tag_content == "li") {
        if (!in_script && !in_style) text += "\n • ";
      }
      
      i = next_i;
      in_tag = false;
      continue;
    }
    
    if (!in_tag && !in_script && !in_style) {
      text += c;
    }
  }
  
  // Replace typical HTML entities
  text.replace("&nbsp;", " ");
  text.replace("&lt;", "<");
  text.replace("&gt;", ">");
  text.replace("&amp;", "&");
  text.replace("&quot;", "\"");
  text.replace("\r", "");
  
  // Remove consecutive blank lines
  String cleaned = "";
  int clean_len = text.length();
  bool prev_newline = false;
  for (int i = 0; i < clean_len; i++) {
    char c = text[i];
    if (c == '\n') {
      if (!prev_newline) {
        cleaned += c;
        prev_newline = true;
      }
    } else {
      cleaned += c;
      prev_newline = false;
    }
  }
  
  return cleaned;
}

// Draw screen UI according to current AppState
void drawUI() {
  M5.Display.startWrite();
  M5.Display.fillScreen(0x0F0F1A); // Sleek dark slate blue

  // 1. Draw Top Header Bar
  M5.Display.fillRect(0, 0, 240, 18, 0x1A1D3A); // Deep glassmorphic bar
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setTextDatum(top_left);
  M5.Display.drawString(" STICKS3 MINI BROWSER", 4, 3);
  
  // Wi-Fi and Battery status on right
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextDatum(top_right);
  if (WiFi.status() == WL_CONNECTED) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("WiFi:OK ", 235, 3);
  } else {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.drawString("WiFi:OFF ", 235, 3);
  }

  // Draw separator line
  M5.Display.drawFastHLine(0, 18, 240, 0x303550);

  // 2. Draw Screen Contents based on State
  switch (g_state) {
    case STATE_SCANNING_WIFI:
      M5.Display.setTextDatum(middle_center);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.drawString("SCANNING WI-FI...", 120, 68);
      M5.Display.drawRoundRect(40, 85, 160, 8, 4, TFT_CYAN);
      // Micro progress bar animation
      M5.Display.fillRoundRect(42, 87, (millis() / 20) % 156, 4, 2, TFT_CYAN);
      break;

    case STATE_SELECTING_WIFI: {
      M5.Display.setTextDatum(top_left);
      M5.Display.setTextColor(0xFFAC); // Bright orange-yellow
      M5.Display.drawString("SELECT WI-FI NETWORK", 8, 24);

      if (g_wifi_count == 0) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("No Networks Found.", 8, 50);
      } else {
        int start_idx = g_selected_bookmark - 2;
        if (start_idx < 0) start_idx = 0;
        
        for (int i = 0; i < 4; i++) {
          int idx = start_idx + i;
          if (idx >= g_wifi_count) break;

          int ypos = 42 + i * 18;
          if (idx == g_selected_bookmark) {
            M5.Display.fillRect(4, ypos - 2, 232, 16, 0x2A2E50); // Cursor highlight
            M5.Display.setTextColor(TFT_CYAN);
          } else {
            M5.Display.setTextColor(TFT_WHITE);
          }
          
          String ssid = WiFi.SSID(idx);
          if (ssid.length() > 22) ssid = ssid.substring(0, 20) + "..";
          M5.Display.drawString(ssid, 10, ypos);
          
          // Draw signal strength bars
          int32_t rssi = WiFi.RSSI(idx);
          uint16_t sigColor = TFT_GREEN;
          if (rssi < -80) sigColor = TFT_RED;
          else if (rssi < -65) sigColor = TFT_YELLOW;
          
          int bars = map(rssi, -100, -30, 1, 5);
          for (int b = 0; b < bars; b++) {
            M5.Display.fillRect(210 + b * 4, ypos + 8 - b, 2, b + 2, sigColor);
          }
        }
      }

      M5.Display.setTextColor(TFT_DARKGREY);
      M5.Display.setTextDatum(bottom_left);
      M5.Display.drawString("L/R:Scroll  CTR:Select SSID", 8, 132);
      break;
    }

    case STATE_WIFI_KEYBOARD:
    case STATE_URL_KEYBOARD: {
      M5.Display.setTextDatum(top_left);
      M5.Display.setTextColor(TFT_CYAN);
      if (g_state == STATE_WIFI_KEYBOARD) {
        M5.Display.drawString("SSID: " + g_selected_ssid, 8, 22);
        M5.Display.drawString("PASS: " + *g_kb_target_string + "|", 8, 33);
      } else {
        M5.Display.drawString("URL: " + *g_kb_target_string + "|", 8, 24);
      }

      // Draw Keyboard Grid
      for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 10; c++) {
          const char* val = kKeyboardChars[r][c];
          if (strcmp(val, "") == 0) continue;

          int kw = 20;
          if (strcmp(val, "CAPS") == 0) kw = 34;
          else if (strcmp(val, "SPACE") == 0) kw = 48;
          else if (strcmp(val, "DEL") == 0) kw = 30;
          else if (strcmp(val, "OK") == 0) kw = 28;
          else if (strcmp(val, "BACK") == 0) kw = 38;

          int xpos = 10 + c * 22;
          if (r == 4) { // Last row offsets due to larger keys
            if (c == 0) xpos = 10;
            else if (c == 1) xpos = 48;
            else if (c == 2) xpos = 100;
            else if (c == 3) xpos = 134;
            else if (c == 4) xpos = 166;
            else continue;
          }

          int ypos = 44 + r * 17;

          if (r == g_kb_y && c == g_kb_x) {
            M5.Display.fillRoundRect(xpos, ypos, kw, 16, 2, TFT_CYAN);
            M5.Display.setTextColor(TFT_BLACK);
          } else {
            M5.Display.drawRoundRect(xpos, ypos, kw, 16, 2, TFT_LIGHTGRAY);
            M5.Display.setTextColor(TFT_WHITE);
          }

          M5.Display.setTextDatum(top_center);
          String label = String(val);
          if (label.length() == 1 && !g_kb_caps) {
            label.toLowerCase();
          }
          M5.Display.drawString(label, xpos + kw / 2, ypos + 4);
        }
      }
      break;
    }

    case STATE_CONNECTING_WIFI:
      M5.Display.setTextDatum(middle_center);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.drawString("CONNECTING TO WI-FI...", 120, 55);
      M5.Display.setTextColor(TFT_CYAN);
      M5.Display.drawString(g_selected_ssid, 120, 75);
      
      M5.Display.drawRoundRect(40, 95, 160, 8, 4, TFT_CYAN);
      M5.Display.fillRoundRect(42, 97, (millis() / 30) % 156, 4, 2, TFT_CYAN);
      break;

    case STATE_BROWSER_HOME: {
      M5.Display.setTextDatum(top_left);
      M5.Display.setTextColor(TFT_CYAN);
      M5.Display.drawString("BROWSER DASHBOARD", 8, 22);

      // 1. URL Entry box
      int ypos = 38;
      if (g_selected_bookmark == 0) {
        M5.Display.fillRect(4, ypos - 2, 232, 18, 0x2A2E50);
        M5.Display.setTextColor(TFT_CYAN);
      } else {
        M5.Display.setTextColor(TFT_WHITE);
      }
      M5.Display.drawString("Go to URL / Search:", 8, ypos);
      M5.Display.setTextColor(TFT_YELLOW);
      M5.Display.drawString(g_input_url, 125, ypos);

      // Divider line
      M5.Display.drawFastHLine(4, 58, 232, 0x303550);

      // 2. Bookmarks List
      M5.Display.setTextColor(0xFFAC);
      M5.Display.drawString("BOOKMARKS & PRESETS", 8, 64);
      
      for (size_t i = 0; i < g_bookmarks.size(); i++) {
        int idx = i + 1;
        int bypos = 80 + i * 14;
        
        if (g_selected_bookmark == idx) {
          M5.Display.fillRect(4, bypos - 2, 232, 13, 0x2A2E50);
          M5.Display.setTextColor(TFT_CYAN);
        } else {
          M5.Display.setTextColor(TFT_WHITE);
        }
        M5.Display.drawString(String(idx) + ". " + g_bookmarks[i], 12, bypos);
      }

      M5.Display.setTextColor(TFT_DARKGREY);
      M5.Display.setTextDatum(bottom_left);
      M5.Display.drawString("UP/DN:Navigate  CTR:Open URL", 8, 132);
      break;
    }

    case STATE_FETCHING_PAGE: {
      M5.Display.setTextDatum(middle_center);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.drawString("FETCHING WEB PAGE...", 120, 50);
      M5.Display.setTextColor(TFT_CYAN);
      
      String displayUrl = g_input_url;
      if (displayUrl.length() > 28) displayUrl = displayUrl.substring(0, 26) + "..";
      M5.Display.drawString(displayUrl, 120, 70);

      M5.Display.drawRoundRect(40, 90, 160, 8, 4, TFT_CYAN);
      M5.Display.fillRoundRect(42, 92, (millis() / 15) % 156, 4, 2, TFT_CYAN);
      break;
    }

    case STATE_VIEWING_PAGE: {
      M5.Display.setTextDatum(top_left);
      
      // Draw sub-header / page title bar
      M5.Display.fillRect(0, 18, 240, 14, 0x0A2B4C); // Soft blue indicator
      M5.Display.setTextColor(TFT_CYAN);
      String label = g_view_raw ? "[HTML Source] " : "[Web Render] ";
      String page_url = g_page_title;
      if (page_url.length() > 22) page_url = page_url.substring(0, 20) + "..";
      M5.Display.drawString(label + page_url, 4, 20);

      // Content rendering area (y = 35 to 122)
      int max_visible_lines = 8;
      int y_start = 36;
      int line_height = 11;
      
      M5.Display.setTextColor(TFT_WHITE);
      for (int i = 0; i < max_visible_lines; i++) {
        int idx = g_scroll_offset + i;
        if (idx >= (int)g_wrapped_lines.size()) break;
        
        String line = g_wrapped_lines[idx];
        int draw_y = y_start + i * line_height;
        
        // Color headers yellow or lists green
        if (line.startsWith(" • ") || line.startsWith("• ")) {
          M5.Display.setTextColor(TFT_GREEN);
        } else if (line.startsWith("[HTTP") || line.startsWith("Error")) {
          M5.Display.setTextColor(TFT_RED);
        } else if (g_view_raw) {
          M5.Display.setTextColor(0xD7AC); // HTML Tag color (gold-ish)
        } else {
          M5.Display.setTextColor(TFT_WHITE);
        }
        
        M5.Display.drawString(line.c_str(), 6, draw_y);
      }

      // Draw simple scroll indicator bar on the right side
      if (g_max_scroll > 0) {
        int bar_height = 80;
        int handle_height = max(5, bar_height * max_visible_lines / (int)g_wrapped_lines.size());
        int handle_y = y_start + (bar_height - handle_height) * g_scroll_offset / g_max_scroll;
        M5.Display.fillRect(236, y_start, 2, bar_height, TFT_DARKGREY);
        M5.Display.fillRect(236, handle_y, 2, handle_height, TFT_CYAN);
      }

      // Footer
      M5.Display.drawFastHLine(0, 124, 240, 0x303550);
      M5.Display.setTextColor(TFT_DARKGREY);
      M5.Display.setTextDatum(bottom_left);
      M5.Display.drawString("L:Home  R:Toggle Raw  UP/DN:Scroll", 8, 132);
      break;
    }
  }

  M5.Display.endWrite();
}

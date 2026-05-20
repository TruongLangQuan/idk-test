#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SD.h>
#include <SPIFFS.h>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

// ─── 5-Way Switch Pins (Requested Mapping) ──────────────────────────
static constexpr int kPinUp     = 32;
static constexpr int kPinDown   = 33;
static constexpr int kPinLeft   = 25;
static constexpr int kPinRight  = 26;
static constexpr int kPinCenter = 0;

// ─── Hardware Config ───────────────────────────────────────────────
// SD Card disabled to avoid pin conflicts with 5-way switch (GPIO 0, 26)
constexpr int kSdCsPin = 14; 

// ─── UI Config ─────────────────────────────────────────────────────
constexpr int kMenuLines = 6;
static LGFX_Sprite g_sprite(&M5.Display); // Double buffering sprite
static bool g_telex_mode = false;
static String g_status = "Ready";

MemoryFontWrapper g_font_wrap(vi12_font, vi12_font_len);

// ─── App State ────────────────────────────────────────────────────
enum class ScreenState {
    MENU,
    AI_RESULT,
    WIFI_LIST
};

struct MenuItem {
    const char* title;
    void (*action)();
};

static String g_api_url = "";
static String g_api_key = "";
static bool g_api_key_loaded = false;
static bool g_sd_ready = false;
static bool g_spiffs_ready = false;

static int g_menu_idx = 0;
static int g_menu_scroll = 0;
static ScreenState g_screen = ScreenState::MENU;

// For AI Result Scrolling
static String g_ai_title = "";
static String g_ai_body = "";
static int g_ai_scroll_y = 0;

// For Wi-Fi Management
struct ScannedWifi {
    String ssid;
    int32_t rssi;
    bool open;
};
static std::vector<ScannedWifi> g_scanned_wifis;
static int g_wifi_idx = 0;
static int g_wifi_scroll = 0;

constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

struct SavedWifi {
    String ssid;
    String pass;
};
static std::vector<SavedWifi> g_saved_wifis_store;
static std::vector<KnownWifi> g_known_wifis_vec;

void loadWifisFromFile() {
    g_saved_wifis_store.clear();
    g_known_wifis_vec.clear();

    for (int i = 0; i < sizeof(kKnownWifis)/sizeof(kKnownWifis[0]); ++i) {
        g_known_wifis_vec.push_back(kKnownWifis[i]);
    }

    auto loadFs = [](fs::FS &fs) {
        if (!fs.exists("/wifi.txt")) return;
        File f = fs.open("/wifi.txt", FILE_READ);
        if (!f) return;
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.isEmpty()) continue;
            int colon = line.indexOf(':');
            if (colon > 0) {
                SavedWifi sw;
                sw.ssid = line.substring(0, colon);
                sw.pass = line.substring(colon + 1);
                g_saved_wifis_store.push_back(sw);
            } else {
                SavedWifi sw;
                sw.ssid = line;
                sw.pass = "";
                g_saved_wifis_store.push_back(sw);
            }
        }
        f.close();
    };

    if (g_spiffs_ready) loadFs(SPIFFS);
    else if (g_sd_ready) loadFs(SD);

    for (const auto& sw : g_saved_wifis_store) {
        g_known_wifis_vec.push_back({sw.ssid.c_str(), sw.pass.c_str()});
    }
}

void saveWifiToFile(const String& ssid, const String& pass) {
    for (const auto& sw : g_saved_wifis_store) {
        if (sw.ssid == ssid && sw.pass == pass) return;
    }
    for (int i = 0; i < sizeof(kKnownWifis)/sizeof(kKnownWifis[0]); ++i) {
        if (String(kKnownWifis[i].ssid) == ssid && String(kKnownWifis[i].pass) == pass) return;
    }

    SavedWifi sw = {ssid, pass};
    g_saved_wifis_store.push_back(sw);
    
    g_known_wifis_vec.clear();
    for (int i = 0; i < sizeof(kKnownWifis)/sizeof(kKnownWifis[0]); ++i) {
        g_known_wifis_vec.push_back(kKnownWifis[i]);
    }
    for (const auto& w : g_saved_wifis_store) {
        g_known_wifis_vec.push_back({w.ssid.c_str(), w.pass.c_str()});
    }

    auto saveFs = [](fs::FS &fs) {
        File f = fs.open("/wifi.txt", FILE_WRITE);
        if (!f) return;
        for (const auto& w : g_saved_wifis_store) {
            f.println(w.ssid + ":" + w.pass);
        }
        f.close();
    };

    if (g_spiffs_ready) saveFs(SPIFFS);
    else if (g_sd_ready) saveFs(SD);
}

// ─── Input Handling (Edge Detection) ───────────────────────────────
struct KeyState {
    int pin;
    bool last;
    bool pressed;
    bool held;
    uint32_t last_ms;
};

KeyState g_keys[] = {
    {kPinUp, true, false, false, 0},
    {kPinDown, true, false, false, 0},
    {kPinLeft, true, false, false, 0},
    {kPinRight, true, false, false, 0},
    {kPinCenter, true, false, false, 0},
};
enum { K_UP, K_DOWN, K_LEFT, K_RIGHT, K_CENTER };

void updateKeys() {
    uint32_t now = millis();
    for (auto& k : g_keys) {
        bool current = (digitalRead(k.pin) == LOW);
        k.pressed = (current && !k.last);
        k.last = current;
        
        // Auto-repeat for scrolling
        if (current) {
            if (now - k.last_ms > 150) {
                k.held = true;
                k.last_ms = now;
            } else {
                k.held = false;
            }
        } else {
            k.held = false;
            k.last_ms = now;
        }
    }
}

// ─── Helpers ───────────────────────────────────────────────────────

String decodeJsonStringFragment(const String& text) {
    String out;
    out.reserve(text.length());
    bool escaping = false;
    for (size_t i = 0; i < text.length(); ++i) {
        char ch = text[i];
        if (!escaping) {
            if (ch == '\\') escaping = true;
            else out += ch;
            continue;
        }
        escaping = false;
        switch (ch) {
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 't': out += '\t'; break;
            case '\\': case '"': case '/': out += ch; break;
            default: out += ch; break;
        }
    }
    return out;
}

String extractJsonStringField(const String& json, const char* key) {
    String marker = String("\"") + key + "\":";
    int start = json.indexOf(marker);
    if (start < 0) return "";
    
    int i = start + marker.length();
    while(i < (int)json.length() && (json[i] == ' ' || json[i] == '\t' || json[i] == '\n' || json[i] == '\r')) {
        i++;
    }
    if (i >= (int)json.length() || json[i] != '"') return "";
    i++;
    
    int contentStart = i;
    bool escaping = false;
    for (; i < (int)json.length(); ++i) {
        const char ch = json[i];
        if (escaping) { escaping = false; continue; }
        if (ch == '\\') { escaping = true; continue; }
        if (ch == '"') break;
    }
    if (i <= contentStart) return "";
    return decodeJsonStringFragment(json.substring(contentStart, i));
}

// ─── API & Files ──────────────────────────────────────────────────

bool loadApiKeyFromFile() {
    auto tryFile = [](fs::FS &fs) -> bool {
        if (!fs.exists("/api_key.txt")) return false;
        File f = fs.open("/api_key.txt", FILE_READ);
        if (!f) return false;
        String content = "";
        while (f.available()) {
            int c = f.read();
            if (c == '\n' || c == '\r') break;
            if (c > 0) content += (char)c;
        }
        f.close();
        content.trim();
        if (content.length() > 0) {
            g_api_key = content;
            g_api_key_loaded = true;
            if (g_api_key.startsWith("AIza")) {
                g_api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent";
            } else {
                g_api_url = "https://openrouter.ai/api/v1/chat/completions";
            }
            return true;
        }
        return false;
    };

    if (g_spiffs_ready && tryFile(SPIFFS)) return true;
    if (g_sd_ready && tryFile(SD)) return true;

    // Hardcoded fallback
    g_api_key = "AIzaSyCd-gqS6kX6TXFBg4DtpaB_mlsvMC7xz7o";
    g_api_key_loaded = true;
    g_api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent";
    return true;
}

String callApi(const String& prompt) {
    if (g_api_url.length() == 0 || !g_api_key_loaded) return "API key missing";
    if (WiFi.status() != WL_CONNECTED) return "WiFi disconnected";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setConnectTimeout(15000);
    http.setTimeout(30000);

    String url = g_api_url;
    bool isGemini = g_api_key.startsWith("AIza");
    if (isGemini) {
        url += "?key=" + g_api_key;
    }

    if (!http.begin(client, url)) return "HTTP begin fail";
    
    http.addHeader("Content-Type", "application/json");
    if (!isGemini) {
        http.addHeader("Authorization", String("Bearer ") + g_api_key);
        http.addHeader("HTTP-Referer", "https://idk-device.local");
        http.addHeader("X-Title", "idk-ai");
    }

    String escaped = prompt;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");

    String payload;
    if (isGemini) {
        payload = "{\"contents\":[{\"parts\":[{\"text\":\"" + escaped + "\"}]}]}";
    } else {
        payload = "{\"model\":\"openrouter/auto\",\"messages\":[{\"role\":\"user\",\"content\":\"" + escaped + "\"}],\"max_tokens\":256}";
    }

    int code = http.POST(payload);
    if (code <= 0) {
        String err = http.errorToString(code);
        http.end();
        return "HTTP error: " + String(code) + " " + err;
    }
    String resp = http.getString();
    http.end();

    String content = extractJsonStringField(resp, "content");
    if (content.isEmpty()) content = extractJsonStringField(resp, "text");
    if (!content.isEmpty()) return content;
    
    String msg = extractJsonStringField(resp, "message");
    if (!msg.isEmpty()) return "Error: " + msg;
    return (resp.length() > 300) ? resp.substring(0, 300) : resp;
}

// ─── Rendering (Sprite-based Anti-Flickering) ───────────────────────

void renderStart() {
    g_sprite.clear();
    g_sprite.setTextColor(TFT_WHITE);
    g_sprite.setTextSize(1);
}

void renderEnd() {
    g_sprite.pushSprite(0, 0);
}

// Forward declarations for actions
void actionAsk();
void actionWifiScan();
void actionStatus();

MenuItem kMenu[] = {
    {"Ask AI", actionAsk},
    {"Scan Wi-Fi", actionWifiScan},
    {"Status", actionStatus},
};
int kMenuCount = sizeof(kMenu) / sizeof(kMenu[0]);

void drawMenu() {
    renderStart();
    g_sprite.setTextColor(TFT_YELLOW);
    g_sprite.setCursor(4, 4);
    g_sprite.print("idk-ai");
    g_sprite.setTextColor(TFT_DARKGREY);
    g_sprite.setCursor(4, 16);
    g_sprite.print(g_status);

    int y = 32;
    for (int i = 0; i < kMenuLines; ++i) {
        int idx = g_menu_scroll + i;
        if (idx >= kMenuCount) break;
        bool sel = (idx == g_menu_idx);
        g_sprite.setTextColor(sel ? TFT_CYAN : TFT_WHITE);
        g_sprite.setCursor(8, y);
        g_sprite.print(sel ? "> " : "  ");
        g_sprite.print(kMenu[idx].title);
        y += 18;
    }
    renderEnd();
}

void drawAiResult() {
    renderStart();
    g_sprite.setTextColor(TFT_YELLOW);
    g_sprite.setCursor(4, 4);
    g_sprite.print(g_ai_title);

    g_sprite.setTextColor(TFT_WHITE);
    int y_cursor = 24 - g_ai_scroll_y;
    
    g_sprite.setClipRect(0, 20, 240, 115);
    g_sprite.setCursor(4, y_cursor);
    g_sprite.setTextWrap(true);
    g_sprite.print(g_ai_body);
    g_sprite.clearClipRect();

    g_sprite.setTextColor(TFT_DARKGREY);
    g_sprite.setCursor(4, 125);
    g_sprite.print("UP/DOWN: Scroll  PWR: Back");
    renderEnd();
}

void drawWifiList() {
    renderStart();
    g_sprite.setTextColor(TFT_CYAN);
    g_sprite.setCursor(4, 4);
    g_sprite.print("Wi-Fi Scan");

    int y = 24;
    for (int i = 0; i < 6; ++i) {
        int idx = g_wifi_scroll + i;
        if (idx >= (int)g_scanned_wifis.size()) break;
        bool sel = (idx == g_wifi_idx);
        g_sprite.setTextColor(sel ? TFT_YELLOW : TFT_WHITE);
        g_sprite.setCursor(8, y);
        g_sprite.printf("%c %-16s %ddBm", sel ? '>' : ' ', 
            g_scanned_wifis[idx].ssid.substring(0, 16).c_str(), 
            g_scanned_wifis[idx].rssi);
        y += 16;
    }
    g_sprite.setTextColor(TFT_DARKGREY);
    g_sprite.setCursor(4, 125);
    g_sprite.print("CTR: Connect  PWR: Back");
    renderEnd();
}

// ─── Actions ───────────────────────────────────────────────────────

void actionAsk() {
    String prompt = "";
    if (!keyboardInput(prompt, "Ask", false, true, g_telex_mode, 200, true)) return;
    if (prompt.isEmpty()) return;
    
    g_status = "AI is thinking...";
    drawMenu();
    String reply = callApi(prompt);
    
    g_ai_title = "AI Response";
    g_ai_body = reply;
    g_ai_scroll_y = 0;
    g_screen = ScreenState::AI_RESULT;
    drawAiResult();
}

void actionWifiScan() {
    g_status = "Scanning...";
    drawMenu();
    WiFi.disconnect();
    int n = WiFi.scanNetworks();
    g_scanned_wifis.clear();
    for (int i = 0; i < n; ++i) {
        g_scanned_wifis.push_back({WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN});
    }
    g_wifi_idx = 0;
    g_wifi_scroll = 0;
    g_screen = ScreenState::WIFI_LIST;
    g_status = "Scan complete";
}

void actionStatus() {
    String out = "WiFi: " + String((WiFi.status() == WL_CONNECTED) ? "OK" : "NO");
    out += "\nSSID: " + WiFi.SSID();
    out += "\nSD: " + String(g_sd_ready ? "OK" : "NO");
    out += "\nAPI Key: " + String(g_api_key_loaded ? "Loaded" : "Missing");
    
    g_ai_title = "System Status";
    g_ai_body = out;
    g_ai_scroll_y = 0;
    g_screen = ScreenState::AI_RESULT;
    drawAiResult();
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(3);
    M5.Display.setBrightness(180);
    
    g_sprite.setColorDepth(8);
    g_sprite.createSprite(M5.Display.width(), M5.Display.height());

    for (auto& k : g_keys) {
        pinMode(k.pin, INPUT_PULLUP);
    }

    // SPI and SD disabled due to pin conflict with 5-way switch (GPIO 0, 26)
    // SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
    // g_sd_ready = SD.begin(kSdCsPin, SPI);
    g_sd_ready = false;
    g_spiffs_ready = SPIFFS.begin(true);

    loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);
    loadApiKeyFromFile();
    loadWifisFromFile();

    WiFi.setAutoReconnect(true);
    autoConnectKnownWifi(g_known_wifis_vec.data(), g_known_wifis_vec.size(), g_status, 2000);
    drawMenu();
}

static uint32_t g_last_wifi_check = 0;

void loop() {
    M5.update();
    updateKeys();

    bool changed = false;

    // Auto-reconnect check every 10 seconds if not connected
    if (g_screen == ScreenState::MENU && WiFi.status() != WL_CONNECTED && millis() - g_last_wifi_check > 10000) {
        g_last_wifi_check = millis();
        autoConnectKnownWifi(g_known_wifis_vec.data(), g_known_wifis_vec.size(), g_status, 500);
        changed = true;
    }

    bool pwr = M5.BtnPWR.wasPressed();
    bool btnA = M5.BtnA.wasPressed();

    if (g_screen == ScreenState::MENU) {
        if (g_keys[K_UP].pressed || g_keys[K_UP].held) {
            g_menu_idx = (g_menu_idx - 1 + kMenuCount) % kMenuCount;
            if (g_menu_idx < g_menu_scroll) g_menu_scroll = g_menu_idx;
            if (g_menu_idx >= g_menu_scroll + kMenuLines) g_menu_scroll = g_menu_idx - kMenuLines + 1;
            changed = true;
        }
        else if (g_keys[K_DOWN].pressed || g_keys[K_DOWN].held) {
            g_menu_idx = (g_menu_idx + 1) % kMenuCount;
            if (g_menu_idx < g_menu_scroll) g_menu_scroll = g_menu_idx;
            if (g_menu_idx >= g_menu_scroll + kMenuLines) g_menu_scroll = g_menu_idx - kMenuLines + 1;
            changed = true;
        }
        else if (g_keys[K_CENTER].pressed || btnA) {
            kMenu[g_menu_idx].action();
            changed = true;
        }
        else if (M5.BtnB.wasPressed()) {
            g_menu_idx = (g_menu_idx + 1) % kMenuCount;
            changed = true;
        }
    } 
    else if (g_screen == ScreenState::AI_RESULT) {
        if (g_keys[K_UP].pressed || g_keys[K_UP].held) { g_ai_scroll_y = std::max(0, g_ai_scroll_y - 12); changed = true; }
        else if (g_keys[K_DOWN].pressed || g_keys[K_DOWN].held) { g_ai_scroll_y += 12; changed = true; }
        else if (pwr || btnA) { g_screen = ScreenState::MENU; changed = true; }
    }
    else if (g_screen == ScreenState::WIFI_LIST) {
        if (g_keys[K_UP].pressed || g_keys[K_UP].held) { 
            if (!g_scanned_wifis.empty()) {
                g_wifi_idx = (g_wifi_idx - 1 + g_scanned_wifis.size()) % g_scanned_wifis.size();
                if (g_wifi_idx < g_wifi_scroll) g_wifi_scroll = g_wifi_idx;
            }
            changed = true; 
        }
        else if (g_keys[K_DOWN].pressed || g_keys[K_DOWN].held) { 
            if (!g_scanned_wifis.empty()) {
                g_wifi_idx = (g_wifi_idx + 1) % g_scanned_wifis.size();
                if (g_wifi_idx >= g_wifi_scroll + 6) g_wifi_scroll = g_wifi_idx - 5;
            }
            changed = true; 
        }
        else if (g_keys[K_CENTER].pressed || btnA) {
            if (!g_scanned_wifis.empty()) {
                String ssid = g_scanned_wifis[g_wifi_idx].ssid;
                String pass = "";
                bool proceed = false;
                if (!g_scanned_wifis[g_wifi_idx].open) {
                    if (keyboardInput(pass, "Password", true, false, g_telex_mode)) {
                        WiFi.mode(WIFI_STA);
                        WiFi.disconnect(true, true);
                        delay(60);
                        WiFi.begin(ssid.c_str(), pass.c_str());
                        proceed = true;
                    }
                } else {
                    WiFi.mode(WIFI_STA);
                    WiFi.disconnect(true, true);
                    delay(60);
                    WiFi.begin(ssid.c_str());
                    proceed = true;
                }
                if (proceed) {
                    g_status = "Connecting...";
                    g_screen = ScreenState::MENU;
                    drawMenu();
                    
                    uint32_t start = millis();
                    while(millis() - start < 8000) {
                        M5.update();
                        if (WiFi.status() == WL_CONNECTED) {
                            saveWifiToFile(ssid, pass);
                            g_status = "WiFi Connected";
                            break;
                        }
                        delay(100);
                    }
                    if (WiFi.status() != WL_CONNECTED) {
                        g_status = "Connect failed";
                        WiFi.disconnect();
                    }
                }
            }
            g_screen = ScreenState::MENU;
            changed = true; 
        }
        else if (pwr) { g_screen = ScreenState::MENU; changed = true; }
    }

    if (changed) {
        switch(g_screen) {
            case ScreenState::MENU: drawMenu(); break;
            case ScreenState::AI_RESULT: drawAiResult(); break;
            case ScreenState::WIFI_LIST: drawWifiList(); break;
            default: break;
        }
    }

    delay(10);
}

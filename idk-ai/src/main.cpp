#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPIFFS.h>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

// TODO: set API endpoint + key when available.
// API will try to load from: /sd/api_key.txt → /SPIFFS/api_key.txt
static String g_api_url = "";    // OpenRouter API endpoint
static String g_api_key = "";    // API key (loaded from file)
static bool g_api_key_loaded = false;

bool g_telex_mode = false;
String g_status = "Ready";

MemoryFontWrapper g_font_wrap(vi12_font, vi12_font_len);

constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

int g_menu_idx = 0;
int g_menu_scroll = 0;

// ─── API key file loading ──────────────────────────────────────

bool loadApiKeyFromFile() {
  // Try SD first, then SPIFFS
  // File should contain exactly one API key (plain text, no quotes)
  
  // Try SD card
  bool sdExists = SD.exists("/api_key.txt");
  if (sdExists) {
    File f = SD.open("/api_key.txt", FILE_READ);
    if (f) {
      String content = "";
      while (f.available()) {
        int c = f.read();
        if (c == '\n' || c == '\r') break;  // stop at first newline
        if (c > 0) content += (char)c;
      }
      f.close();
      content.trim();
      if (content.length() > 0) {
        g_api_key = content;
        g_api_key_loaded = true;
        g_api_url = "https://openrouter.ai/api/v1/chat/completions";
        return true;
      }
    }
  }

  // Try SPIFFS
  bool spiffsExists = SPIFFS.exists("/api_key.txt");
  if (spiffsExists) {
    File f = SPIFFS.open("/api_key.txt", FILE_READ);
    if (f) {
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
        g_api_url = "https://openrouter.ai/api/v1/chat/completions";
        return true;
      }
    }
  }

  return false;
}

String callApi(const String& prompt) {
  if (g_api_url.length() == 0) {
    if (!g_api_key_loaded) {
      return "API key not loaded. Place api_key.txt on SD or SPIFFS.";
    }
    return "API not configured";
  }
  if (WiFi.status() != WL_CONNECTED) return "WiFi not connected";

  HTTPClient http;
  http.begin(g_api_url);
  http.addHeader("Content-Type", "application/json");
  if (g_api_key.length() > 0) {
    http.addHeader("Authorization", String("Bearer ") + g_api_key);
  }
  http.addHeader("HTTP-Referer", "https://idk-device.local");
  http.addHeader("X-Title", "idk-ai");

  // OpenRouter chat completions format
  String escaped = prompt;
  escaped.replace("\\", "\\\\");
  escaped.replace("\"", "\\\"");
  escaped.replace("\n", "\\n");

  String payload = "{\"model\":\"meta-llama/llama-3.1-8b-instruct:free\",\"messages\":[{\"role\":\"user\",\"content\":\"";
  payload += escaped;
  payload += "\"}],\"max_tokens\":256}";

  int code = http.POST(payload);
  if (code <= 0) {
    http.end();
    return String("HTTP error: ") + String(code);
  }

  String resp = http.getString();
  http.end();

  // Parse JSON response to extract message content
  // Format: {"choices":[{"message":{"content":"..."}}]}
  int contentStart = resp.indexOf("\"content\":\"");
  if (contentStart >= 0) {
    contentStart += 11;  // skip "content":"
    int contentEnd = resp.indexOf("\"", contentStart);
    // Handle escaped quotes
    while (contentEnd > 0 && resp[contentEnd - 1] == '\\') {
      contentEnd = resp.indexOf("\"", contentEnd + 1);
    }
    if (contentEnd > contentStart) {
      String content = resp.substring(contentStart, contentEnd);
      content.replace("\\n", "\n");
      content.replace("\\\"", "\"");
      content.replace("\\\\", "\\");
      if (content.length() > 400) content = content.substring(0, 400) + "...";
      return content;
    }
  }

  // Fallback: check for error
  int errStart = resp.indexOf("\"message\":\"");
  if (errStart >= 0) {
    errStart += 11;
    int errEnd = resp.indexOf("\"", errStart);
    if (errEnd > errStart) return "Error: " + resp.substring(errStart, errEnd);
  }

  if (resp.length() > 320) resp = resp.substring(0, 320);
  return resp;
}

void actionAsk() {
  String prompt = "";
  if (!keyboardInput(prompt, "Ask", false, true, g_telex_mode, 200, true)) return;
  if (prompt.isEmpty()) return;
  String reply = callApi(prompt);
  showResult("AI", reply);
}

void actionStatus() {
  String out = "WiFi: ";
  out += (WiFi.status() == WL_CONNECTED) ? "OK" : "NO";
  out += "\nAPI: ";
  out += g_api_key_loaded ? "LOADED" : "NOT LOADED";
  if (g_api_key_loaded) {
    out += "\nKey length: ";
    out += g_api_key.length();
    out += " chars";
  }
  showResult("Status", out);
}

MenuItem kMenu[] = {
    {"Ask AI", actionAsk},
    {"Status", actionStatus},
};
constexpr int kMenuCount = sizeof(kMenu) / sizeof(kMenu[0]);

void ensureScroll() {
  if (g_menu_idx < g_menu_scroll) g_menu_scroll = g_menu_idx;
  if (g_menu_idx >= g_menu_scroll + kMenuLines) g_menu_scroll = g_menu_idx - kMenuLines + 1;
}

void drawMenu() {
  ensureScroll();
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-ai");

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 14);
  M5.Display.print(g_status);

  int y = 28;
  for (int i = 0; i < kMenuLines; ++i) {
    int idx = g_menu_scroll + i;
    if (idx >= kMenuCount) break;
    bool sel = (idx == g_menu_idx);
    M5.Display.setTextColor(sel ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(4, y);
    M5.Display.print(sel ? "> " : "  ");
    M5.Display.print(kMenu[idx].title);
    y += 16;
  }

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 124);
  M5.Display.print("A:Select  B:Next  PWR:Prev");
}

void drawBoot(const char* msg) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-ai");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 24);
  M5.Display.print(msg);
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  M5.Display.setTextSize(1);
  drawBoot("Booting...");

  // Initialize file systems
  drawBoot("Mount SD/SPIFFS...");
  SD.begin();        // SD card (if available)
  SPIFFS.begin(true);  // SPIFFS with format if needed

  drawBoot("Loading font...");
  (void)loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);
  
  // Load API key from file
  drawBoot("Loading API key...");
  if (loadApiKeyFromFile()) {
    g_status = "API key loaded";
  } else {
    g_status = "No API key";
  }

  drawBoot("Connecting WiFi...");
  autoConnectKnownWifi(kKnownWifis, sizeof(kKnownWifis) / sizeof(kKnownWifis[0]), g_status, 3000);
  drawMenu();
}

void loop() {
  M5.update();
  if (M5.BtnB.wasPressed()) {
    g_menu_idx = (g_menu_idx + 1) % kMenuCount;
    drawMenu();
  }
  if (M5.BtnPWR.wasPressed()) {
    g_menu_idx = (g_menu_idx - 1 + kMenuCount) % kMenuCount;
    drawMenu();
  }
  if (M5.BtnA.wasPressed()) {
    kMenu[g_menu_idx].action();
    drawMenu();
  }
  delay(10);
}

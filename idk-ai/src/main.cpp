#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <idk_ui.h>
#include "generated/vi12_font.h"

namespace {

constexpr int kMenuLines = 6;

struct MenuItem {
  const char* title;
  void (*action)();
};

// TODO: set API endpoint + key when available.
static const char* kApiUrl = "";
static const char* kApiKey = "";

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

String callApi(const String& prompt) {
  if (strlen(kApiUrl) == 0) return "API not configured";
  if (WiFi.status() != WL_CONNECTED) return "WiFi not connected";

  HTTPClient http;
  http.begin(kApiUrl);
  http.addHeader("Content-Type", "application/json");
  if (strlen(kApiKey) > 0) {
    http.addHeader("Authorization", String("Bearer ") + kApiKey);
  }

  String payload = "{\"prompt\":\"" + prompt + "\"}";
  int code = http.POST(payload);
  if (code <= 0) {
    http.end();
    return "HTTP error";
  }
  String resp = http.getString();
  http.end();
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
  out += (strlen(kApiUrl) > 0) ? kApiUrl : "not set";
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

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);

  (void)loadFontFromMemory(vi12_font, vi12_font_len, g_font_wrap);
  autoConnectKnownWifi(kKnownWifis, sizeof(kKnownWifis) / sizeof(kKnownWifis[0]), g_status);
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

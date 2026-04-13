#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>

#ifndef TFT_I2C_POWER
#define TFT_I2C_POWER 21
#endif

static const char *kSsid = "dhe";
static const char *kPass = "dhe";
static const uint16_t kUdpPort = 4210;
static const uint16_t kFgColor = 0x07E0;
static const uint16_t kAccentColor = 0xF800;
static const uint16_t kBgColor = 0x0000;
static const int kHeaderH = 18;
static const int kFooterH = 14;

static TFT_eSPI tft = TFT_eSPI();
static WiFiUDP udp;
static String g_text = "";
static bool g_connected = false;
static uint32_t g_last_attempt_ms = 0;
static uint32_t g_last_hello_ms = 0;

static void enableDisplayPower() {
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
}

static void enableBacklight() {
#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  #ifdef TFT_BACKLIGHT_ON
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  #else
  digitalWrite(TFT_BL, HIGH);
  #endif
#endif
}

static void drawFrame() {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(kBgColor);
  tft.drawRoundRect(3, 3, w - 6, h - 6, 5, kFgColor);
  tft.drawLine(5, kHeaderH, w - 6, kHeaderH, kFgColor);
  tft.drawLine(5, h - kFooterH - 2, w - 6, h - kFooterH - 2, kFgColor);
  tft.setTextColor(kFgColor, kBgColor);
  tft.setTextSize(1);
  tft.setCursor(8, 5);
  tft.print("IDK-SUB");
}

static void drawStatus() {
  int h = tft.height();
  tft.fillRect(6, h - kFooterH - 1, tft.width() - 12, kFooterH - 2, kBgColor);
  tft.drawRoundRect(6, h - kFooterH - 1, tft.width() - 12, kFooterH - 2, 3, kFgColor);
  tft.setTextColor(kAccentColor, kBgColor);
  tft.setTextSize(1);
  tft.setCursor(10, h - kFooterH + 1);
  if (g_connected) {
    tft.printf("WiFi: %s", WiFi.localIP().toString().c_str());
  } else {
    tft.print("WiFi: connecting...");
  }
}

static void drawWrappedText(const String &text) {
  int w = tft.width();
  int h = tft.height();
  int x = 8;
  int y = kHeaderH + 6;
  int area_w = w - 16;
  int area_h = h - kHeaderH - kFooterH - 12;
  tft.fillRect(x, y, area_w, area_h, kBgColor);
  tft.setTextColor(kFgColor, kBgColor);
  tft.setTextSize(1);

  int cursor_x = x;
  int cursor_y = y;
  String word = "";
  for (size_t i = 0; i <= text.length(); ++i) {
    char c = (i < text.length()) ? text[i] : ' ';
    bool split = (c == ' ' || c == '\n' || c == '\r' || i == text.length());
    if (!split) {
      word += c;
      continue;
    }
    if (word.length()) {
      String token = word + " ";
      int word_w = tft.textWidth(token);
      if (cursor_x + word_w > x + area_w) {
        cursor_x = x;
        cursor_y += 10;
      }
      if (cursor_y + 10 > y + area_h) break;
      tft.setCursor(cursor_x, cursor_y);
      tft.print(token);
      cursor_x += word_w;
      word = "";
    }
    if (c == '\n') {
      cursor_x = x;
      cursor_y += 10;
      if (cursor_y + 10 > y + area_h) break;
    }
  }
}

static void updateSubtitle(const String &text) {
  g_text = text;
  drawWrappedText(g_text);
}

static void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!g_connected) {
      g_connected = true;
      drawStatus();
    }
    return;
  }
  g_connected = false;
  const uint32_t now = millis();
  if (now - g_last_attempt_ms < 3000) return;
  g_last_attempt_ms = now;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(kSsid, kPass);
  drawStatus();
}

void setup() {
  enableDisplayPower();
  enableBacklight();
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(false);
  drawFrame();
  drawStatus();

  WiFi.mode(WIFI_STA);
  WiFi.begin(kSsid, kPass);
  udp.begin(kUdpPort);
}

void loop() {
  ensureWifi();

  if (WiFi.status() == WL_CONNECTED) {
    const uint32_t now = millis();
    if (now - g_last_hello_ms > 1000) {
      g_last_hello_ms = now;
      udp.beginPacket(IPAddress(192, 168, 4, 1), kUdpPort);
      udp.print("HELLO");
      udp.endPacket();
    }
  }

  int packet = udp.parsePacket();
  if (packet) {
    String text = "";
    while (udp.available()) {
      char c = static_cast<char>(udp.read());
      text += c;
      if (text.length() > 240) break;
    }
    updateSubtitle(text);
  }

  delay(10);
}

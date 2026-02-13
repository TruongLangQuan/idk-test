#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <XPT2046_Touchscreen.h>
#include <time.h>

#include "generated/cyd_img_assets.h"
#include "generated/cyd_gif_asset.h"

namespace {

constexpr int TFT_W = 320;
constexpr int TFT_H = 240;
constexpr int TOUCH_CS_PIN = 33;
constexpr int TOUCH_IRQ_PIN = 36;

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen touch(TOUCH_CS_PIN, TOUCH_IRQ_PIN);

enum class AppState {
  WIFI_SELECT,
  WIFI_CONNECTING,
  MAIN,
};

enum class Screen {
  CLOCK,
  GIF,
  IMG,
};

AppState g_state = AppState::WIFI_SELECT;
Screen g_screen = Screen::CLOCK;

String g_ssids[16];
int g_rssi[16]{};
int g_wifi_count = 0;
int g_wifi_index = 0;
String g_wifi_status = "Scanning...";
unsigned long g_connect_ms = 0;

unsigned long g_touch_debounce_ms = 0;
unsigned long g_last_clock_ms = 0;
unsigned long g_next_gif_ms = 0;
unsigned long g_next_img_ms = 0;
int g_gif_idx = 0;
int g_img_idx = 0;

bool touchRead(int& x, int& y) {
  if (!touch.touched()) return false;
  TS_Point p = touch.getPoint();

  // CYD 2.8" touch raw range (approx). Keep mapping simple/robust.
  int tx = map(p.x, 300, 3900, 0, TFT_W);
  int ty = map(p.y, 300, 3900, 0, TFT_H);
  tx = constrain(tx, 0, TFT_W - 1);
  ty = constrain(ty, 0, TFT_H - 1);

  x = tx;
  y = ty;
  return true;
}

void drawHeader(const char* title) {
  tft.fillRect(0, 0, TFT_W, 20, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setCursor(4, 6);
  tft.print(title);
}

void drawWifiUI() {
  tft.fillScreen(TFT_BLACK);
  drawHeader("idk-cyd WiFi");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(4, 24);
  tft.println("Touch left/right: scroll");
  tft.println("Touch center: connect");
  tft.println("Hold center: offline");
  tft.println(g_wifi_status);

  int start = g_wifi_index - 2;
  if (start < 0) start = 0;
  int end = start + 5;
  if (end > g_wifi_count) end = g_wifi_count;

  for (int i = start; i < end; ++i) {
    tft.setTextColor(i == g_wifi_index ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    tft.setCursor(6, 90 + (i - start) * 24);
    tft.printf("%c %s (%d)", i == g_wifi_index ? '>' : ' ', g_ssids[i].c_str(), g_rssi[i]);
  }
}

void scanWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(80);

  g_wifi_status = "Scanning...";
  drawWifiUI();

  g_wifi_count = WiFi.scanNetworks();
  if (g_wifi_count < 0) g_wifi_count = 0;
  if (g_wifi_count > 16) g_wifi_count = 16;
  for (int i = 0; i < g_wifi_count; ++i) {
    g_ssids[i] = WiFi.SSID(i);
    g_rssi[i] = WiFi.RSSI(i);
  }
  g_wifi_index = 0;
  g_wifi_status = (g_wifi_count > 0) ? "Select WiFi" : "No WiFi";
  drawWifiUI();
}

void drawClock() {
  tft.fillScreen(TFT_BLACK);
  drawHeader("Clock GMT+07");

  struct tm ti;
  bool ok = getLocalTime(&ti, 100);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(40, 90);
  if (ok) {
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", &ti);
    tft.print(buf);
    tft.setCursor(40, 130);
    strftime(buf, sizeof(buf), "%d-%m-%Y", &ti);
    tft.print(buf);
  } else {
    tft.print("No NTP time");
  }
  tft.setTextSize(1);
}

void drawGifFrame() {
  tft.fillScreen(TFT_BLACK);
  drawHeader("Built-in GIF");
  if (GIF_FRAME_COUNT == 0) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 40);
    tft.print("No GIF compiled");
    return;
  }

  int x = (TFT_W - GIF_WIDTH) / 2;
  int y = (TFT_H - GIF_HEIGHT) / 2;
  if (x < 0) x = 0;
  if (y < 20) y = 20;

  const uint16_t* px = reinterpret_cast<const uint16_t*>(GIF_FRAMES[g_gif_idx].rgb565);
  tft.pushImage(x, y, GIF_WIDTH, GIF_HEIGHT, px);
  g_next_gif_ms = millis() + (GIF_FRAMES[g_gif_idx].delay_ms ? GIF_FRAMES[g_gif_idx].delay_ms : 100);
}

void drawImage() {
  tft.fillScreen(TFT_BLACK);
  drawHeader("Built-in IMG");
  if (IMAGE_COUNT == 0) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 40);
    tft.print("No IMG compiled");
    return;
  }

  const auto& img = IMAGE_TABLE[g_img_idx];
  int x = (TFT_W - img.width) / 2;
  int y = (TFT_H - img.height) / 2;
  if (x < 0) x = 0;
  if (y < 20) y = 20;
  const uint16_t* px = reinterpret_cast<const uint16_t*>(img.rgb565);
  tft.pushImage(x, y, img.width, img.height, px);

  tft.fillRect(0, TFT_H - 18, TFT_W, 18, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(4, TFT_H - 14);
  tft.printf("%d/%d %s", g_img_idx + 1, static_cast<int>(IMAGE_COUNT), img.name);
}

void drawMainScreen() {
  if (g_screen == Screen::CLOCK) {
    drawClock();
  } else if (g_screen == Screen::GIF) {
    drawGifFrame();
  } else {
    drawImage();
  }
}

void nextScreen() {
  g_screen = (g_screen == Screen::CLOCK) ? Screen::GIF : (g_screen == Screen::GIF ? Screen::IMG : Screen::CLOCK);
  drawMainScreen();
}

void prevScreen() {
  g_screen = (g_screen == Screen::CLOCK) ? Screen::IMG : (g_screen == Screen::GIF ? Screen::CLOCK : Screen::GIF);
  drawMainScreen();
}

void handleWifiTouch(int x, int y) {
  (void)y;
  if (g_wifi_count <= 0) {
    g_state = AppState::MAIN;
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    drawMainScreen();
    return;
  }

  if (x < 100) {
    g_wifi_index = (g_wifi_index + g_wifi_count - 1) % g_wifi_count;
    drawWifiUI();
    return;
  }

  if (x > 220) {
    g_wifi_index = (g_wifi_index + 1) % g_wifi_count;
    drawWifiUI();
    return;
  }

  g_wifi_status = "Connecting: " + g_ssids[g_wifi_index];
  drawWifiUI();
  WiFi.begin(g_ssids[g_wifi_index].c_str());
  g_connect_ms = millis();
  g_state = AppState::WIFI_CONNECTING;
}

void handleMainTouch(int x, int y) {
  if (x < 80) {
    prevScreen();
    return;
  }
  if (x > 240) {
    nextScreen();
    return;
  }

  if (g_screen == Screen::IMG && IMAGE_COUNT > 0) {
    if (y > TFT_H / 2) g_img_idx = (g_img_idx + 1) % IMAGE_COUNT;
    else g_img_idx = (g_img_idx + IMAGE_COUNT - 1) % IMAGE_COUNT;
    drawImage();
  }
}

void setupTouch() {
  touch.begin();
  touch.setRotation(1);
}

}  // namespace

void setup() {
  SPI.begin();
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  setupTouch();
  scanWifi();
}

void loop() {
  int tx = 0, ty = 0;
  bool touched = touchRead(tx, ty);

  if (touched && millis() > g_touch_debounce_ms) {
    g_touch_debounce_ms = millis() + 180;
    if (g_state == AppState::WIFI_SELECT) {
      handleWifiTouch(tx, ty);
    } else if (g_state == AppState::MAIN) {
      handleMainTouch(tx, ty);
    }
  }

  if (g_state == AppState::WIFI_CONNECTING) {
    wl_status_t st = WiFi.status();
    if (st == WL_CONNECTED) {
      g_state = AppState::MAIN;
      configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
      drawMainScreen();
    } else if (millis() - g_connect_ms > 10000) {
      WiFi.disconnect(true, true);
      g_state = AppState::WIFI_SELECT;
      g_wifi_status = "Connect fail";
      drawWifiUI();
    }
  }

  if (g_state == AppState::MAIN) {
    if (g_screen == Screen::CLOCK) {
      if (millis() - g_last_clock_ms > 1000) {
        g_last_clock_ms = millis();
        drawClock();
      }
    } else if (g_screen == Screen::GIF) {
      if (GIF_FRAME_COUNT > 0 && static_cast<int32_t>(millis() - g_next_gif_ms) >= 0) {
        g_gif_idx = (g_gif_idx + 1) % GIF_FRAME_COUNT;
        drawGifFrame();
      }
    } else {
      if (IMAGE_COUNT > 0 && millis() - g_next_img_ms > 3000) {
        g_next_img_ms = millis();
        g_img_idx = (g_img_idx + 1) % IMAGE_COUNT;
        drawImage();
      }
    }
  }

  delay(10);
}

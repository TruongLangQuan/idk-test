#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>

#include "../../shared/idk_vn_text.h"

#ifndef TFT_I2C_POWER
#define TFT_I2C_POWER 21
#endif

namespace {

static const char* kSsid = "Diddy Heil Epstein";
static const char* kPass = "TruongLangQuan";
static const IPAddress kHostIp(192, 168, 4, 1);
static const uint16_t kSubtitlePort = 4210;
static const uint16_t kMazePort = 4211;
static const uint16_t kDicePort = 4212;
static const uint16_t kLocalPort = 4210;
static const uint32_t kHelloIntervalMs = 1000;
static const uint32_t kLongPressMs = 650;
static const uint32_t kModeHoldMs = 1600;
static const uint16_t kFgColor = 0x07E0;
static const uint16_t kAccentColor = 0xF800;
static const uint16_t kBgColor = 0x0000;
static const int kHeaderH = 20;
static const int kFooterH = 32;

enum class Mode {
  Subtitle,
  Maze,
  Dice,
};

static TFT_eSPI tft = TFT_eSPI();
static WiFiUDP udp;
static Mode g_mode = Mode::Subtitle;
static String g_text;
static String g_status = "Ready";
static bool g_wifi_connected = false;
static bool g_button_down = false;
static uint32_t g_press_start_ms = 0;
static uint32_t g_last_wifi_attempt_ms = 0;
static uint32_t g_last_hello_ms = 0;
static int g_maze_index = 0;

static const char* kMazeCommands[] = {
    "RIGHT",
    "LEFT",
    "UP",
    "DOWN",
    "STAIR_UP",
    "STAIR_DOWN",
    "NEW",
    "TOGGLE_3D",
};

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

static const char* modeName() {
  switch (g_mode) {
    case Mode::Subtitle:
      return "SUBTITLE";
    case Mode::Maze:
      return "MAZE";
    case Mode::Dice:
      return "DICE";
  }
  return "SUBTITLE";
}

static const char* currentCommand() {
  if (g_mode == Mode::Maze) return kMazeCommands[g_maze_index];
  if (g_mode == Mode::Dice) return "ROLL";
  return "HEARTBEAT";
}

static uint16_t currentPort() {
  if (g_mode == Mode::Maze) return kMazePort;
  if (g_mode == Mode::Dice) return kDicePort;
  return kSubtitlePort;
}

static void drawSubtitleArea() {
  int y = kHeaderH + 6;
  int h = tft.height() - kHeaderH - kFooterH - 10;
  tft.fillRect(6, y, tft.width() - 12, h, kBgColor);
  tft.drawRoundRect(4, y - 2, tft.width() - 8, h + 4, 4, kFgColor);
  idk_vn_text::drawWrapped(tft, g_text, 8, y + 4, tft.width() - 16, h - 8, kFgColor, kBgColor, 1);
}

static void drawFooter() {
  int h = tft.height();
  int y = h - kFooterH;
  tft.fillRect(0, y, tft.width(), kFooterH, kBgColor);
  tft.drawLine(5, y, tft.width() - 6, y, kFgColor);
  tft.setTextColor(kAccentColor, kBgColor);
  tft.setTextSize(1);
  tft.setCursor(8, y + 4);
  tft.printf("MODE: %s", modeName());
  tft.setTextColor(kFgColor, kBgColor);
  tft.setCursor(8, y + 14);
  tft.printf("CMD: %s", currentCommand());
  tft.setCursor(8, y + 24);
  tft.print(g_wifi_connected ? "WiFi OK" : "WiFi...");
  tft.print(" | ");
  tft.print(g_status);
}

static void drawFrame() {
  tft.fillScreen(kBgColor);
  tft.drawRoundRect(3, 3, tft.width() - 6, tft.height() - 6, 5, kFgColor);
  tft.drawLine(5, kHeaderH, tft.width() - 6, kHeaderH, kFgColor);
  tft.setTextColor(kFgColor, kBgColor);
  tft.setTextSize(1);
  tft.setCursor(8, 6);
  tft.print("IDK-SUB+REMOTE");
  drawSubtitleArea();
  drawFooter();
}

static void setStatus(const String& status) {
  g_status = status;
  drawFooter();
}

static void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!g_wifi_connected) {
      g_wifi_connected = true;
      setStatus("Connected");
    }
    return;
  }

  g_wifi_connected = false;
  const uint32_t now = millis();
  if (now - g_last_wifi_attempt_ms < 3000) return;
  g_last_wifi_attempt_ms = now;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(kSsid, kPass);
  setStatus("Connecting...");
}

static void sendPacket(uint16_t port, const char* payload) {
  if (WiFi.status() != WL_CONNECTED) {
    setStatus("No WiFi");
    return;
  }
  udp.beginPacket(kHostIp, port);
  udp.print(payload);
  udp.endPacket();
}

static void sendHeartbeat() {
  if (WiFi.status() != WL_CONNECTED) return;
  uint32_t now = millis();
  if (now - g_last_hello_ms < kHelloIntervalMs) return;
  g_last_hello_ms = now;
  sendPacket(kSubtitlePort, "HELLO");
}

static void sendActiveCommand() {
  if (g_mode == Mode::Subtitle) {
    setStatus("Subtitle only");
    return;
  }
  sendPacket(currentPort(), currentCommand());
  setStatus(String("Sent: ") + currentCommand());
}

static void nextShortAction() {
  if (g_mode == Mode::Maze) {
    g_maze_index = (g_maze_index + 1) % (sizeof(kMazeCommands) / sizeof(kMazeCommands[0]));
    setStatus("Maze cmd");
  } else if (g_mode == Mode::Dice) {
    sendPacket(kDicePort, "ROLL");
    setStatus("Dice roll");
  } else {
    setStatus("Subtitle live");
  }
}

static void toggleMode() {
  if (g_mode == Mode::Subtitle) g_mode = Mode::Maze;
  else if (g_mode == Mode::Maze) g_mode = Mode::Dice;
  else g_mode = Mode::Subtitle;
  setStatus("Mode changed");
}

static void handleButton() {
  bool down = digitalRead(0) == LOW;
  uint32_t now = millis();
  if (down && !g_button_down) {
    g_button_down = true;
    g_press_start_ms = now;
  }
  if (!down && g_button_down) {
    g_button_down = false;
    uint32_t held = now - g_press_start_ms;
    if (held >= kModeHoldMs) toggleMode();
    else if (held >= kLongPressMs) sendActiveCommand();
    else nextShortAction();
  }
}

static void updateSubtitle(const String& text) {
  g_text = text;
  drawSubtitleArea();
}

static void pollUdp() {
  int packet = udp.parsePacket();
  if (packet <= 0) return;
  String text = "";
  while (udp.available()) {
    text += static_cast<char>(udp.read());
    if (text.length() > 320) break;
  }
  text.replace("\r", "");
  if (text == "HELLO") return;
  updateSubtitle(text);
}

}  // namespace

void setup() {
  pinMode(0, INPUT_PULLUP);
  enableDisplayPower();
  enableBacklight();
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(false);
  drawFrame();
  WiFi.mode(WIFI_STA);
  WiFi.begin(kSsid, kPass);
  udp.begin(kLocalPort);
}

void loop() {
  ensureWifi();
  sendHeartbeat();
  pollUdp();
  handleButton();
  delay(10);
}

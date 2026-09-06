#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <cmath>

namespace {

static const uint16_t kBg = TFT_BLACK;
static const uint16_t kFg = TFT_GREEN;
static const uint16_t kAccent = TFT_RED;
static const char* kApSsid = "Diddy Heil Epstein";
static const char* kApPass = "TruongLangQuan";
static const uint16_t kUdpPort = 4212;

uint32_t g_last_roll_ms = 0;
int g_d1 = 1;
int g_d2 = 1;
int g_d3 = 1;
int g_total = 3;
String g_result = "Ready";
WiFiUDP g_udp;

void drawFrame() {
  M5.Display.fillScreen(kBg);
  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(4, 4);
  M5.Display.print("idk-dice");
  M5.Display.setTextColor(TFT_DARKGREY, kBg);
  M5.Display.setCursor(4, 20);
  M5.Display.print("Shake/UDP to roll");
}

void drawDie(int x, int y, int v) {
  M5.Display.drawRoundRect(x, y, 60, 60, 6, kFg);
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(x + 24, y + 20);
  M5.Display.print(v);
  M5.Display.setTextSize(1);
}

void drawDice() {
  drawFrame();
  drawDie(10, 40, g_d1);
  drawDie(88, 40, g_d2);
  drawDie(166, 40, g_d3);
  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(10, 108);
  M5.Display.printf("Total: %d", g_total);
  M5.Display.setTextColor(g_result == "WIN" ? TFT_GREEN : (g_result == "LOSE" ? TFT_RED : TFT_YELLOW), kBg);
  M5.Display.setCursor(110, 108);
  M5.Display.print(g_result);
  M5.Display.setTextColor(TFT_DARKGREY, kBg);
  M5.Display.setCursor(4, 124);
  M5.Display.print("A:Roll");
}

void rollDice() {
  g_d1 = 1 + (esp_random() % 6);
  g_d2 = 1 + (esp_random() % 6);
  g_d3 = 1 + (esp_random() % 6);
  g_total = g_d1 + g_d2 + g_d3;
  if (g_total >= 4 && g_total <= 10) g_result = "LOSE";
  else if (g_total >= 11 && g_total <= 17) g_result = "WIN";
  else g_result = "JACKPOT";
  drawDice();
}

void pollRemoteRoll() {
  int packet = g_udp.parsePacket();
  if (packet <= 0) return;
  String cmd = "";
  while (g_udp.available()) {
    cmd += static_cast<char>(g_udp.read());
    if (cmd.length() > 32) break;
  }
  cmd.trim();
  if (cmd == "ROLL") {
    rollDice();
    g_last_roll_ms = millis();
  }
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
#if defined(STICKS3)
  M5.Display.setRotation(1);
#else
  M5.Display.setRotation(3);
#endif
  M5.Display.setBrightness(180);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApSsid, kApPass);
  g_udp.begin(kUdpPort);
  drawDice();
}

void loop() {
  M5.update();
  pollRemoteRoll();

  if (M5.BtnA.wasPressed()) {
    rollDice();
    g_last_roll_ms = millis();
  }

  float ax = 0, ay = 0, az = 0;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    float mag = sqrtf(ax * ax + ay * ay + az * az);
    float delta = fabsf(mag - 1.0f);
    uint32_t now = millis();
    if (delta > 1.4f && (now - g_last_roll_ms) > 800) {
      rollDice();
      g_last_roll_ms = now;
    }
  }

  delay(10);
}

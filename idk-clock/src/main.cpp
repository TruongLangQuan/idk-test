#include <M5Unified.h>
#include <WiFi.h>
#include <time.h>

#include <array>

namespace {

enum class AppState {
  WIFI_SELECT,
  WIFI_CONNECTING,
  CLOCK,
};

struct KnownWifi {
  const char* ssid;
  const char* pass;
};

constexpr KnownWifi kKnownWifis[] = {
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

AppState g_state = AppState::WIFI_SELECT;
std::array<String, 16> g_ssids{};
std::array<int, 16> g_rssi{};
std::array<bool, 16> g_secured{};
int g_wifi_count = 0;
int g_wifi_index = 0;
String g_wifi_status = "Scanning...";
uint32_t g_connect_start = 0;
uint32_t g_next_clock = 0;

void drawWifi() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(2, 2);
  M5.Display.println("idk-clock WiFi");
  M5.Display.println("Next/Prev:scroll M5:connect");
  M5.Display.println("M5+Prev:offline");
  M5.Display.println(g_wifi_status);

  if (g_wifi_count <= 0) {
    M5.Display.println("No AP found");
    return;
  }

  int start = std::max(0, g_wifi_index - 2);
  int end = std::min(g_wifi_count, start + 4);
  for (int i = start; i < end; ++i) {
    M5.Display.setTextColor(i == g_wifi_index ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.printf("%c %s (%d)\n", i == g_wifi_index ? '>' : ' ', g_ssids[i].c_str(), g_rssi[i]);
  }
}

void syncTimeVN() {
  configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com", "time.windows.com");
}

void startClock() {
  syncTimeVN();
  g_state = AppState::CLOCK;
  g_next_clock = 0;
}

bool tryKnownWifiAutoConnect() {
  WiFi.mode(WIFI_STA);
  for (const auto& known : kKnownWifis) {
    int found = -1;
    for (int i = 0; i < g_wifi_count; ++i) {
      if (g_ssids[i] == known.ssid) {
        found = i;
        break;
      }
    }
    if (found < 0) continue;

    g_wifi_index = found;
    g_wifi_status = String("Auto connect: ") + known.ssid;
    drawWifi();
    WiFi.disconnect(true, true);
    delay(100);
    WiFi.begin(known.ssid, known.pass);
    uint32_t start = millis();
    while (millis() - start < 9000) {
      if (WiFi.status() == WL_CONNECTED) {
        g_wifi_status = String("Connected: ") + known.ssid;
        drawWifi();
        return true;
      }
      delay(120);
    }
  }
  WiFi.disconnect(true, true);
  return false;
}

void scanWifi() {
  g_wifi_status = "Scanning...";
  drawWifi();
  g_wifi_count = WiFi.scanNetworks();
  if (g_wifi_count < 0) g_wifi_count = 0;
  if (g_wifi_count > static_cast<int>(g_ssids.size())) g_wifi_count = g_ssids.size();
  for (int i = 0; i < g_wifi_count; ++i) {
    g_ssids[i] = WiFi.SSID(i);
    g_rssi[i] = WiFi.RSSI(i);
    g_secured[i] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  }
  g_wifi_index = 0;

  if (g_wifi_count > 0 && tryKnownWifiAutoConnect()) {
    startClock();
    return;
  }

  g_wifi_status = g_wifi_count > 0 ? "Select WiFi" : "No WiFi";
  drawWifi();
}

bool inputPassword(String& out) {
  String value = "";
  const char* top[4] = {"OK", "DEL", "SP", "ESC"};
  const char* row1 = "1234567890";
  const char* row2 = "qwertyuiop";
  const char* row3 = "asdfghjkl";
  const char* row4 = "zxcvbnm-_.@";

  int row = 0;
  int col = 0;

  auto rowLen = [&](int r) {
    if (r == 0) return 4;
    if (r == 1) return (int)strlen(row1);
    if (r == 2) return (int)strlen(row2);
    if (r == 3) return (int)strlen(row3);
    return (int)strlen(row4);
  };

  auto draw = [&]() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, 2);
    M5.Display.println("Password");
    M5.Display.setCursor(2, 14);
    M5.Display.print(value);

    int y = 42;
    for (int i = 0; i < 4; ++i) {
      M5.Display.setTextColor((row == 0 && col == i) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(4 + i * 58, y);
      M5.Display.print(top[i]);
    }

    auto drawRow = [&](int rr, const char* text, int yy) {
      int n = strlen(text);
      for (int i = 0; i < n && i < 28; ++i) {
        M5.Display.setTextColor((row == rr && col == i) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(4 + (i % 14) * 16, yy + (i / 14) * 10);
        M5.Display.print(text[i]);
      }
    };
    drawRow(1, row1, 56);
    drawRow(2, row2, 68);
    drawRow(3, row3, 88);
    drawRow(4, row4, 108);
  };

  draw();
  while (true) {
    M5.update();
    if (M5.BtnPWR.wasPressed()) {
      row = (row + 1) % 5;
      int mx = rowLen(row);
      if (col >= mx) col = mx - 1;
      if (col < 0) col = 0;
      draw();
    }
    if (M5.BtnB.wasPressed()) {
      int mx = rowLen(row);
      col = (col + 1) % mx;
      draw();
    }
    if (M5.BtnA.wasPressed()) {
      if (row == 0) {
        if (col == 0) { out = value; return true; }
        if (col == 1) { if (!value.isEmpty()) value.remove(value.length() - 1); }
        if (col == 2) { if (value.length() < 63) value += ' '; }
        if (col == 3) { return false; }
      } else {
        char ch = 0;
        if (row == 1) ch = row1[col];
        if (row == 2) ch = row2[col];
        if (row == 3) ch = row3[col];
        if (row == 4) ch = row4[col];
        if (ch && value.length() < 63) value += ch;
      }
      draw();
    }
    delay(8);
  }
}

void handleWifiInput() {
  if (M5.BtnB.wasPressed() && g_wifi_count > 0) {
    g_wifi_index = (g_wifi_index + 1) % g_wifi_count;
    drawWifi();
  }
  if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed() && g_wifi_count > 0) {
    g_wifi_index = (g_wifi_index + g_wifi_count - 1) % g_wifi_count;
    drawWifi();
  }

  bool selectPressed = M5.BtnA.wasPressed();
  bool skipPressed = M5.BtnA.isPressed() && M5.BtnPWR.wasPressed();
  if (skipPressed) {
    g_wifi_status = "Offline mode";
    startClock();
    return;
  }

  if (!selectPressed) return;
  if (g_wifi_count == 0) {
    startClock();
    return;
  }

  String pass = "";
  if (g_secured[g_wifi_index]) {
    bool ok = inputPassword(pass);
    if (!ok) {
      g_wifi_status = "Cancelled";
      drawWifi();
      return;
    }
  }

  WiFi.mode(WIFI_STA);
  if (g_secured[g_wifi_index]) WiFi.begin(g_ssids[g_wifi_index].c_str(), pass.c_str());
  else WiFi.begin(g_ssids[g_wifi_index].c_str());
  g_connect_start = millis();
  g_wifi_status = "Connecting: " + g_ssids[g_wifi_index];
  g_state = AppState::WIFI_CONNECTING;
  drawWifi();
}

void handleConnectingState() {
  if (WiFi.status() == WL_CONNECTED) {
    g_wifi_status = "Connected";
    delay(250);
    startClock();
    return;
  }
  if (millis() - g_connect_start > 10000) {
    WiFi.disconnect(true, true);
    g_wifi_status = "Connect fail";
    g_state = AppState::WIFI_SELECT;
    drawWifi();
  }
}

void drawClockUI() {
  M5.Display.fillScreen(TFT_BLACK);

  // Minimal stars
  M5.Display.drawPixel(40, 30, M5.Display.color565(180, 180, 180));
  M5.Display.drawPixel(41, 30, M5.Display.color565(180, 180, 180));
  M5.Display.drawPixel(200, 90, M5.Display.color565(170, 170, 170));
  M5.Display.drawPixel(120, 60, M5.Display.color565(175, 175, 175));

  // Static accretion ring
  M5.Display.drawEllipse(120, 67, 80, 18, M5.Display.color565(150, 150, 150));
  M5.Display.drawEllipse(120, 67, 79, 17, M5.Display.color565(70, 70, 70));

  // Black-hole core with monochrome gradient layers
  const uint16_t shades[] = {
      M5.Display.color565(0, 0, 0),
      M5.Display.color565(5, 5, 5),
      M5.Display.color565(10, 10, 10),
      M5.Display.color565(17, 17, 17),
      M5.Display.color565(8, 8, 8),
      M5.Display.color565(0, 0, 0),
  };
  const int radii[] = {55, 48, 40, 31, 22, 12};
  for (int i = 0; i < 6; ++i) {
    M5.Display.fillCircle(120, 67, radii[i], shades[i]);
  }
  M5.Display.drawCircle(120, 67, 55, M5.Display.color565(50, 50, 50));

  struct tm tm_now;
  bool hasTime = getLocalTime(&tm_now, 20);

  char time_buf[8] = "--:--";
  char date_buf[6] = "--/--";
  if (hasTime) {
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d", tm_now.tm_hour, tm_now.tm_min);
    snprintf(date_buf, sizeof(date_buf), "%02d/%02d", tm_now.tm_mday, tm_now.tm_mon + 1);
  }

  // Larger time
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextFont(4);
  int tw = M5.Display.textWidth(time_buf);
  M5.Display.setCursor(120 - tw / 2, 48);
  M5.Display.print(time_buf);

  // Larger date
  M5.Display.setTextColor(M5.Display.color565(200, 200, 200), TFT_BLACK);
  M5.Display.setTextFont(2);
  int dw = M5.Display.textWidth(date_buf);
  M5.Display.setCursor(120 - dw / 2, 86);
  M5.Display.print(date_buf);
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  scanWifi();
}

void loop() {
  M5.update();

  if (g_state == AppState::WIFI_SELECT) {
    handleWifiInput();
    delay(10);
    return;
  }

  if (g_state == AppState::WIFI_CONNECTING) {
    handleConnectingState();
    delay(10);
    return;
  }

  if ((int32_t)(millis() - g_next_clock) >= 0) {
    drawClockUI();
    g_next_clock = millis() + 1000;
  }
  delay(10);
}

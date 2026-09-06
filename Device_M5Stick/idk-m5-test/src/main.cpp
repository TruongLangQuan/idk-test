#include <M5Unified.h>

namespace {

static constexpr int kPeerTx = 33;  // M5StickC Plus2 -> ESP32-S3 RX48
static constexpr int kPeerRx = 32;  // M5StickC Plus2 <- ESP32-S3 TX47
static constexpr uint32_t kPeerBaud = 115200;

static constexpr int kJoyX = 36;
static constexpr int kJoyY = 26;
static constexpr int kJoyBtn = 0;

static constexpr size_t kVisibleRows = 6;
static constexpr uint16_t kBg = TFT_BLACK;
static constexpr uint16_t kFg = TFT_WHITE;
static constexpr uint16_t kAccent = TFT_CYAN;
static constexpr uint16_t kOk = TFT_GREEN;
static constexpr uint16_t kWarn = TFT_YELLOW;

HardwareSerial peerSerial(1);

enum class AppMode {
  REMOTE,
  JOYSTICK_TEST
};

AppMode g_appMode = AppMode::REMOTE;
bool g_peerOnline = false;
bool g_inFrame = false;
bool g_uiReady = false;
String g_title = "idk-m5-test remote";
String g_sub = "Waiting for ESP32-S3";
String g_status = "Connect UART TX33/RX32";
String g_footer = "A:OPEN B:DOWN PWR:UP";
String g_lines[kVisibleRows];
String g_peerMode = "NONE";
uint32_t g_lastPeerSeenMs = 0;
uint32_t g_lastSyncMs = 0;
char g_rxBuffer[128];
size_t g_rxLen = 0;

void drawUi() {
  if (g_appMode == AppMode::JOYSTICK_TEST) return;
  M5.Display.fillScreen(kBg);
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setCursor(4, 4);
  M5.Display.print(g_title);

  M5.Display.setTextColor(g_peerOnline ? kOk : kWarn, kBg);
  M5.Display.setCursor(4, 18);
  M5.Display.print(g_sub);

  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(4, 32);
  M5.Display.print(g_status);

  for (size_t i = 0; i < kVisibleRows; ++i) {
    M5.Display.setCursor(4, 48 + static_cast<int>(i) * 12);
    M5.Display.print(g_lines[i]);
  }

  M5.Display.fillRect(0, 118, 240, 17, kBg);
  M5.Display.drawFastHLine(0, 118, 240, kAccent);
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setCursor(4, 122);
  M5.Display.print(g_footer);
}

void drawJoystickTest() {
  int x = analogRead(kJoyX);
  int y = analogRead(kJoyY);
  bool sw = digitalRead(kJoyBtn) == LOW;

  M5.Display.fillScreen(kBg);
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setCursor(4, 4);
  M5.Display.print("Joystick Test");

  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(4, 25);
  M5.Display.printf("X (G36): %4d", x);
  M5.Display.setCursor(4, 40);
  M5.Display.printf("Y (G26): %4d", y);
  M5.Display.setCursor(4, 55);
  M5.Display.printf("SW (G0): %s", sw ? "PRESSED" : "RELEASED");

  // Visual representation
  int centerX = 160;
  int centerY = 70;
  int offX = map(x, 0, 4095, 30, -30); // Reversed usually for stick
  int offY = map(y, 0, 4095, 30, -30);
  
  M5.Display.drawCircle(centerX, centerY, 35, kAccent);
  M5.Display.drawFastHLine(centerX - 35, centerY, 70, kAccent);
  M5.Display.drawFastVLine(centerX, centerY - 35, 70, kAccent);
  M5.Display.fillCircle(centerX + offX, centerY + offY, 8, sw ? kOk : kFg);
  
  M5.Display.drawFastHLine(0, 118, 240, kAccent);
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setCursor(4, 122);
  M5.Display.print("Hold B: Exit");
}

void resetUiToWaiting() {
  g_uiReady = false;
  g_title = "idk-m5-test remote";
  g_sub = "Waiting for ESP32-S3";
  g_status = "Check TX33/RX32/GND";
  g_footer = "A:SYNC B:DOWN PWR:UP";
  for (auto& line : g_lines) line = "";
}

void sendCommand(const char* cmd) {
  peerSerial.print("CMD ");
  peerSerial.println(cmd);
}

void applyFrameLine(const String& line, size_t& lineIndex) {
  if (g_appMode != AppMode::REMOTE) return;
  if (line == "UI_BEGIN") {
    g_inFrame = true;
    g_uiReady = false;
    for (auto& item : g_lines) item = "";
    g_footer = "";
    return;
  }

  if (line == "UI_END") {
    g_inFrame = false;
    g_uiReady = true;
    g_peerOnline = true;
    g_lastPeerSeenMs = millis();
    drawUi();
    return;
  }

  if (!g_inFrame) {
    if (line.startsWith("STAT ")) {
      g_peerOnline = true;
      g_lastPeerSeenMs = millis();
    }
    return;
  }

  if (line.startsWith("MODE:")) {
    g_peerMode = line.substring(5);
  } else if (line.startsWith("TITLE:")) {
    g_title = line.substring(6);
  } else if (line.startsWith("SUB:")) {
    g_sub = line.substring(4);
  } else if (line.startsWith("STATUS:")) {
    g_status = line.substring(7);
  } else if (line.startsWith("LINE:")) {
    if (lineIndex < kVisibleRows) g_lines[lineIndex++] = line.substring(5);
  } else if (line.startsWith("FOOTER:")) {
    g_footer = line.substring(7);
  }
}

void pollPeer() {
  size_t lineIndex = 0;
  while (peerSerial.available()) {
    const int ch = peerSerial.read();
    if (ch < 0) break;
    if (ch == '\r') continue;
    if (ch == '\n') {
      g_rxBuffer[g_rxLen] = '\0';
      applyFrameLine(String(g_rxBuffer), lineIndex);
      g_rxLen = 0;
      continue;
    }
    if (g_rxLen + 1 < sizeof(g_rxBuffer)) {
      g_rxBuffer[g_rxLen++] = static_cast<char>(ch);
    } else {
      g_rxLen = 0;
    }
  }

  const uint32_t now = millis();
  if (!g_peerOnline || now - g_lastSyncMs >= 1500) {
    g_lastSyncMs = now;
    sendCommand("SYNC");
  }

  if (g_peerOnline && now - g_lastPeerSeenMs > 5000) {
    g_peerOnline = false;
    if (g_appMode == AppMode::REMOTE) {
      resetUiToWaiting();
      drawUi();
    }
  }
}

void handleButtons() {
  if (M5.BtnB.pressedFor(1000)) {
    if (g_appMode == AppMode::REMOTE) {
      g_appMode = AppMode::JOYSTICK_TEST;
      M5.Display.fillScreen(kBg);
    } else {
      g_appMode = AppMode::REMOTE;
      drawUi();
    }
    delay(500); // Debounce mode switch
    return;
  }

  if (g_appMode == AppMode::REMOTE) {
    if (M5.BtnA.wasPressed()) {
      sendCommand("OPEN");
    }
    if (M5.BtnB.wasPressed()) {
      sendCommand("DOWN");
    }
    if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
      sendCommand("UP");
    }

    if (M5.BtnA.pressedFor(700)) {
      sendCommand("REMOUNT");
      delay(120);
    }
    if (M5.BtnPWR.pressedFor(700) && !M5.BtnA.isPressed()) {
      sendCommand("BACK");
      delay(120);
    }
  }
}

void printInfo() {
  Serial.println();
  Serial.println("idk-m5-test remote client");
  Serial.println("M5StickC Plus2 receives UI over UART from ESP32-S3.");
  Serial.printf("UART TX=%d RX=%d baud=%lu\n", kPeerTx, kPeerRx, static_cast<unsigned long>(kPeerBaud));
  Serial.println("Buttons: BtnA=open, BtnB=down/next, BtnPWR=up/prev, hold PWR=back, hold A=remount.");
  Serial.println("Joystick: Hold B to toggle Joystick Test Mode.");
  Serial.println("Joystick Mapping:");
  Serial.println("  VRx -> G36");
  Serial.println("  VRy -> G26");
  Serial.println("  SW  -> G0");
  Serial.println();
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
  M5.Display.setTextSize(1);
  
  pinMode(kJoyX, INPUT);
  pinMode(kJoyY, INPUT);
  pinMode(kJoyBtn, INPUT_PULLUP);

  Serial.begin(115200);
  delay(150);

  peerSerial.begin(kPeerBaud, SERIAL_8N1, kPeerRx, kPeerTx);
  resetUiToWaiting();
  drawUi();
  sendCommand("SYNC");
  printInfo();
}

void loop() {
  M5.update();
  pollPeer();
  handleButtons();
  
  if (g_appMode == AppMode::JOYSTICK_TEST) {
    static uint32_t lastDraw = 0;
    if (millis() - lastDraw > 50) {
      drawJoystickTest();
      lastDraw = millis();
    }
  }
  
  delay(10);
}


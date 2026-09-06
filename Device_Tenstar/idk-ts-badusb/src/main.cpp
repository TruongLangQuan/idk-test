#include <Arduino.h>
#include <TFT_eSPI.h>
#include "payload.h"

namespace {

static constexpr int kCh9329Tx = 1;       // TS TX -> CH9329 RXD
static constexpr int kCh9329Rx = 2;       // TS RX -> CH9329 TXD
static constexpr int kArmButton = 0;      // BOOT button
static constexpr uint32_t kCh9329Baud = 9600;

TFT_eSPI tft = TFT_eSPI();
HardwareSerial ch9329(1);

bool g_running = false;
bool g_done = false;
uint32_t g_pressStart = 0;
int g_lineNo = 0;

void drawStatus(const char* title, const String& msg, uint16_t color = TFT_WHITE) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(4, 4);
  tft.print("idk-ts-badusb");
  tft.setTextColor(color, TFT_BLACK);
  tft.setCursor(4, 28);
  tft.print(title);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(4, 48);
  tft.print(msg);
}

uint8_t checksum(const uint8_t* buf, size_t len) {
  uint16_t sum = 0;
  for (size_t i = 0; i < len; ++i) sum += buf[i];
  return static_cast<uint8_t>(sum & 0xFF);
}

void sendKeyboardReport(uint8_t modifier, const uint8_t keys[6]) {
  uint8_t frame[16] = {
      0x57, 0xAB, 0x00, 0x02, 0x08,
      modifier, 0x00,
      keys[0], keys[1], keys[2], keys[3], keys[4], keys[5],
      0x00,
  };
  frame[13] = checksum(frame, 13);
  ch9329.write(frame, 14);
  ch9329.flush();
  delay(8);
}

void releaseKeys() {
  const uint8_t keys[6] = {0, 0, 0, 0, 0, 0};
  sendKeyboardReport(0, keys);
  delay(12);
}

bool asciiToHid(char c, uint8_t& modifier, uint8_t& key) {
  modifier = 0;
  key = 0;
  if (c >= 'a' && c <= 'z') { key = 0x04 + (c - 'a'); return true; }
  if (c >= 'A' && c <= 'Z') { modifier = 0x02; key = 0x04 + (c - 'A'); return true; }
  if (c >= '1' && c <= '9') { key = 0x1E + (c - '1'); return true; }
  if (c == '0') { key = 0x27; return true; }
  switch (c) {
    case ' ': key = 0x2C; return true;
    case '\n': key = 0x28; return true;
    case '-': key = 0x2D; return true;
    case '_': modifier = 0x02; key = 0x2D; return true;
    case '=': key = 0x2E; return true;
    case '+': modifier = 0x02; key = 0x2E; return true;
    case '[': key = 0x2F; return true;
    case ']': key = 0x30; return true;
    case '\\': key = 0x31; return true;
    case ';': key = 0x33; return true;
    case ':': modifier = 0x02; key = 0x33; return true;
    case '\'': key = 0x34; return true;
    case '"': modifier = 0x02; key = 0x34; return true;
    case '`': key = 0x35; return true;
    case '~': modifier = 0x02; key = 0x35; return true;
    case ',': key = 0x36; return true;
    case '<': modifier = 0x02; key = 0x36; return true;
    case '.': key = 0x37; return true;
    case '>': modifier = 0x02; key = 0x37; return true;
    case '/': key = 0x38; return true;
    case '?': modifier = 0x02; key = 0x38; return true;
    case '!': modifier = 0x02; key = 0x1E; return true;
    case '@': modifier = 0x02; key = 0x1F; return true;
    case '#': modifier = 0x02; key = 0x20; return true;
    case '$': modifier = 0x02; key = 0x21; return true;
    case '%': modifier = 0x02; key = 0x22; return true;
    case '^': modifier = 0x02; key = 0x23; return true;
    case '&': modifier = 0x02; key = 0x24; return true;
    case '*': modifier = 0x02; key = 0x25; return true;
    case '(': modifier = 0x02; key = 0x26; return true;
    case ')': modifier = 0x02; key = 0x27; return true;
  }
  return false;
}

void tapKey(uint8_t key, uint8_t modifier = 0) {
  uint8_t keys[6] = {key, 0, 0, 0, 0, 0};
  sendKeyboardReport(modifier, keys);
  releaseKeys();
}

void typeText(const String& text) {
  for (int i = 0; i < text.length(); ++i) {
    uint8_t mod = 0;
    uint8_t key = 0;
    if (asciiToHid(text[i], mod, key)) tapKey(key, mod);
    delay(4);
  }
}

void tapAsciiChord(const String& keyText, uint8_t extraModifier) {
  String s = keyText;
  s.trim();
  if (s.length() == 0) return;
  uint8_t mod = 0;
  uint8_t key = 0;
  if (s.equalsIgnoreCase("ENTER")) key = 0x28;
  else if (s.equalsIgnoreCase("TAB")) key = 0x2B;
  else if (s.equalsIgnoreCase("SPACE")) key = 0x2C;
  else if (s.equalsIgnoreCase("ESC") || s.equalsIgnoreCase("ESCAPE")) key = 0x29;
  else if (!asciiToHid(s[0], mod, key)) return;
  tapKey(key, mod | extraModifier);
}

String trimLine(String s) {
  s.trim();
  return s;
}

void runLine(const String& line) {
  if (line.isEmpty() || line.startsWith("#") || line.startsWith("//")) return;
  if (line.startsWith("DELAY ")) {
    delay(static_cast<uint32_t>(line.substring(6).toInt()));
  } else if (line.startsWith("STRING ")) {
    typeText(line.substring(7));
  } else if (line.startsWith("GUI ") || line.startsWith("WINDOWS ")) {
    const int offset = line.startsWith("GUI ") ? 4 : 8;
    tapAsciiChord(line.substring(offset), 0x08);
  } else if (line == "ENTER") {
    tapKey(0x28);
  } else if (line == "TAB") {
    tapKey(0x2B);
  } else if (line == "SPACE") {
    tapKey(0x2C);
  } else if (line == "ESC" || line == "ESCAPE") {
    tapKey(0x29);
  }
}

void runPayload() {
  g_running = true;
  g_done = false;
  g_lineNo = 0;
  drawStatus("Running", "Built-in payload...", TFT_GREEN);

  String line;
  const char* p = kBuiltInPayload;
  while (*p) {
    char c = *p++;
    if (c == '\r') continue;
    if (c == '\n') {
      ++g_lineNo;
      runLine(trimLine(line));
      line = "";
      continue;
    }
    line += c;
  }
  if (!line.isEmpty()) {
    ++g_lineNo;
    runLine(trimLine(line));
  }

  releaseKeys();
  g_running = false;
  g_done = true;
  drawStatus("Done", String("Lines: ") + g_lineNo, TFT_GREEN);
}

}  // namespace

void setup() {
  pinMode(kArmButton, INPUT_PULLUP);
  Serial.begin(115200);
  ch9329.begin(kCh9329Baud, SERIAL_8N1, kCh9329Rx, kCh9329Tx);

  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  drawStatus("Ready", "Hold BOOT to run", TFT_CYAN);
}

void loop() {
  if (g_running) return;

  const bool pressed = digitalRead(kArmButton) == LOW;
  if (pressed) {
    if (g_pressStart == 0) g_pressStart = millis();
    if (millis() - g_pressStart > 1000 && !g_done) {
      runPayload();
    }
  } else {
    g_pressStart = 0;
    if (g_done) {
      delay(800);
      g_done = false;
      drawStatus("Ready", "Hold BOOT to run", TFT_CYAN);
    }
  }
  delay(10);
}

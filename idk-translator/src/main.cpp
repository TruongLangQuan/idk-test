#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <array>
#include <cstring>
#include <lgfx/v1/misc/DataWrapper.hpp>

#include "generated/vi12_font.h"

namespace {

constexpr int kMaxInputBytes = 512;
constexpr int kMaxOutputBytes = 768;
constexpr uint32_t kHttpTimeoutMs = 12000;
constexpr uint32_t kLongPressMs = 700;

const char* kDefaultEndpoint = "https://libretranslate.de/translate";

const char* kTargetLangs[] = {"en", "fr", "de", "es", "ja", "ko", "zh", "ru", "th"};
constexpr size_t kTargetLangCount = sizeof(kTargetLangs) / sizeof(kTargetLangs[0]);

struct VowelGroup {
  const char* base;
  const char* tone[5];
};

// Tone order: s (acute), f (grave), r (hook), x (tilde), j (dot)
const VowelGroup kVowelsLower[] = {
    {"a", {u8"\u00E1", u8"\u00E0", u8"\u1EA3", u8"\u00E3", u8"\u1EA1"}},
    {u8"\u0103", {u8"\u1EAF", u8"\u1EB1", u8"\u1EB3", u8"\u1EB5", u8"\u1EB7"}},
    {u8"\u00E2", {u8"\u1EA5", u8"\u1EA7", u8"\u1EA9", u8"\u1EAB", u8"\u1EAD"}},
    {"e", {u8"\u00E9", u8"\u00E8", u8"\u1EBB", u8"\u1EBD", u8"\u1EB9"}},
    {u8"\u00EA", {u8"\u1EBF", u8"\u1EC1", u8"\u1EC3", u8"\u1EC5", u8"\u1EC7"}},
    {"i", {u8"\u00ED", u8"\u00EC", u8"\u1EC9", u8"\u0129", u8"\u1ECB"}},
    {"o", {u8"\u00F3", u8"\u00F2", u8"\u1ECF", u8"\u00F5", u8"\u1ECD"}},
    {u8"\u00F4", {u8"\u1ED1", u8"\u1ED3", u8"\u1ED5", u8"\u1ED7", u8"\u1ED9"}},
    {u8"\u01A1", {u8"\u1EDB", u8"\u1EDD", u8"\u1EDF", u8"\u1EE1", u8"\u1EE3"}},
    {"u", {u8"\u00FA", u8"\u00F9", u8"\u1EE7", u8"\u0169", u8"\u1EE5"}},
    {u8"\u01B0", {u8"\u1EE9", u8"\u1EEB", u8"\u1EED", u8"\u1EEF", u8"\u1EF1"}},
    {"y", {u8"\u00FD", u8"\u1EF3", u8"\u1EF7", u8"\u1EF9", u8"\u1EF5"}},
};

const VowelGroup kVowelsUpper[] = {
    {"A", {u8"\u00C1", u8"\u00C0", u8"\u1EA2", u8"\u00C3", u8"\u1EA0"}},
    {u8"\u0102", {u8"\u1EAE", u8"\u1EB0", u8"\u1EB2", u8"\u1EB4", u8"\u1EB6"}},
    {u8"\u00C2", {u8"\u1EA4", u8"\u1EA6", u8"\u1EA8", u8"\u1EAA", u8"\u1EAC"}},
    {"E", {u8"\u00C9", u8"\u00C8", u8"\u1EBA", u8"\u1EBC", u8"\u1EB8"}},
    {u8"\u00CA", {u8"\u1EBE", u8"\u1EC0", u8"\u1EC2", u8"\u1EC4", u8"\u1EC6"}},
    {"I", {u8"\u00CD", u8"\u00CC", u8"\u1EC8", u8"\u0128", u8"\u1ECA"}},
    {"O", {u8"\u00D3", u8"\u00D2", u8"\u1ECE", u8"\u00D5", u8"\u1ECC"}},
    {u8"\u00D4", {u8"\u1ED0", u8"\u1ED2", u8"\u1ED4", u8"\u1ED6", u8"\u1ED8"}},
    {u8"\u01A0", {u8"\u1EDA", u8"\u1EDC", u8"\u1EDE", u8"\u1EE0", u8"\u1EE2"}},
    {"U", {u8"\u00DA", u8"\u00D9", u8"\u1EE6", u8"\u0168", u8"\u1EE4"}},
    {u8"\u01AF", {u8"\u1EE8", u8"\u1EEA", u8"\u1EEC", u8"\u1EEE", u8"\u1EF0"}},
    {"Y", {u8"\u00DD", u8"\u1EF2", u8"\u1EF6", u8"\u1EF8", u8"\u1EF4"}},
};

class MemoryFontWrapper : public lgfx::v1::DataWrapper {
 public:
  MemoryFontWrapper(const uint8_t* data, size_t len) : data_(data), len_(len), pos_(0) {}

  bool open(const char*) override {
    pos_ = 0;
    return data_ != nullptr && len_ > 0;
  }

  int read(uint8_t* buf, uint32_t len) override {
    if (pos_ >= len_) return 0;
    size_t remain = len_ - pos_;
    size_t n = len < remain ? len : remain;
    memcpy(buf, data_ + pos_, n);
    pos_ += n;
    return static_cast<int>(n);
  }

  void skip(int32_t offset) override {
    int64_t next = static_cast<int64_t>(pos_) + offset;
    if (next < 0) next = 0;
    if (static_cast<size_t>(next) > len_) next = static_cast<int64_t>(len_);
    pos_ = static_cast<size_t>(next);
  }

  bool seek(uint32_t offset) override {
    if (offset > len_) return false;
    pos_ = offset;
    return true;
  }

  void close(void) override {}

  int32_t tell(void) override { return static_cast<int32_t>(pos_); }

 private:
  const uint8_t* data_;
  size_t len_;
  size_t pos_;
};

MemoryFontWrapper g_font_wrap(vi12_font, vi12_font_len);

enum class ScreenState {
  WIFI,
  MAIN,
};

ScreenState g_state = ScreenState::WIFI;

Preferences g_prefs;
WebServer g_server(80);

String g_endpoint;
String g_api_key;
String g_input_text = "";
String g_output_text = "";
String g_status = "Ready";
String g_src_lang = "auto";
String g_tgt_lang = "vi";
String g_alt_tgt_lang = "en";

bool g_font_loaded = false;
bool g_telex_mode = true;
bool g_server_started = false;

std::array<String, 16> g_ssids{};
std::array<int, 16> g_rssi{};
std::array<bool, 16> g_secured{};
int g_wifi_count = 0;
int g_wifi_index = 0;
String g_wifi_status = "Scanning...";
uint32_t g_last_wifi_scan = 0;

struct KnownWifi {
  const char* ssid;
  const char* pass;
};

constexpr KnownWifi kKnownWifis[] = {
    {"THCS NTD 02", "thcsnguyenthidinh"},
    {"THCS Nguyen Thi Dinh", "thcsnguyenthidinh"},
    {"Quan Le", "15032011"},
    {"NTD-THCS", "GIAOVIEN2425@"},
};

String getLocalIpString() {
  if (WiFi.status() != WL_CONNECTED) return String("0.0.0.0");
  return WiFi.localIP().toString();
}

int utf8PrevStart(const String& s, int from) {
  int i = from - 1;
  while (i > 0 && (static_cast<uint8_t>(s[i]) & 0xC0) == 0x80) {
    --i;
  }
  return i;
}

String utf8LastCodepoint(const String& s, int* outStart = nullptr) {
  if (s.isEmpty()) {
    if (outStart) *outStart = 0;
    return String("");
  }
  int end = s.length();
  int start = utf8PrevStart(s, end);
  if (outStart) *outStart = start;
  return s.substring(start, end);
}

void removeLastCodepoint(String& s) {
  if (s.isEmpty()) return;
  int start = 0;
  (void)utf8LastCodepoint(s, &start);
  s.remove(start);
}

bool replaceLastCodepoint(String& s, const char* replacement) {
  if (s.isEmpty()) return false;
  int start = 0;
  (void)utf8LastCodepoint(s, &start);
  s.remove(start);
  s += replacement;
  return true;
}

const VowelGroup* findVowelGroup(const String& cp) {
  for (const auto& group : kVowelsLower) {
    if (cp == group.base) return &group;
    for (const auto& tone : group.tone) {
      if (cp == tone) return &group;
    }
  }
  for (const auto& group : kVowelsUpper) {
    if (cp == group.base) return &group;
    for (const auto& tone : group.tone) {
      if (cp == tone) return &group;
    }
  }
  return nullptr;
}

bool applyToneToLastVowel(String& s, int toneIndex) {
  if (toneIndex < 0 || toneIndex >= 5) return false;
  int i = s.length();
  while (i > 0) {
    int start = utf8PrevStart(s, i);
    String cp = s.substring(start, i);
    const VowelGroup* group = findVowelGroup(cp);
    if (group != nullptr) {
      s.remove(start, i - start);
      s += group->tone[toneIndex];
      return true;
    }
    i = start;
  }
  return false;
}

bool applyTelexCombo(String& s, char ch) {
  if (s.isEmpty()) return false;
  int start = 0;
  String last = utf8LastCodepoint(s, &start);

  if (ch == 'w' || ch == 'W') {
    if (last == "a") return replaceLastCodepoint(s, u8"\u0103");
    if (last == "A") return replaceLastCodepoint(s, u8"\u0102");
    if (last == "o") return replaceLastCodepoint(s, u8"\u01A1");
    if (last == "O") return replaceLastCodepoint(s, u8"\u01A0");
    if (last == "u") return replaceLastCodepoint(s, u8"\u01B0");
    if (last == "U") return replaceLastCodepoint(s, u8"\u01AF");
    return false;
  }

  if (ch == 'a' || ch == 'A') {
    if (last == "a") return replaceLastCodepoint(s, u8"\u00E2");
    if (last == "A") return replaceLastCodepoint(s, u8"\u00C2");
    return false;
  }

  if (ch == 'e' || ch == 'E') {
    if (last == "e") return replaceLastCodepoint(s, u8"\u00EA");
    if (last == "E") return replaceLastCodepoint(s, u8"\u00CA");
    return false;
  }

  if (ch == 'o' || ch == 'O') {
    if (last == "o") return replaceLastCodepoint(s, u8"\u00F4");
    if (last == "O") return replaceLastCodepoint(s, u8"\u00D4");
    return false;
  }

  if (ch == 'd' || ch == 'D') {
    if (last == "d") return replaceLastCodepoint(s, u8"\u0111");
    if (last == "D") return replaceLastCodepoint(s, u8"\u0110");
    return false;
  }

  return false;
}

bool applyTelexInput(String& s, char ch) {
  const char lower = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
  if (lower == 's' || lower == 'f' || lower == 'r' || lower == 'x' || lower == 'j') {
    int toneIndex = -1;
    if (lower == 's') toneIndex = 0;
    if (lower == 'f') toneIndex = 1;
    if (lower == 'r') toneIndex = 2;
    if (lower == 'x') toneIndex = 3;
    if (lower == 'j') toneIndex = 4;
    if (applyToneToLastVowel(s, toneIndex)) return true;
  }

  if (applyTelexCombo(s, ch)) return true;

  return false;
}

String truncateUtf8(const String& s, size_t maxBytes) {
  if (s.length() <= maxBytes) return s;
  int cut = static_cast<int>(maxBytes);
  while (cut > 0 && (static_cast<uint8_t>(s[cut]) & 0xC0) == 0x80) {
    --cut;
  }
  if (cut <= 0) return String("");
  String out = s.substring(0, cut);
  out += "...";
  return out;
}

void saveSettings() {
  g_prefs.putString("endpoint", g_endpoint);
  g_prefs.putString("apikey", g_api_key);
  g_prefs.putString("src", g_src_lang);
  g_prefs.putString("tgt", g_tgt_lang);
  g_prefs.putString("alt", g_alt_tgt_lang);
}

void loadSettings() {
  g_endpoint = g_prefs.getString("endpoint", kDefaultEndpoint);
  g_api_key = g_prefs.getString("apikey", "");
  g_src_lang = g_prefs.getString("src", "auto");
  g_tgt_lang = g_prefs.getString("tgt", "vi");
  g_alt_tgt_lang = g_prefs.getString("alt", "en");
}

void drawHeader(const char* title) {
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print(title);
}

void drawMain() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextWrap(false, false);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-translator");

  M5.Display.setCursor(2, 14);
  if (WiFi.status() == WL_CONNECTED) {
    M5.Display.printf("WiFi:%s", WiFi.SSID().c_str());
  } else {
    M5.Display.print("WiFi:offline");
  }

  M5.Display.setCursor(2, 24);
  M5.Display.printf("Dir: %s -> %s", g_src_lang.c_str(), g_tgt_lang.c_str());

  if (g_telex_mode) {
    M5.Display.setCursor(170, 24);
    M5.Display.print("TELEX");
  }

  M5.Display.setTextWrap(true, true);
  M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
  M5.Display.setCursor(2, 36);
  M5.Display.print("IN:");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 48);
  M5.Display.println(truncateUtf8(g_input_text, 140));

  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setCursor(2, 78);
  M5.Display.print("OUT:");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 90);
  M5.Display.println(truncateUtf8(g_output_text, 170));

  M5.Display.setTextWrap(false, false);
  M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Display.setCursor(2, 122);
  M5.Display.print(g_status);

  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setCursor(2, 132);
  M5.Display.print("A:Input  B:Trans  PWR:Swap  PWR+hold:WiFi");
}

void drawWifi() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-translator WiFi");
  M5.Display.setCursor(2, 14);
  M5.Display.print("A:connect  B:next  PWR:prev");
  M5.Display.setCursor(2, 24);
  M5.Display.print("Hold PWR: exit");

  if (g_wifi_count <= 0) {
    M5.Display.setCursor(2, 42);
    M5.Display.print("No AP found");
    M5.Display.setCursor(2, 126);
    M5.Display.print(g_wifi_status);
    return;
  }

  int start = std::max(0, g_wifi_index - 2);
  int end = std::min(g_wifi_count, start + 6);
  int y = 42;
  for (int i = start; i < end; ++i) {
    M5.Display.setTextColor(i == g_wifi_index ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(4, y);
    M5.Display.printf("%c %s (%d)", i == g_wifi_index ? '>' : ' ', g_ssids[i].c_str(), g_rssi[i]);
    y += 12;
  }
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 126);
  M5.Display.print(g_wifi_status);
}

void scanWifi() {
  g_wifi_status = "Scanning...";
  drawWifi();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(80);
  g_wifi_count = WiFi.scanNetworks();
  if (g_wifi_count < 0) g_wifi_count = 0;
  if (g_wifi_count > static_cast<int>(g_ssids.size())) g_wifi_count = g_ssids.size();
  for (int i = 0; i < g_wifi_count; ++i) {
    g_ssids[i] = WiFi.SSID(i);
    g_rssi[i] = WiFi.RSSI(i);
    g_secured[i] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  }
  g_wifi_index = 0;
  if (g_wifi_count > 0) {
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
      while (millis() - start < 8000) {
        if (WiFi.status() == WL_CONNECTED) {
          g_wifi_status = String("Connected: ") + known.ssid;
          drawWifi();
          delay(200);
          g_state = ScreenState::MAIN;
          g_status = String("IP: ") + getLocalIpString();
          drawMain();
          return;
        }
        delay(120);
      }
    }
    WiFi.disconnect(true, true);
  }
  g_wifi_status = g_wifi_count > 0 ? "Select WiFi" : "No WiFi";
  drawWifi();
}

bool inputPassword(String& out) {
  String value = "";
  const char* top[] = {"OK", "DEL", "SP", "ESC"};
  const char* row1 = "1234567890";
  const char* row2 = "qwertyuiop";
  const char* row3 = "asdfghjkl";
  const char* row4 = "zxcvbnm-_.@";

  int row = 0;
  int col = 0;

  auto rowLen = [&](int r) {
    if (r == 0) return 4;
    if (r == 1) return static_cast<int>(strlen(row1));
    if (r == 2) return static_cast<int>(strlen(row2));
    if (r == 3) return static_cast<int>(strlen(row3));
    return static_cast<int>(strlen(row4));
  };

  auto draw = [&]() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, 2);
    M5.Display.println("Password");
    M5.Display.setCursor(2, 14);
    M5.Display.print("Len: ");
    M5.Display.print(value.length());

    M5.Display.drawRect(2, 24, 236, 12, TFT_DARKGREY);
    M5.Display.setCursor(4, 26);
    for (size_t i = 0; i < value.length() && i < 42; ++i) M5.Display.print('*');

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

bool connectSelectedWifi() {
  if (g_wifi_count <= 0) return false;
  const String ssid = g_ssids[g_wifi_index];
  const bool secured = g_secured[g_wifi_index];
  String pass = "";
  if (secured) {
    if (!inputPassword(pass)) {
      g_wifi_status = "Canceled";
      drawWifi();
      delay(200);
      return false;
    }
  }

  g_wifi_status = String("Connecting: ") + ssid;
  drawWifi();

  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_STA);
  if (secured) WiFi.begin(ssid.c_str(), pass.c_str());
  else WiFi.begin(ssid.c_str());

  uint32_t start = millis();
  while (millis() - start < 12000) {
    if (WiFi.status() == WL_CONNECTED) {
      g_wifi_status = String("Connected: ") + ssid;
      drawWifi();
      delay(200);
      return true;
    }
    delay(120);
  }

  g_wifi_status = "Connect failed";
  WiFi.disconnect(true, true);
  drawWifi();
  delay(200);
  return false;
}

void handleWifiScreen() {
  if (g_wifi_count == 0 && millis() - g_last_wifi_scan > 300) {
    g_last_wifi_scan = millis();
    scanWifi();
  }

  if (M5.BtnB.wasPressed() && g_wifi_count > 0) {
    g_wifi_index = (g_wifi_index + 1) % g_wifi_count;
    drawWifi();
  }

  if (M5.BtnPWR.wasPressed() && g_wifi_count > 0) {
    g_wifi_index = (g_wifi_index - 1 + g_wifi_count) % g_wifi_count;
    drawWifi();
  }

  if (M5.BtnA.wasPressed()) {
    if (connectSelectedWifi()) {
      g_state = ScreenState::MAIN;
      g_status = String("IP: ") + getLocalIpString();
      drawMain();
    }
  }

  if (M5.BtnPWR.pressedFor(kLongPressMs)) {
    g_state = ScreenState::MAIN;
    g_status = "Offline";
    drawMain();
    delay(200);
  }
}

bool inputText(String& out) {
  const char* row1 = "1234567890";
  const char* row2 = "qwertyuiop";
  const char* row3 = "asdfghjkl";
  const char* row4 = "zxcvbnm-_.@";
  const char* top[] = {"OK", "DEL", "SP", "NL", "ESC"};
  constexpr int topCount = 5;

  int row = 0;
  int col = 0;
  String value = out;

  auto rowLen = [&](int r) -> int {
    if (r == 0) return topCount;
    if (r == 1) return static_cast<int>(strlen(row1));
    if (r == 2) return static_cast<int>(strlen(row2));
    if (r == 3) return static_cast<int>(strlen(row3));
    return static_cast<int>(strlen(row4));
  };

  auto draw = [&]() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, 2);
    M5.Display.print("Input Text (");
    M5.Display.print(g_telex_mode ? "VI" : "EN");
    M5.Display.print(")");
    M5.Display.setCursor(2, 14);
    M5.Display.print("Len: ");
    M5.Display.print(value.length());

    M5.Display.drawRect(2, 24, 236, 12, TFT_DARKGREY);
    M5.Display.setCursor(4, 26);
    M5.Display.print(truncateUtf8(value, 38));

    int y = 42;
    for (int i = 0; i < topCount; ++i) {
      M5.Display.setTextColor((row == 0 && col == i) ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(4 + i * 46, y);
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

    M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Display.setCursor(2, 122);
    M5.Display.print("Hold PWR: toggle TELEX");
  };

  draw();
  bool longHandled = false;
  while (true) {
    M5.update();
    if (M5.BtnPWR.pressedFor(kLongPressMs) && !longHandled) {
      longHandled = true;
      g_telex_mode = !g_telex_mode;
      draw();
    }
    if (M5.BtnPWR.wasReleased()) {
      longHandled = false;
    }
    if (M5.BtnPWR.wasPressed() && !longHandled) {
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
        if (col == 1) { if (!value.isEmpty()) removeLastCodepoint(value); }
        if (col == 2) { if (value.length() + 1 <= kMaxInputBytes) value += ' '; }
        if (col == 3) { if (value.length() + 1 <= kMaxInputBytes) value += '\n'; }
        if (col == 4) { return false; }
      } else {
        char ch = 0;
        if (row == 1) ch = row1[col];
        if (row == 2) ch = row2[col];
        if (row == 3) ch = row3[col];
        if (row == 4) ch = row4[col];
        if (ch) {
          if (g_telex_mode) {
            if (!applyTelexInput(value, ch)) {
              if (value.length() + 1 <= kMaxInputBytes) value += ch;
            }
          } else {
            if (value.length() + 1 <= kMaxInputBytes) value += ch;
          }
        }
      }
      draw();
    }
    delay(8);
  }
}

bool translateText(const String& text, String& out, String& err) {
  if (text.isEmpty()) {
    err = "Empty input";
    return false;
  }
  if (text.length() > kMaxInputBytes) {
    err = "Input too long";
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) {
    err = "WiFi offline";
    return false;
  }

  String endpoint = g_endpoint;
  if (endpoint.isEmpty()) endpoint = kDefaultEndpoint;

  WiFiClientSecure client;
  if (endpoint.startsWith("https://")) {
    client.setInsecure();
  }

  HTTPClient http;
  http.setTimeout(kHttpTimeoutMs);
  if (!http.begin(client, endpoint)) {
    err = "HTTP begin failed";
    return false;
  }

  DynamicJsonDocument doc(1024 + text.length());
  doc["q"] = text;
  doc["source"] = g_src_lang;
  doc["target"] = g_tgt_lang;
  doc["format"] = "text";
  if (!g_api_key.isEmpty()) doc["api_key"] = g_api_key;

  String payload;
  serializeJson(doc, payload);

  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(payload);
  if (httpCode < 200 || httpCode >= 300) {
    err = String("HTTP ") + httpCode;
    http.end();
    return false;
  }

  String resp = http.getString();
  http.end();
  if (resp.isEmpty()) {
    err = "Empty response";
    return false;
  }

  DynamicJsonDocument resDoc(1024 + resp.length());
  DeserializationError jerr = deserializeJson(resDoc, resp);
  if (jerr) {
    err = "JSON parse failed";
    return false;
  }

  const char* translated = resDoc["translatedText"] | "";
  if (translated == nullptr || strlen(translated) == 0) {
    err = "No translatedText";
    return false;
  }
  out = truncateUtf8(String(translated), kMaxOutputBytes);
  return true;
}

void applyDirectionToggle() {
  if (g_src_lang == "auto" && g_tgt_lang == "vi") {
    g_src_lang = "vi";
    g_tgt_lang = g_alt_tgt_lang;
  } else {
    if (g_src_lang == "vi") g_alt_tgt_lang = g_tgt_lang;
    g_src_lang = "auto";
    g_tgt_lang = "vi";
  }
  saveSettings();
}

void advanceTargetLang() {
  size_t idx = 0;
  for (size_t i = 0; i < kTargetLangCount; ++i) {
    if (g_alt_tgt_lang == kTargetLangs[i]) {
      idx = i;
      break;
    }
  }
  idx = (idx + 1) % kTargetLangCount;
  g_alt_tgt_lang = kTargetLangs[idx];
  if (g_src_lang == "vi") {
    g_tgt_lang = g_alt_tgt_lang;
  }
  saveSettings();
}

void doTranslate() {
  g_status = "Translating...";
  drawMain();

  String err;
  String out;
  if (translateText(g_input_text, out, err)) {
    g_output_text = out;
    g_status = "Done";
  } else {
    g_status = err;
  }
  drawMain();
}

String htmlPage() {
  String html;
  html.reserve(3000);
  html += R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>idk-translator</title>
<style>
  body { font-family: Arial, sans-serif; margin: 16px; background:#0e0f12; color:#e6e6e6; }
  textarea, input { width: 100%; box-sizing: border-box; margin: 6px 0; padding: 10px; background:#1b1c21; color:#e6e6e6; border:1px solid #333; border-radius:8px; }
  button { padding: 10px 14px; margin: 6px 4px 6px 0; border:0; border-radius:8px; background:#2b7cff; color:#fff; }
  .row { display:flex; gap:8px; }
  .row > div { flex:1; }
  .status { margin-top:10px; font-size:14px; color:#9aa0a6; }
</style>
</head>
<body>
<h2>idk-translator</h2>
<div class="row">
  <div>
    <label>Source lang</label>
    <input id="src" placeholder="auto" />
  </div>
  <div>
    <label>Target lang</label>
    <input id="tgt" placeholder="vi" />
  </div>
</div>
<label>Text</label>
<textarea id="text" rows="6" placeholder="Type any language here..."></textarea>
<div>
  <button onclick="sendInput()">Send To Device</button>
  <button onclick="doTranslate()">Translate</button>
  <button onclick="refreshState()">Refresh</button>
</div>
<label>Endpoint (LibreTranslate)</label>
<input id="endpoint" placeholder="https://libretranslate.de/translate" />
<label>API Key (optional)</label>
<input id="apikey" placeholder="" />
<div>
  <button onclick="saveConfig()">Save Config</button>
</div>
<label>Output</label>
<textarea id="out" rows="6" readonly></textarea>
<div class="status" id="status">Ready</div>
<script>
async function sendInput() {
  const payload = new URLSearchParams();
  payload.set('text', document.getElementById('text').value);
  payload.set('src', document.getElementById('src').value || '');
  payload.set('tgt', document.getElementById('tgt').value || '');
  await fetch('/input', { method:'POST', body: payload });
  await refreshState();
}
async function doTranslate() {
  const res = await fetch('/translate');
  const data = await res.json();
  document.getElementById('out').value = data.output || '';
  document.getElementById('status').innerText = data.status || 'Done';
}
async function saveConfig() {
  const payload = new URLSearchParams();
  payload.set('endpoint', document.getElementById('endpoint').value || '');
  payload.set('apikey', document.getElementById('apikey').value || '');
  await fetch('/config', { method:'POST', body: payload });
  await refreshState();
}
async function refreshState() {
  const res = await fetch('/state');
  const data = await res.json();
  document.getElementById('src').value = data.src || '';
  document.getElementById('tgt').value = data.tgt || '';
  document.getElementById('endpoint').value = data.endpoint || '';
  document.getElementById('status').innerText = data.status || '';
  document.getElementById('out').value = data.output || '';
}
refreshState();
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  g_server.send(200, "text/html; charset=utf-8", htmlPage());
}

void handleInput() {
  String text = g_server.arg("text");
  if (!text.isEmpty()) {
    g_input_text = text;
  }
  String src = g_server.arg("src");
  String tgt = g_server.arg("tgt");
  if (!src.isEmpty()) g_src_lang = src;
  if (!tgt.isEmpty()) g_tgt_lang = tgt;
  if (g_src_lang == "vi") g_alt_tgt_lang = g_tgt_lang;
  g_status = "Web input updated";
  saveSettings();
  drawMain();
  g_server.send(200, "text/plain", "OK");
}

void handleConfig() {
  String endpoint = g_server.arg("endpoint");
  String apikey = g_server.arg("apikey");
  if (!endpoint.isEmpty()) g_endpoint = endpoint;
  if (!apikey.isEmpty()) g_api_key = apikey;
  saveSettings();
  g_status = "Config saved";
  drawMain();
  g_server.send(200, "text/plain", "OK");
}

void handleState() {
  DynamicJsonDocument doc(768);
  doc["src"] = g_src_lang;
  doc["tgt"] = g_tgt_lang;
  doc["endpoint"] = g_endpoint;
  doc["status"] = g_status;
  doc["output"] = g_output_text;
  doc["ip"] = getLocalIpString();
  String payload;
  serializeJson(doc, payload);
  g_server.send(200, "application/json", payload);
}

void handleTranslate() {
  String err;
  String out;
  if (translateText(g_input_text, out, err)) {
    g_output_text = out;
    g_status = "Done";
  } else {
    g_status = err;
  }
  drawMain();
  DynamicJsonDocument doc(768 + g_output_text.length());
  doc["status"] = g_status;
  doc["output"] = g_output_text;
  String payload;
  serializeJson(doc, payload);
  g_server.send(200, "application/json", payload);
}

void startWebServer() {
  if (g_server_started) return;
  g_server.on("/", handleRoot);
  g_server.on("/input", HTTP_POST, handleInput);
  g_server.on("/translate", HTTP_GET, handleTranslate);
  g_server.on("/state", HTTP_GET, handleState);
  g_server.on("/config", HTTP_POST, handleConfig);
  g_server.begin();
  g_server_started = true;
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);

  g_font_loaded = g_font_wrap.open(nullptr) && M5.Display.loadFont(&g_font_wrap);
  if (!g_font_loaded) {
    g_status = "Font load failed";
  }

  g_prefs.begin("translator", false);
  loadSettings();

  g_state = ScreenState::WIFI;
  scanWifi();
}

void loop() {
  M5.update();

  if (g_state == ScreenState::WIFI) {
    handleWifiScreen();
  } else {
    if (WiFi.status() == WL_CONNECTED) {
      startWebServer();
      g_server.handleClient();
    }

    static bool pwrLongHandled = false;
    if (M5.BtnPWR.pressedFor(kLongPressMs) && !pwrLongHandled) {
      pwrLongHandled = true;
      g_state = ScreenState::WIFI;
      scanWifi();
      return;
    }
    if (M5.BtnPWR.wasReleased()) {
      pwrLongHandled = false;
    }

    if (M5.BtnA.wasPressed()) {
      if (inputText(g_input_text)) {
        g_status = "Input updated";
      } else {
        g_status = "Input canceled";
      }
      drawMain();
    }

    if (M5.BtnB.wasPressed()) {
      doTranslate();
    }

    if (M5.BtnB.pressedFor(kLongPressMs)) {
      advanceTargetLang();
      g_status = String("Target: ") + g_alt_tgt_lang;
      drawMain();
      delay(200);
    }

    if (M5.BtnPWR.wasPressed() && !pwrLongHandled) {
      applyDirectionToggle();
      g_status = "Direction swapped";
      drawMain();
    }
  }

  delay(8);
}

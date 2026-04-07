#include "idk_ui.h"
#include <cstring>

namespace {

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

constexpr int kKeyboardRows = 4;
constexpr int kKeyboardCols = 12;
const char kQwertyKeys[kKeyboardRows][kKeyboardCols][2] = {
    {{'1', '!'},
     {'2', '@'},
     {'3', '#'},
     {'4', '$'},
     {'5', '%'},
     {'6', '^'},
     {'7', '&'},
     {'8', '*'},
     {'9', '('},
     {'0', ')'},
     {'-', '_'},
     {'=', '+'}},
    {{'q', 'Q'},
     {'w', 'W'},
     {'e', 'E'},
     {'r', 'R'},
     {'t', 'T'},
     {'y', 'Y'},
     {'u', 'U'},
     {'i', 'I'},
     {'o', 'O'},
     {'p', 'P'},
     {'[', '{'},
     {']', '}'}},
    {{'a', 'A'},
     {'s', 'S'},
     {'d', 'D'},
     {'f', 'F'},
     {'g', 'G'},
     {'h', 'H'},
     {'j', 'J'},
     {'k', 'K'},
     {'l', 'L'},
     {';', ':'},
     {'"', '\''},
     {'|', '\\'}},
    {{'\\', '|'},
     {'z', 'Z'},
     {'x', 'X'},
     {'c', 'C'},
     {'v', 'V'},
     {'b', 'B'},
     {'n', 'N'},
     {'m', 'M'},
     {',', '<'},
     {'.', '>'},
     {'?', '/'},
     {'/', '/'}},
};

const char* kMathKeys[kKeyboardRows][kKeyboardCols] = {
    {"7", "8", "9", "/", "sin(", "cos(", "tan(", "log(", "ln(", "sqrt(", "pi", "e"},
    {"4", "5", "6", "*", "asin(", "acos(", "atan(", "abs(", "^", "(", ")", ","},
    {"1", "2", "3", "-", "x", "y", "t", "=", "<=", ">=", "min(", "max("},
    {"0", ".", "+", "%", "deg", "rad", "exp(", "pow(", "floor(", "ceil(", "", ""},
};

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

}  // namespace

bool loadFontFromMemory(const uint8_t* data, size_t len, MemoryFontWrapper& wrap) {
  return wrap.open(nullptr) && M5.Display.loadFont(&wrap);
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

String formatFloat(double value, int decimals) {
  char buf[32];
  dtostrf(value, 0, decimals, buf);
  String out(buf);
  out.trim();
  if (out.indexOf('.') >= 0) {
    while (out.endsWith("0")) out.remove(out.length() - 1);
    if (out.endsWith(".")) out.remove(out.length() - 1);
  }
  if (out == "-0") out = "0";
  return out;
}

bool keyboardInput(String& out, const String& title, bool mask_input, bool allow_telex, bool& telexMode,
                   size_t maxLen, bool allow_math) {
  const char* buttons_all[7];
  int btnCount = 0;
  buttons_all[btnCount++] = "OK";
  buttons_all[btnCount++] = "CAP";
  buttons_all[btnCount++] = "DEL";
  buttons_all[btnCount++] = "SPACE";
  buttons_all[btnCount++] = "BACK";
  if (allow_telex) buttons_all[btnCount++] = "VI";
  if (allow_math) buttons_all[btnCount++] = "MATH";
  constexpr int gap = 2;
  constexpr int btnH = 16;

  const int screenW = M5.Display.width();
  const int screenH = M5.Display.height();
  const int btnW = (screenW - gap * (btnCount + 1)) / btnCount;
  const int btnY = 2;
  const int titleY = btnY + btnH + 2;
  const int boxY = titleY + 10;
  const int boxH = 18;
  const int gridY = boxY + boxH + 4;
  const int keyW = screenW / kKeyboardCols;
  const int keyH = (screenH - gridY) / kKeyboardRows;

  bool caps = false;
  bool mathMode = false;
  int x = 0;
  int y = -1;  // -1 = top buttons row
  String value = out;

  auto draw = [&]() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextWrap(false, false);
    M5.Display.setTextSize(1);

    // Top buttons
    for (int i = 0; i < btnCount; ++i) {
      const int bx = gap + i * (btnW + gap);
      const bool selected = (y == -1 && x == i);
      bool active = false;
      if (i == 1 && caps) active = true;
      if (allow_telex) {
        int viIndex = 5;
        if (allow_math) viIndex = 5;
        if (allow_telex && i == viIndex && telexMode) active = true;
      }
      if (allow_math) {
        int mathIndex = allow_telex ? 6 : 5;
        if (i == mathIndex && mathMode) active = true;
      }

      uint16_t bg = selected ? TFT_WHITE : (active ? TFT_DARKGREY : TFT_BLACK);
      uint16_t fg = selected ? TFT_BLACK : TFT_WHITE;

      M5.Display.fillRect(bx, btnY, btnW, btnH, bg);
      M5.Display.drawRect(bx, btnY, btnW, btnH, TFT_WHITE);
      const int tw = M5.Display.textWidth(buttons_all[i]);
      const int th = M5.Display.fontHeight();
      const int tx = bx + (btnW - tw) / 2;
      const int ty = btnY + (btnH - th) / 2;
      M5.Display.setTextColor(fg, bg);
      M5.Display.setCursor(tx, ty);
      M5.Display.print(buttons_all[i]);
    }

    // Title + counter
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, titleY);
    M5.Display.print(title);
    if (allow_telex) {
      M5.Display.print(" (");
      M5.Display.print(telexMode ? "VI" : "EN");
      M5.Display.print(")");
    }

    String counter = String(value.length()) + "/" + String(maxLen);
    int cw = M5.Display.textWidth(counter.c_str());
    M5.Display.setCursor(screenW - cw - 2, titleY);
    M5.Display.print(counter);

    // Input box
    M5.Display.drawRect(2, boxY, screenW - 4, boxH, TFT_DARKGREY);
    M5.Display.setCursor(4, boxY + 2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    String displayValue = value;
    if (mask_input) {
      displayValue = "";
      displayValue.reserve(value.length());
      for (size_t i = 0; i < value.length(); ++i) displayValue += '*';
    }
    M5.Display.print(truncateUtf8(displayValue, 48));

    // Keyboard grid
    for (int row = 0; row < kKeyboardRows; ++row) {
      for (int col = 0; col < kKeyboardCols; ++col) {
        const int kx = col * keyW;
        const int ky = gridY + row * keyH;
        const bool selected = (y == row && x == col);
        uint16_t bg = selected ? TFT_WHITE : TFT_BLACK;
        uint16_t fg = selected ? TFT_BLACK : TFT_WHITE;
        M5.Display.fillRect(kx, ky, keyW, keyH, bg);
        M5.Display.drawRect(kx, ky, keyW, keyH, TFT_DARKGREY);

        String label = "";
        if (mathMode && allow_math) {
          label = kMathKeys[row][col];
        } else {
          char ch = kQwertyKeys[row][col][caps ? 1 : 0];
          label = String(ch);
        }
        if (label.isEmpty()) continue;
        const int tw = M5.Display.textWidth(label.c_str());
        const int th = M5.Display.fontHeight();
        const int tx = kx + (keyW - tw) / 2;
        const int ty = ky + (keyH - th) / 2;
        M5.Display.setTextColor(fg, bg);
        M5.Display.setCursor(tx, ty);
        M5.Display.print(label);
      }
    }
  };

  draw();
  bool longBHandled = false;
  bool longPwrHandled = false;
  while (true) {
    M5.update();

    if (M5.BtnB.pressedFor(650) && !longBHandled) {
      longBHandled = true;
      int maxX = (y == -1) ? btnCount : kKeyboardCols;
      x = (x - 1 + maxX) % maxX;
      draw();
    }
    if (M5.BtnB.wasReleased()) {
      longBHandled = false;
    }
    if (M5.BtnB.wasPressed() && !longBHandled) {
      int maxX = (y == -1) ? btnCount : kKeyboardCols;
      x = (x + 1) % maxX;
      draw();
    }

    if (M5.BtnPWR.pressedFor(650) && !longPwrHandled) {
      longPwrHandled = true;
      y--;
      if (y < -1) y = kKeyboardRows - 1;
      if (y == -1 && x >= btnCount) x = btnCount - 1;
      draw();
    }
    if (M5.BtnPWR.wasReleased()) {
      longPwrHandled = false;
    }
    if (M5.BtnPWR.wasPressed() && !longPwrHandled) {
      y++;
      if (y >= kKeyboardRows) y = -1;
      if (y == -1 && x >= btnCount) x = btnCount - 1;
      draw();
    }

    if (M5.BtnA.wasPressed()) {
      if (y == -1) {
        if (x == 0) { out = value; return true; }
        if (x == 1) { caps = !caps; }
        if (x == 2) { if (!value.isEmpty()) removeLastCodepoint(value); }
        if (x == 3) { if (value.length() + 1 <= maxLen) value += ' '; }
        if (x == 4) { return false; }
        if (allow_telex) {
          int viIndex = 5;
          if (x == viIndex) { telexMode = !telexMode; }
        }
        if (allow_math) {
          int mathIndex = allow_telex ? 6 : 5;
          if (x == mathIndex) { mathMode = !mathMode; }
        }
      } else {
        if (mathMode && allow_math) {
          const char* token = kMathKeys[y][x];
          if (token && token[0] != '\0') {
            size_t len = strlen(token);
            if (value.length() + len <= maxLen) value += token;
          }
        } else {
          char ch = kQwertyKeys[y][x][caps ? 1 : 0];
          if (allow_telex && telexMode) {
            if (!applyTelexInput(value, ch)) {
              if (value.length() + 1 <= maxLen) value += ch;
            }
          } else {
            if (value.length() + 1 <= maxLen) value += ch;
          }
        }
      }
      draw();
    }

    delay(8);
  }
}

bool inputNumber(double& out, const String& title, bool& telexMode, bool allowNegative) {
  String value = "";
  if (!keyboardInput(value, title, false, false, telexMode, 32)) return false;
  value.trim();
  if (value.isEmpty()) return false;
  if (!allowNegative && value.startsWith("-")) return false;
  char* endptr = nullptr;
  double v = strtod(value.c_str(), &endptr);
  if (endptr == value.c_str()) return false;
  out = v;
  return true;
}

bool autoConnectKnownWifi(const KnownWifi* list, size_t count, String& status, uint32_t timeoutMs) {
  if (count == 0 || list == nullptr) return false;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(60);

  int n = WiFi.scanNetworks();
  if (n <= 0) {
    status = "WiFi: none";
    return false;
  }

  for (size_t k = 0; k < count; ++k) {
    const KnownWifi& known = list[k];
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) == known.ssid) {
        status = String("WiFi: ") + known.ssid;
        WiFi.begin(known.ssid, known.pass);
        uint32_t start = millis();
        while (millis() - start < timeoutMs) {
          if (WiFi.status() == WL_CONNECTED) {
            status = String("WiFi OK: ") + known.ssid;
            return true;
          }
          delay(120);
        }
        WiFi.disconnect(true, true);
      }
    }
  }
  status = "WiFi: offline";
  return false;
}

void drawWrapped(const String& text, int x, int y, int maxW, int maxLines) {
  M5.Display.setTextWrap(false, false);
  int line = 0;
  String current = "";
  int i = 0;
  while (i < text.length() && line < maxLines) {
    char c = text[i++];
    if (c == '\n') {
      M5.Display.setCursor(x, y + line * 12);
      M5.Display.print(current);
      current = "";
      line++;
      continue;
    }
    current += c;
    if (M5.Display.textWidth(current.c_str()) > maxW) {
      current.remove(current.length() - 1);
      M5.Display.setCursor(x, y + line * 12);
      M5.Display.print(current);
      current = String(c);
      line++;
    }
  }
  if (line < maxLines && !current.isEmpty()) {
    M5.Display.setCursor(x, y + line * 12);
    M5.Display.print(current);
  }
}

void showResult(const char* title, const String& body) {
  while (true) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(2, 2);
    M5.Display.print(title);
    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
    drawWrapped(body, 2, 16, 236, 9);
    M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Display.setCursor(2, 124);
    M5.Display.print("A/B/PWR: back");

    M5.update();
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()) {
      break;
    }
    delay(10);
  }
}

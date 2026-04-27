#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>

namespace {

enum class UiMode {
  Browser,
  Viewer,
};

struct ButtonState {
  const char* name;
  int pin;
  bool pressed;
};

struct FsEntry {
  String name;
  bool isDir;
  bool isParent;
};

static constexpr int kDisplayMosi = 11;
static constexpr int kDisplaySck = 12;
static constexpr int kDisplayCs = 10;
static constexpr int kDisplayDc = 9;
static constexpr int kDisplayRst = 14;
static constexpr int kDisplayBl = 21;

static constexpr int kSdMosi = 15;
static constexpr int kSdSck = 16;
static constexpr int kSdMiso = 17;
static constexpr int kSdCs = 18;

static constexpr int kBtnUp = 4;
static constexpr int kBtnDown = 5;
static constexpr int kBtnLeft = 6;
static constexpr int kBtnRight = 7;
static constexpr int kBtnCenter = 8;

// UART to M5StickC Plus2 remote UI.
static constexpr int kPeerTx = 47;
static constexpr int kPeerRx = 48;
static constexpr uint32_t kPeerBaud = 115200;

static constexpr uint16_t kBg = TFT_BLACK;
static constexpr uint16_t kFg = TFT_WHITE;
static constexpr uint16_t kAccent = TFT_CYAN;
static constexpr uint16_t kOk = TFT_GREEN;
static constexpr uint16_t kWarn = TFT_YELLOW;
static constexpr uint16_t kErr = TFT_RED;
static constexpr uint32_t kSdRetryMs = 4000;
static constexpr uint32_t kHeartbeatMs = 1000;
static constexpr size_t kVisibleRows = 6;
static constexpr size_t kViewerLinesPerPage = 6;
static constexpr size_t kMaxBrowserEntries = 48;

ButtonState g_buttons[] = {
    {"UP", kBtnUp, false},
    {"DOWN", kBtnDown, false},
    {"LEFT", kBtnLeft, false},
    {"RIGHT", kBtnRight, false},
    {"CENTER", kBtnCenter, false},
};

TFT_eSPI tft = TFT_eSPI();
SPIClass sdSpi(FSPI);
HardwareSerial peerSerial(1);

bool g_displayReady = false;
bool g_sdReady = false;
bool g_peerSeen = false;
bool g_uiDirty = true;
String g_sdStatus = "Mounting microSD...";
String g_peerStatus = "Waiting for M5StickC";
String g_lastButton = "None";
String g_lastPeerLine = "-";
uint64_t g_sdSizeMb = 0;
uint32_t g_lastSdTryMs = 0;
uint32_t g_lastHeartbeatMs = 0;
uint32_t g_lastPeerSeenMs = 0;
uint32_t g_lastUiMs = 0;

UiMode g_uiMode = UiMode::Browser;
FsEntry g_browserEntries[kMaxBrowserEntries];
size_t g_browserEntryCount = 0;
size_t g_browserCursor = 0;
size_t g_browserScroll = 0;
String g_browserPath = "/";

String g_viewerPath = "/";
String g_viewerTitle = "";
String g_viewerStatus = "Select a readable file";
String g_viewerLines[kViewerLinesPerPage];
size_t g_viewerLineCount = 0;
size_t g_viewerPage = 0;
bool g_viewerHasNext = false;

char g_peerBuffer[128];
size_t g_peerLen = 0;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

String parentPathOf(const String& path) {
  if (path.length() <= 1) return "/";
  const int slash = path.lastIndexOf('/');
  if (slash <= 0) return "/";
  return path.substring(0, slash);
}

String joinPath(const String& base, const String& name) {
  if (base == "/") return "/" + name;
  return base + "/" + name;
}

bool hasReadableExtension(const String& path) {
  String lower = path;
  lower.toLowerCase();
  const char* exts[] = {
      ".txt", ".log", ".md",  ".ini", ".cfg", ".json", ".csv",
      ".h",   ".hpp", ".c",   ".cpp", ".py",  ".js",   ".css",
      ".htm", ".html",".xml", ".yaml",".yml", ".srt",
  };
  for (const char* ext : exts) {
    if (lower.endsWith(ext)) return true;
  }
  return false;
}

bool fileLooksText(File& file) {
  uint8_t probe[96];
  const size_t n = file.read(probe, sizeof(probe));
  file.seek(0);
  if (n == 0) return true;

  size_t bad = 0;
  for (size_t i = 0; i < n; ++i) {
    const uint8_t ch = probe[i];
    const bool ok = ch == '\n' || ch == '\r' || ch == '\t' || (ch >= 32 && ch < 127) || ch >= 160;
    if (!ok) ++bad;
  }
  return bad <= 4;
}

void enableBacklight() {
  pinMode(kDisplayBl, OUTPUT);
  digitalWrite(kDisplayBl, HIGH);
}

void setupButtons() {
  for (auto& button : g_buttons) {
    pinMode(button.pin, INPUT_PULLUP);
    button.pressed = readPressed(button.pin);
  }
}

void markUiDirty() {
  g_uiDirty = true;
}

void loadBrowserDirectory(const String& path) {
  g_browserPath = path.length() ? path : "/";
  g_browserEntryCount = 0;
  g_browserCursor = 0;
  g_browserScroll = 0;

  if (g_browserPath != "/" && g_browserEntryCount < kMaxBrowserEntries) {
    g_browserEntries[g_browserEntryCount++] = {"..", true, true};
  }

  File dir = SD.open(g_browserPath.c_str());
  if (!dir || !dir.isDirectory()) {
    g_sdStatus = "Cannot open dir";
    if (dir) dir.close();
    markUiDirty();
    return;
  }

  while (g_browserEntryCount < kMaxBrowserEntries) {
    File entry = dir.openNextFile();
    if (!entry) break;
    String name = entry.name();
    const int slash = name.lastIndexOf('/');
    if (slash >= 0) name = name.substring(slash + 1);
    if (name.length() == 0) {
      entry.close();
      continue;
    }
    g_browserEntries[g_browserEntryCount++] = {name, entry.isDirectory(), false};
    entry.close();
  }
  dir.close();

  g_uiMode = UiMode::Browser;
  g_sdStatus = (g_browserEntryCount == 0) ? "Mounted, dir empty" : (String("Path: ") + g_browserPath);
  markUiDirty();
}

bool loadViewerPage(const String& path, size_t page) {
  for (auto& line : g_viewerLines) line = "";
  g_viewerLineCount = 0;
  g_viewerHasNext = false;

  File file = SD.open(path.c_str(), FILE_READ);
  if (!file || file.isDirectory()) {
    g_viewerStatus = "Cannot open file";
    if (file) file.close();
    markUiDirty();
    return false;
  }

  const size_t startLine = page * kViewerLinesPerPage;
  size_t lineIndex = 0;
  String current = "";

  while (file.available()) {
    const char ch = static_cast<char>(file.read());
    if (ch == '\r') continue;
    if (ch == '\n') {
      if (lineIndex >= startLine && g_viewerLineCount < kViewerLinesPerPage) {
        g_viewerLines[g_viewerLineCount++] = current;
      }
      ++lineIndex;
      current = "";
      if (g_viewerLineCount >= kViewerLinesPerPage) break;
      continue;
    }
    if (lineIndex >= startLine && current.length() < 48) {
      current += ch;
    }
  }

  if ((current.length() > 0 || lineIndex == 0) && g_viewerLineCount < kViewerLinesPerPage && lineIndex >= startLine) {
    g_viewerLines[g_viewerLineCount++] = current;
  }

  if (file.available()) g_viewerHasNext = true;
  file.close();

  if (g_viewerLineCount == 0 && page > 0) {
    g_viewerStatus = "End of file";
    markUiDirty();
    return false;
  }

  g_uiMode = UiMode::Viewer;
  g_viewerPage = page;
  g_viewerStatus = String("Page ") + (g_viewerPage + 1);
  markUiDirty();
  return true;
}

void openReadableFile(const String& path) {
  File file = SD.open(path.c_str(), FILE_READ);
  if (!file || file.isDirectory()) {
    g_sdStatus = "Cannot open file";
    if (file) file.close();
    markUiDirty();
    return;
  }

  const bool extReadable = hasReadableExtension(path);
  const bool contentReadable = fileLooksText(file);
  const uint32_t fileSize = file.size();
  file.close();

  if (!extReadable && !contentReadable) {
    g_sdStatus = "Unsupported binary file";
    markUiDirty();
    return;
  }

  g_viewerPath = path;
  g_viewerTitle = path;
  const int slash = g_viewerTitle.lastIndexOf('/');
  if (slash >= 0) g_viewerTitle = g_viewerTitle.substring(slash + 1);
  g_sdStatus = String("Open: ") + g_viewerTitle + " (" + fileSize + "B)";
  loadViewerPage(g_viewerPath, 0);
}

void mountSd(bool force) {
  const uint32_t now = millis();
  if (!force && g_sdReady) return;
  if (!force && now - g_lastSdTryMs < kSdRetryMs) return;

  g_lastSdTryMs = now;
  g_sdReady = false;
  g_sdStatus = "Mounting microSD...";
  SD.end();
  sdSpi.end();
  sdSpi.begin(kSdSck, kSdMiso, kSdMosi, kSdCs);

  if (!SD.begin(kSdCs, sdSpi)) {
    g_sdStatus = "Check card or wiring";
    markUiDirty();
    return;
  }
  if (SD.cardType() == CARD_NONE) {
    g_sdStatus = "No card present";
    SD.end();
    markUiDirty();
    return;
  }

  g_sdReady = true;
  g_sdSizeMb = SD.cardSize() / (1024ULL * 1024ULL);
  loadBrowserDirectory("/");
}

void moveCursor(int delta) {
  if (g_browserEntryCount == 0) return;
  const size_t count = g_browserEntryCount;
  if (delta > 0) {
    g_browserCursor = (g_browserCursor + 1) % count;
  } else {
    g_browserCursor = (g_browserCursor + count - 1) % count;
  }
  markUiDirty();
}

void handleOpenSelection() {
  if (!g_sdReady) {
    mountSd(true);
    return;
  }

  if (g_uiMode == UiMode::Viewer) {
    g_uiMode = UiMode::Browser;
    markUiDirty();
    return;
  }

  if (g_browserEntryCount == 0) return;
  const auto& entry = g_browserEntries[g_browserCursor];
  if (entry.isParent) {
    loadBrowserDirectory(parentPathOf(g_browserPath));
    return;
  }

  const String fullPath = joinPath(g_browserPath, entry.name);
  if (entry.isDir) {
    loadBrowserDirectory(fullPath);
  } else {
    openReadableFile(fullPath);
  }
}

void handleBackAction() {
  if (!g_sdReady) return;
  if (g_uiMode == UiMode::Viewer) {
    g_uiMode = UiMode::Browser;
    markUiDirty();
    return;
  }
  if (g_browserPath != "/") {
    loadBrowserDirectory(parentPathOf(g_browserPath));
  }
}

void handleCommand(const String& cmd) {
  if (cmd.length() == 0) return;
  g_lastPeerLine = cmd;

  if (cmd == "SYNC" || cmd == "REFRESH") {
    markUiDirty();
    return;
  }
  if (cmd == "REMOUNT") {
    mountSd(true);
    return;
  }
  if (cmd == "HOME") {
    if (g_sdReady) loadBrowserDirectory("/");
    return;
  }
  if (!g_sdReady) {
    if (cmd == "OPEN" || cmd == "CENTER" || cmd == "RIGHT") mountSd(true);
    return;
  }

  if (g_uiMode == UiMode::Browser) {
    if (cmd == "UP" || cmd == "PREV") {
      moveCursor(-1);
    } else if (cmd == "DOWN" || cmd == "NEXT") {
      moveCursor(+1);
    } else if (cmd == "LEFT" || cmd == "BACK") {
      handleBackAction();
    } else if (cmd == "RIGHT" || cmd == "CENTER" || cmd == "OPEN") {
      handleOpenSelection();
    }
    return;
  }

  if (cmd == "UP" || cmd == "PREV") {
    if (g_viewerPage > 0) loadViewerPage(g_viewerPath, g_viewerPage - 1);
  } else if (cmd == "DOWN" || cmd == "NEXT" || cmd == "RIGHT") {
    if (g_viewerHasNext) loadViewerPage(g_viewerPath, g_viewerPage + 1);
  } else if (cmd == "LEFT" || cmd == "BACK" || cmd == "CENTER" || cmd == "OPEN") {
    handleBackAction();
  }
}

void buildUiFrame(String& title, String& sub, String& status, String lines[kVisibleRows], String& footer) {
  for (size_t i = 0; i < kVisibleRows; ++i) lines[i] = "";

  title = "ESP32-S3 SD remote";
  if (!g_sdReady) {
    sub = "microSD not mounted";
    status = g_sdStatus;
    lines[0] = "CENTER/RIGHT: remount";
    lines[1] = "M5 BtnA: remount";
    lines[2] = "UART TX47 RX48";
    footer = "Wait SD or press OPEN";
    return;
  }

  if (g_uiMode == UiMode::Browser) {
    sub = g_browserPath;
    status = g_sdStatus;
    if (g_browserCursor < g_browserScroll) g_browserScroll = g_browserCursor;
    if (g_browserCursor >= g_browserScroll + kVisibleRows) {
      g_browserScroll = g_browserCursor - kVisibleRows + 1;
    }
    const size_t end = (g_browserScroll + kVisibleRows < g_browserEntryCount) ? (g_browserScroll + kVisibleRows)
                                                                               : g_browserEntryCount;
    for (size_t i = g_browserScroll; i < end; ++i) {
      String label = g_browserEntries[i].name;
      if (g_browserEntries[i].isDir && !g_browserEntries[i].isParent) label += "/";
      if (label.length() > 28) label = label.substring(0, 25) + "...";
      lines[i - g_browserScroll] = String(i == g_browserCursor ? "> " : "  ") + label;
    }
    footer = "UP/DN move  RT/CT open LT back";
    return;
  }

  title = g_viewerTitle;
  sub = String("Page ") + (g_viewerPage + 1);
  status = g_viewerStatus;
  for (size_t i = 0; i < g_viewerLineCount && i < kVisibleRows; ++i) {
    lines[i] = g_viewerLines[i];
  }
  footer = "UP/DN page  LT/CT back";
}

void sendUiSnapshot() {
  String title;
  String sub;
  String status;
  String footer;
  String lines[kVisibleRows];
  buildUiFrame(title, sub, status, lines, footer);

  peerSerial.println("UI_BEGIN");
  peerSerial.printf("MODE:%s\n", g_uiMode == UiMode::Browser ? "BROWSER" : "VIEWER");
  peerSerial.printf("TITLE:%s\n", title.c_str());
  peerSerial.printf("SUB:%s\n", sub.c_str());
  peerSerial.printf("STATUS:%s\n", status.c_str());
  for (size_t i = 0; i < kVisibleRows; ++i) {
    peerSerial.printf("LINE:%s\n", lines[i].c_str());
  }
  peerSerial.printf("FOOTER:%s\n", footer.c_str());
  peerSerial.println("UI_END");
  g_uiDirty = false;
}

void drawLocalUi() {
  String title;
  String sub;
  String status;
  String footer;
  String lines[kVisibleRows];
  buildUiFrame(title, sub, status, lines, footer);

  tft.fillScreen(kBg);
  tft.setTextColor(kAccent, kBg);
  tft.setCursor(4, 4);
  tft.print(title);
  tft.setTextColor(g_sdReady ? kOk : kWarn, kBg);
  tft.setCursor(4, 20);
  tft.print(sub);
  tft.setTextColor(kFg, kBg);
  tft.setCursor(4, 36);
  tft.print(status);
  for (size_t i = 0; i < kVisibleRows; ++i) {
    tft.setCursor(4, 56 + static_cast<int>(i) * 14);
    tft.print(lines[i]);
  }

  tft.drawFastHLine(0, 286, 320, kAccent);
  tft.setTextColor(kAccent, kBg);
  tft.setCursor(4, 292);
  tft.print(footer);
}

void consumePeerLine(const char* line) {
  if (!line[0]) return;
  g_peerSeen = true;
  g_lastPeerSeenMs = millis();
  g_peerStatus = "M5StickC online";
  Serial.printf("[PEER] %s\n", line);

  String cmd = line;
  if (cmd.startsWith("CMD ")) {
    cmd.remove(0, 4);
    handleCommand(cmd);
  }
}

void pollPeer() {
  while (peerSerial.available()) {
    const int ch = peerSerial.read();
    if (ch < 0) break;
    if (ch == '\r') continue;
    if (ch == '\n') {
      g_peerBuffer[g_peerLen] = '\0';
      consumePeerLine(g_peerBuffer);
      g_peerLen = 0;
      continue;
    }
    if (g_peerLen + 1 < sizeof(g_peerBuffer)) {
      g_peerBuffer[g_peerLen++] = static_cast<char>(ch);
    } else {
      g_peerLen = 0;
    }
  }

  if (g_peerSeen && millis() - g_lastPeerSeenMs > 5000) {
    g_peerSeen = false;
    g_peerStatus = "M5StickC timeout";
    markUiDirty();
  }

  if (millis() - g_lastHeartbeatMs >= kHeartbeatMs) {
    g_lastHeartbeatMs = millis();
    peerSerial.printf("STAT BTN=%s SD=%s PATH=%s\n", g_lastButton.c_str(), g_sdReady ? "OK" : "MISS",
                      g_browserPath.c_str());
    if (g_uiDirty) sendUiSnapshot();
  }
}

bool pollButtons() {
  bool changed = false;
  for (auto& button : g_buttons) {
    const bool nowPressed = readPressed(button.pin);
    if (nowPressed == button.pressed) continue;
    button.pressed = nowPressed;
    changed = true;
    if (!nowPressed) continue;

    g_lastButton = button.name;
    if (button.pin == kBtnUp) handleCommand("UP");
    else if (button.pin == kBtnDown) handleCommand("DOWN");
    else if (button.pin == kBtnLeft) handleCommand("LEFT");
    else if (button.pin == kBtnRight) handleCommand("RIGHT");
    else if (button.pin == kBtnCenter) handleCommand("CENTER");
  }
  return changed;
}

void logWiring() {
  Serial.println();
  Serial.println("idk-s3-test");
  Serial.println("ESP32-S3 hosts 5-way + microSD and streams UI to M5StickC Plus2 over UART.");
  Serial.printf("SD MOSI=%d SCK=%d MISO=%d CS=%d\n", kSdMosi, kSdSck, kSdMiso, kSdCs);
  Serial.printf("5WAY UP=%d DOWN=%d LEFT=%d RIGHT=%d CENTER=%d\n", kBtnUp, kBtnDown, kBtnLeft, kBtnRight,
                kBtnCenter);
  Serial.printf("UART TX=%d RX=%d baud=%lu\n", kPeerTx, kPeerRx, static_cast<unsigned long>(kPeerBaud));
  Serial.println("Commands from peer: CMD UP/DOWN/LEFT/RIGHT/CENTER/OPEN/BACK/HOME/REMOUNT/SYNC");
  Serial.println();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  setupButtons();
  peerSerial.begin(kPeerBaud, SERIAL_8N1, kPeerRx, kPeerTx);

  enableBacklight();
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(kBg);
  g_displayReady = true;

  mountSd(true);
  drawLocalUi();
  sendUiSnapshot();
  logWiring();
}

void loop() {
  const bool changed = pollButtons();
  pollPeer();
  if (!g_sdReady) mountSd(false);
  if (changed || g_uiDirty || millis() - g_lastUiMs >= 250) {
    g_lastUiMs = millis();
    drawLocalUi();
    if (g_uiDirty) sendUiSnapshot();
  }
  delay(15);
}

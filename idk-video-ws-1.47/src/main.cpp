#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <SD_MMC.h>

#include <FS.h>

#include <SPI.h>
#include <TJpg_Decoder.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "../../shared/idk_vi_font.h"
#include "../../shared/idk_vn_text.h"


class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;
public:
  LGFX(void) {
    auto bus_cfg = _bus_instance.config();
    bus_cfg.spi_host   = SPI2_HOST;
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
    bus_cfg.spi_mode   = 0;
    bus_cfg.freq_write = 80000000;
    bus_cfg.freq_read  = 16000000;
    bus_cfg.pin_sclk   = 40;
    bus_cfg.pin_mosi   = 45;
    bus_cfg.pin_miso   = -1;
    bus_cfg.pin_dc     = 41;
    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);

    auto panel_cfg = _panel_instance.config();
    panel_cfg.pin_cs           = 42;
    panel_cfg.pin_rst          = 39;
    panel_cfg.pin_busy         = -1;
    panel_cfg.panel_width      = 172;
    panel_cfg.panel_height     = 320;
    panel_cfg.offset_x         = 34;
    panel_cfg.offset_y         = 0;
    panel_cfg.offset_rotation  = 0;
    panel_cfg.dummy_read_bits  = 8;
    panel_cfg.readable         = false;
    panel_cfg.invert           = true;
    panel_cfg.rgb_order        = false;
    _panel_instance.config(panel_cfg);

    auto light_cfg = _light_instance.config();
    light_cfg.pin_bl = 46;
    light_cfg.invert = false;
    light_cfg.freq   = 12000;
    light_cfg.pwm_channel = 7;
    _light_instance.config(light_cfg);
    _panel_instance.setLight(&_light_instance);

    setPanel(&_panel_instance);
  }
};
LGFX display_lcd;

class Button {
  int pin;
  bool state;
  bool lastState;
  uint32_t pressTime;
  uint32_t lastDebounce;
  bool _wasPressed;
public:
  Button(int p) : pin(p), state(false), lastState(false), pressTime(0), lastDebounce(0), _wasPressed(false) {
    pinMode(pin, INPUT_PULLUP);
  }
  void update() {
    _wasPressed = false;
    bool reading = (digitalRead(pin) == LOW);
    if (reading != lastState) lastDebounce = millis();
    if ((millis() - lastDebounce) > 50) {
      if (reading != state) {
        state = reading;
        if (state) { pressTime = millis(); _wasPressed = true; }
      }
    }
    lastState = reading;
  }
  bool isPressed() { return state; }
  bool wasPressed() { return _wasPressed; }
  bool pressedFor(uint32_t ms) { return state && (millis() - pressTime > ms); }
};

Button btnUp(0);
Button btnDown(2);
Button btnLeft(3);
Button btnRight(4);
Button btnPress(5);

constexpr int kPinSD_CLK    = 14;
constexpr int kPinSD_CMD    = 15;
constexpr int kPinSD_D0     = 16;
constexpr int kPinSD_D1     = 18;
constexpr int kPinSD_D2     = 17;
constexpr int kPinSD_D3     = 21;

static const uint32_t kFrameDelayMs = 33; // 10 fps default
static const size_t kFrameBufSize = 80 * 1024;
static const int kMaxFiles = 128;
static const int kSkipSeconds = 10;
static const int kMaxSubtitles = 600;
static const uint16_t kSubtitlePort = 4210;
static const char *kApSsid = "Diddy Heil Epstein";
static const char *kApPass = "TruongLangQuan";
static const uint32_t kTenstarTimeoutMs = 3000;
static const uint16_t kFgColor = 0x07E0;    // Launcher-like green
static const uint16_t kAccentColor = 0xF800; // Launcher-like red
static const uint16_t kBgColor = 0x0000;    // black
static const int kHeaderH = 20;
static const int kFooterH = 16;


enum class AppState {
  FILE_SELECT,
  PLAYING,
};

struct FileEntry {
  String name;
  String path;
  bool isDir = false;
};

struct SubtitleCue {
  uint32_t start_ms = 0;
  uint32_t end_ms = 0;
  String text;
};

static uint8_t *g_frame_buf = nullptr;
static size_t g_frame_len = 0;
static File g_file;
static bool g_paused = false;
static float g_avg_frame_ms = kFrameDelayMs;
static uint32_t g_last_frame_ms = 0;
static uint32_t g_play_time_ms = 0;

static AppState g_state = AppState::FILE_SELECT;
static FileEntry g_entries[kMaxFiles];
static int g_entry_count = 0;
static int g_entry_index = 0;
static String g_status = "";
static String g_current_path = "";
static String g_dir = "/";
static fs::FS *g_fs = nullptr;
static bool g_sd_ready = false;
static SubtitleCue g_subs[kMaxSubtitles];
static int g_sub_count = 0;
static int g_sub_index = 0;
static bool g_has_subs = false;
static String g_last_sub_sent = "";
static String g_current_sub_text = "";
static uint32_t g_last_tenstar_ms = 0;
static bool g_tenstar_connected = false;
static WiFiUDP g_udp;
static bool g_vi_font_loaded = false;

static bool loadSubtitlesForVideo(const String &videoPath);
static void updateSubtitleForTime(uint32_t ms);

static String trimText(const String &text, int maxLen) {
  if (text.length() <= maxLen) return text;
  if (maxLen <= 3) return text.substring(0, maxLen);
  return text.substring(0, maxLen - 3) + "...";
}

static int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static bool isVideoFile(const String &name) {
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".mjpg") || lower.endsWith(".mjpeg");
}

static bool ensureSdReady() {
  if (g_sd_ready) return true;
  SD_MMC.end();
  delay(10);
  if (!SD_MMC.setPins(kPinSD_CLK, kPinSD_CMD, kPinSD_D0, kPinSD_D1, kPinSD_D2, kPinSD_D3)) {
    return false;
  }
  if (!SD_MMC.begin("/sdcard", false, false, 40000000)) {
    if (!SD_MMC.begin("/sdcard", true, false, 40000000)) {
      return false;
    }
  }
  g_sd_ready = true;
  return g_sd_ready;
}
static String joinPath(const String &base, const String &name) {
  if (name.startsWith("/")) return name;
  if (base.length() == 0 || base == "/") return String("/") + name;
  if (base.endsWith("/")) return base + name;
  return base + "/" + name;
}

static String parentPath(const String &path) {
  if (path.length() == 0 || path == "/") return "/";
  int slash = path.lastIndexOf('/');
  if (slash <= 0) return "/";
  return path.substring(0, slash);
}

static void drawBattery() {
  int w = display_lcd.width();
  int x = w - 42;
  int y = 6;
  int bw = 32;
  int bh = 10;
  display_lcd.drawRoundRect(x, y, bw, bh, 2, kFgColor);
  int raw_adc = analogRead(1);
  float voltage = (raw_adc > 0) ? (raw_adc / 4095.0f) * 3.3f * 2.0f : 3.85f;
  if (voltage > 4.2f) voltage = 4.2f;
  if (voltage < 3.2f) voltage = 3.2f;
  int level = (int)(((voltage - 3.2f) / 1.0f) * 100.0f);
  level = clampInt(level, 0, 100);
  int fill = (bw - 4) * level / 100;
  if (fill > 0) {
    display_lcd.fillRoundRect(x + 2, y + 2, fill, bh - 4, 2, kFgColor);
  }
}
static void drawFrame() {
  int w = display_lcd.width();
  int h = display_lcd.height();
  display_lcd.fillScreen(kBgColor);
  display_lcd.drawRoundRect(3, 3, w - 6, h - 6, 5, kFgColor);
  display_lcd.drawLine(5, kHeaderH, w - 6, kHeaderH, kFgColor);
  display_lcd.drawLine(5, h - kFooterH - 2, w - 6, h - kFooterH - 2, kFgColor);
  drawBattery();
}

static void drawFooter(const char *left, const char *center, const char *right) {
  int w = display_lcd.width();
  int h = display_lcd.height();
  int y = h - kFooterH;
  display_lcd.drawRoundRect(5, y - 1, w - 10, kFooterH - 2, 3, kFgColor);
  display_lcd.setTextSize(1);
  display_lcd.setTextColor(kFgColor, kBgColor);
  display_lcd.setCursor(10, y + 2);
  display_lcd.print(left);
  display_lcd.setCursor(w / 2 - 18, y + 2);
  display_lcd.print(center);
  display_lcd.setCursor(w - 70, y + 2);
  display_lcd.print(right);
}

static void drawFileMenu() {
  drawFrame();
  display_lcd.setTextFont(1);
  display_lcd.setTextSize(1);
  display_lcd.setTextColor(kFgColor, kBgColor);
  display_lcd.setCursor(8, 6);
  display_lcd.print("IDK-VIDEO");

  String pathLabel = trimText(g_dir, 26);
  display_lcd.setTextColor(kAccentColor, kBgColor);
  display_lcd.setCursor(8, kHeaderH + 4);
  display_lcd.printf("SD:%s", pathLabel.c_str());

  if (g_entry_count <= 0) {
    display_lcd.setTextColor(kFgColor, kBgColor);
    display_lcd.setCursor(10, 40);
    display_lcd.print("No entries");
  } else {
    const int visible = 6;
    int start = g_entry_index - visible / 2;
    if (start < 0) start = 0;
    int end = start + visible;
    if (end > g_entry_count) {
      end = g_entry_count;
      start = end - visible;
      if (start < 0) start = 0;
    }
    int y = 36;
    for (int i = start; i < end; ++i) {
      bool selected = (i == g_entry_index);
      uint16_t fg = g_entries[i].isDir ? kAccentColor : kFgColor;
      if (selected) {
        display_lcd.fillRoundRect(8, y - 1, display_lcd.width() - 16, 11, 3, kFgColor);
        display_lcd.setTextColor(kBgColor, kFgColor);
      } else {
        display_lcd.setTextColor(fg, kBgColor);
      }
      String label = g_entries[i].isDir ? "[D] " : "    ";
      label += trimText(g_entries[i].name, 24);
      display_lcd.setCursor(10, y);
      display_lcd.printf("%c %s", selected ? '>' : ' ', label.c_str());
      y += 12;
    }
  }

  display_lcd.setTextColor(kAccentColor, kBgColor);
  display_lcd.setCursor(8, display_lcd.height() - kFooterH - 12);
  display_lcd.print(trimText(g_status, 28));
  drawFooter("L:UPDIR", "OK:PLAY", "D/R:NEXT");
}

static void drawPlaybackOverlay() {
  if (!g_paused) return;
  int w = display_lcd.width();
  int h = display_lcd.height();
  display_lcd.fillRoundRect(40, h / 2 - 12, w - 80, 24, 4, kBgColor);
  display_lcd.drawRoundRect(40, h / 2 - 12, w - 80, 24, 4, kAccentColor);
  display_lcd.setTextColor(kAccentColor, kBgColor);
  display_lcd.setTextSize(1);
  display_lcd.setCursor(w / 2 - 18, h / 2 - 4);
  display_lcd.print("PAUSED");
}

static bool scanDir(const String &dirPath) {
  g_entry_count = 0;
  g_entry_index = 0;
  bool truncated = false;

  if (!ensureSdReady()) {
    g_status = "SD mount failed";
    return false;
  }

  g_fs = &SD_MMC;
  File root = g_fs->open(dirPath.c_str());
  if (!root) {
    g_status = "Open failed";
    return false;
  }
  if (!root.isDirectory()) {
    root.close();
    g_status = "Not a folder";
    return false;
  }

  File f = root.openNextFile();
  while (f && g_entry_count < kMaxFiles) {
    String name = f.name();
    int slash = name.lastIndexOf('/');
    if (slash >= 0) {
      name = name.substring(slash + 1);
    }
    if (name.length() == 0) {
      f = root.openNextFile();
      continue;
    }
    if (f.isDirectory()) {
      if (g_entry_count < kMaxFiles) {
        g_entries[g_entry_count].name = name;
        g_entries[g_entry_count].path = joinPath(dirPath, name);
        g_entries[g_entry_count].isDir = true;
        g_entry_count++;
      } else {
        truncated = true;
      }
    } else if (isVideoFile(name)) {
      if (g_entry_count < kMaxFiles) {
        g_entries[g_entry_count].name = name;
        g_entries[g_entry_count].path = joinPath(dirPath, name);
        g_entries[g_entry_count].isDir = false;
        g_entry_count++;
      } else {
        truncated = true;
      }
    }
    f = root.openNextFile();
  }
  root.close();

  if (g_entry_count == 0) {
    g_status = "No entries";
  } else if (truncated) {
    g_status = String("List truncated (") + kMaxFiles + ")";
  } else {
    g_status = String("Found ") + g_entry_count;
  }
  return true;
}

static bool openCurrentVideo() {
  if (!g_fs || g_entry_count <= 0) return false;
  if (g_entries[g_entry_index].isDir) return false;
  if (g_file) g_file.close();
  g_current_path = g_entries[g_entry_index].path;
  g_file = g_fs->open(g_current_path.c_str(), FILE_READ);
  if (!g_file) {
    g_status = "Open failed";
    return false;
  }
  g_play_time_ms = 0;
  g_last_frame_ms = 0;
  g_avg_frame_ms = kFrameDelayMs;
  g_paused = false;
  g_last_sub_sent = "";
  loadSubtitlesForVideo(g_current_path);
  updateSubtitleForTime(0);
  return true;
}

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  if (y >= display_lcd.height()) return false;
  display_lcd.pushImage(x, y, w, h, bitmap);
  return true;
}

static bool readNextFrame(File &f, uint8_t *buf, size_t bufSize, size_t &outLen) {
  outLen = 0;
  // 1. Find 0xFF 0xD8
  while (f.available()) {
    int c = f.read();
    if (c == 0xFF) {
      int c2 = f.read();
      if (c2 == 0xD8) {
        buf[0] = 0xFF;
        buf[1] = 0xD8;
        outLen = 2;
        break;
      } else {
        f.seek(f.position() - 1);
      }
    }
  }

  if (outLen == 0) return false;

  // 2. Read in chunks and find 0xFF 0xD9
  while (f.available() && outLen < bufSize) {
    size_t to_read = bufSize - outLen;
    if (to_read > 4096) to_read = 4096;
    
    size_t actually_read = f.read(buf + outLen, to_read);
    if (actually_read == 0) break;

    size_t start_search = (outLen > 2) ? (outLen - 1) : 2;
    outLen += actually_read;

    for (size_t i = start_search; i < outLen - 1; i++) {
      if (buf[i] == 0xFF && buf[i+1] == 0xD9) {
        size_t frameLen = i + 2;
        size_t over_read = outLen - frameLen;
        if (over_read > 0) {
          f.seek(f.position() - over_read);
        }
        outLen = frameLen;
        return true;
      }
    }
  }
  return false;
}

static bool skipFrames(File &f, uint8_t *buf, size_t bufSize, int frameCount) {
  size_t len = 0;
  for (int i = 0; i < frameCount; ++i) {
    if (!readNextFrame(f, buf, bufSize, len)) {
      return false;
    }
  }
  return true;
}

static int framesForSeconds(float seconds) {
  if (g_avg_frame_ms <= 1.0f) return int(seconds * 1000.0f / kFrameDelayMs);
  return int(seconds * 1000.0f / g_avg_frame_ms);
}

static uint32_t parseTimeMs(const String &s) {
  int p1 = s.indexOf(':');
  int p2 = s.indexOf(':', p1 + 1);
  if (p1 < 0 || p2 < 0) return 0;
  int p3 = s.indexOf(',', p2 + 1);
  if (p3 < 0) p3 = s.indexOf('.', p2 + 1);
  int h = s.substring(0, p1).toInt();
  int m = s.substring(p1 + 1, p2).toInt();
  int sec = 0;
  int ms = 0;
  if (p3 >= 0) {
    sec = s.substring(p2 + 1, p3).toInt();
    ms = s.substring(p3 + 1).toInt();
  } else {
    sec = s.substring(p2 + 1).toInt();
  }
  return ((h * 3600UL + m * 60UL + sec) * 1000UL) + ms;
}

static bool parseSubtitleTimeLine(const String &line, uint32_t &start_ms, uint32_t &end_ms) {
  int arrow = line.indexOf("-->");
  if (arrow < 0) return false;
  String left = line.substring(0, arrow);
  String right = line.substring(arrow + 3);
  left.trim();
  right.trim();
  start_ms = parseTimeMs(left);
  end_ms = parseTimeMs(right);
  return end_ms >= start_ms;
}

static String subtitlePathForVideo(const String &videoPath) {
  int slash = videoPath.lastIndexOf('/');
  String dir = (slash >= 0) ? videoPath.substring(0, slash) : String("/");
  String name = (slash >= 0) ? videoPath.substring(slash + 1) : videoPath;
  int dot = name.lastIndexOf('.');
  String stem = (dot > 0) ? name.substring(0, dot) : name;
  const String targets[] = {stem + ".srt", stem + ".vi.srt", stem + ".vi-orig.srt", stem + ".en.srt"};

  for (const auto &target : targets) {
    String candidate = joinPath(dir, target);
    if (g_fs && g_fs->exists(candidate.c_str())) return candidate;
  }

  // Case-insensitive scan for .srt
  if (g_fs) {
    File root = g_fs->open(dir.c_str());
    if (root && root.isDirectory()) {
      File f = root.openNextFile();
      while (f) {
        if (!f.isDirectory()) {
          String fname = f.name();
          int s = fname.lastIndexOf('/');
          if (s >= 0) fname = fname.substring(s + 1);
          String lower = fname;
          lower.toLowerCase();
          for (const auto &target : targets) {
            String want = target;
            want.toLowerCase();
            if (lower == want) {
              root.close();
              return joinPath(dir, fname);
            }
          }
        }
        f = root.openNextFile();
      }
      root.close();
    }
  }
  return joinPath(dir, targets[0]);
}

static bool loadSubtitlesForVideo(const String &videoPath) {
  g_sub_count = 0;
  g_sub_index = 0;
  g_has_subs = false;
  String subPath = subtitlePathForVideo(videoPath);
  if (!g_fs) return false;
  File sf = g_fs->open(subPath.c_str(), FILE_READ);
  if (!sf) return false;

  String line;
  String text = "";
  uint32_t start_ms = 0;
  uint32_t end_ms = 0;
  bool inCue = false;
  while (sf.available()) {
    line = sf.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      if (inCue && g_sub_count < kMaxSubtitles) {
        g_subs[g_sub_count].start_ms = start_ms;
        g_subs[g_sub_count].end_ms = end_ms;
        g_subs[g_sub_count].text = text;
        g_sub_count++;
      }
      inCue = false;
      text = "";
      continue;
    }
    if (!inCue) {
      if (parseSubtitleTimeLine(line, start_ms, end_ms)) {
        inCue = true;
        text = "";
      }
      continue;
    }
    if (text.length() > 0) text += " ";
    text += line;
  }
  if (inCue && g_sub_count < kMaxSubtitles) {
    g_subs[g_sub_count].start_ms = start_ms;
    g_subs[g_sub_count].end_ms = end_ms;
    g_subs[g_sub_count].text = text;
    g_sub_count++;
  }
  sf.close();
  g_has_subs = g_sub_count > 0;
  return g_has_subs;
}

static void sendSubtitleIfChanged(const String &text) {
  if (text == g_last_sub_sent) return;
  g_last_sub_sent = text;
  g_current_sub_text = text;
  IPAddress broadcastIp(192, 168, 4, 255);
  g_udp.beginPacket(broadcastIp, kSubtitlePort);
  g_udp.print(text);
  g_udp.endPacket();
}

static void updateTenstarStatus() {
  int packet = g_udp.parsePacket();
  if (packet > 0) {
    String msg = "";
    while (g_udp.available()) {
      char c = static_cast<char>(g_udp.read());
      msg += c;
      if (msg.length() > 32) break;
    }
    if (msg.startsWith("HELLO")) {
      g_last_tenstar_ms = millis();
    }
  }
  uint32_t now = millis();
  g_tenstar_connected = (g_last_tenstar_ms > 0) && (now - g_last_tenstar_ms < kTenstarTimeoutMs);
}

static void drawSubtitleLocal(const String &text) {
  int w = display_lcd.width();
  int h = display_lcd.height();
  int box_h = 30;
  int y = h - box_h;
  display_lcd.fillRect(0, y, w, box_h, kBgColor);
  display_lcd.drawRect(0, y, w, box_h, kFgColor);
  if (text.isEmpty()) return;
  // Use idk_vn_text for proper Vietnamese rendering (handles Unicode/diacritics)
  idk_vn_text::drawWrapped(display_lcd, text, 4, y + 4, w - 8, box_h - 8, kFgColor, kBgColor, 1);
}

static void updateSubtitleForTime(uint32_t ms) {
  if (!g_has_subs) {
    if (g_last_sub_sent.length()) sendSubtitleIfChanged("");
    return;
  }
  if (g_sub_index >= g_sub_count) {
    if (g_last_sub_sent.length()) sendSubtitleIfChanged("");
    return;
  }
  if (ms < g_subs[g_sub_index].start_ms) {
    g_sub_index = 0;
  }
  while (g_sub_index < g_sub_count && ms > g_subs[g_sub_index].end_ms) {
    g_sub_index++;
  }
  String text = "";
  if (g_sub_index < g_sub_count) {
    const auto &cue = g_subs[g_sub_index];
    if (ms >= cue.start_ms && ms <= cue.end_ms) {
      text = cue.text;
    }
  }
  sendSubtitleIfChanged(text);
}

static void clearSubtitleState() {
  g_has_subs = false;
  g_sub_count = 0;
  g_sub_index = 0;
  g_current_sub_text = "";
  sendSubtitleIfChanged("");
}

void setup() {
  Serial.begin(115200);
  display_lcd.init();
  display_lcd.setRotation(1);
  display_lcd.setBrightness(180);
  g_vi_font_loaded = idk_vi_font::load(display_lcd);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApSsid, kApPass);
  g_udp.begin(kSubtitlePort);

  g_frame_buf = static_cast<uint8_t *>(ps_malloc(kFrameBufSize));
  if (!g_frame_buf) {
    g_frame_buf = static_cast<uint8_t *>(malloc(kFrameBufSize));
  }
  if (!g_frame_buf) {
    display_lcd.fillScreen(TFT_BLACK);
    display_lcd.setTextColor(TFT_RED, TFT_BLACK);
    display_lcd.setCursor(4, 4);
    display_lcd.print("Frame buffer alloc failed");
    return;
  }

  TJpgDec.setCallback(tft_output);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setJpgScale(1);

  g_state = AppState::FILE_SELECT;
  g_status = "Scanning...";
  scanDir(g_dir);
  drawFileMenu();
}
void loop() {
  btnUp.update(); btnDown.update(); btnLeft.update(); btnRight.update(); btnPress.update();
  updateTenstarStatus();

  if (g_state == AppState::FILE_SELECT) {
    if (btnLeft.pressedFor(700)) {
      String parent = parentPath(g_dir);
      if (parent != g_dir) {
        g_dir = parent;
        g_status = "Scanning...";
        scanDir(g_dir);
        drawFileMenu();
        delay(200);
        return;
      }
    }
    if ((btnRight.wasPressed() || btnDown.wasPressed()) && g_entry_count > 0) {
      g_entry_index = (g_entry_index + 1) % g_entry_count;
      drawFileMenu();
    }
    if ((btnLeft.wasPressed() || btnUp.wasPressed()) && g_entry_count > 0) {
      g_entry_index = (g_entry_index + g_entry_count - 1) % g_entry_count;
      drawFileMenu();
    }
    if (btnPress.wasPressed() && g_entry_count > 0) {
      if (g_entries[g_entry_index].isDir) {
        g_dir = g_entries[g_entry_index].path;
        g_status = "Scanning...";
        scanDir(g_dir);
        drawFileMenu();
        delay(200);
        return;
      }
      if (openCurrentVideo()) {
        g_state = AppState::PLAYING;
      } else {
        drawFileMenu();
      }
    }
    delay(10);
    return;
  }

  if (g_state == AppState::PLAYING) {
    if (btnLeft.pressedFor(700)) {
      if (g_file) g_file.close();
      g_paused = false;
      clearSubtitleState();
      g_state = AppState::FILE_SELECT;
      drawFileMenu();
      delay(10);
      return;
    }
    if (btnPress.wasPressed()) {
      g_paused = !g_paused;
      drawPlaybackOverlay();
    }
    if (btnRight.wasPressed()) {
      const uint32_t target_ms = g_play_time_ms + (kSkipSeconds * 1000UL);
      const int framesToSkip = framesForSeconds(kSkipSeconds);
      if (!skipFrames(g_file, g_frame_buf, kFrameBufSize, framesToSkip)) {
        g_status = "Skip end";
        g_file.seek(0);
        g_play_time_ms = 0;
        g_sub_index = 0;
        updateSubtitleForTime(0);
      } else {
        g_play_time_ms = target_ms;
        g_last_frame_ms = millis();
        updateSubtitleForTime(g_play_time_ms);
      }
    }

    if (!g_file) {
      g_status = "File closed";
      g_state = AppState::FILE_SELECT;
      drawFileMenu();
      delay(10);
      return;
    }

    if (g_paused) {
      if (!g_tenstar_connected) {
        drawSubtitleLocal(g_current_sub_text);
      }
      delay(20);
      return;
    }

    if (!readNextFrame(g_file, g_frame_buf, kFrameBufSize, g_frame_len)) {
      g_file.seek(0);
      g_play_time_ms = 0;
      g_sub_index = 0;
      updateSubtitleForTime(0);
      return;
    }

    updateSubtitleForTime(g_play_time_ms);
    uint32_t frame_start_ms = millis();
    display_lcd.startWrite(); TJpgDec.drawJpg(0, 0, g_frame_buf, g_frame_len); display_lcd.endWrite();
    if (!g_tenstar_connected) {
      drawSubtitleLocal(g_current_sub_text);
    }
    uint32_t draw_ms = millis() - frame_start_ms;
    if (draw_ms < kFrameDelayMs) {
      delay(kFrameDelayMs - draw_ms);
    }
    uint32_t frame_ms = millis() - frame_start_ms;
    g_avg_frame_ms = g_avg_frame_ms * 0.9f + frame_ms * 0.1f;
    g_last_frame_ms = millis();
    g_play_time_ms += frame_ms;
    return;
  }
}

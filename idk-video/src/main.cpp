#include <M5Unified.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "../../shared/idk_vi_font.h"

static const uint32_t kFrameDelayMs = 100; // 10 fps default
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

// SD pins aligned with /home/truonglangquan/idk-code/idk-firmware-idk/boards/m5stack-cplus2/m5stack-cplus2.ini
static const int kSdCsPin = 14;
static const int kSdSckPin = 0;
static const int kSdMisoPin = 36;
static const int kSdMosiPin = 26;

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
  SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
  g_sd_ready = SD.begin(kSdCsPin, SPI);
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
  int w = M5.Display.width();
  int x = w - 42;
  int y = 6;
  int bw = 32;
  int bh = 10;
  M5.Display.drawRoundRect(x, y, bw, bh, 2, kFgColor);
  int level = M5.Power.getBatteryLevel();
  level = clampInt(level, 0, 100);
  int fill = (bw - 4) * level / 100;
  if (fill > 0) {
    M5.Display.fillRoundRect(x + 2, y + 2, fill, bh - 4, 2, kFgColor);
  }
}

static void drawFrame() {
  int w = M5.Display.width();
  int h = M5.Display.height();
  M5.Display.fillScreen(kBgColor);
  M5.Display.drawRoundRect(3, 3, w - 6, h - 6, 5, kFgColor);
  M5.Display.drawLine(5, kHeaderH, w - 6, kHeaderH, kFgColor);
  M5.Display.drawLine(5, h - kFooterH - 2, w - 6, h - kFooterH - 2, kFgColor);
  drawBattery();
}

static void drawFooter(const char *left, const char *center, const char *right) {
  int w = M5.Display.width();
  int h = M5.Display.height();
  int y = h - kFooterH;
  M5.Display.drawRoundRect(5, y - 1, w - 10, kFooterH - 2, 3, kFgColor);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kFgColor, kBgColor);
  M5.Display.setCursor(10, y + 2);
  M5.Display.print(left);
  M5.Display.setCursor(w / 2 - 18, y + 2);
  M5.Display.print(center);
  M5.Display.setCursor(w - 70, y + 2);
  M5.Display.print(right);
}

static void drawFileMenu() {
  drawFrame();
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kFgColor, kBgColor);
  M5.Display.setCursor(8, 6);
  M5.Display.print("IDK-VIDEO");

  String pathLabel = trimText(g_dir, 26);
  M5.Display.setTextColor(kAccentColor, kBgColor);
  M5.Display.setCursor(8, kHeaderH + 4);
  M5.Display.printf("SD:%s", pathLabel.c_str());

  if (g_entry_count <= 0) {
    M5.Display.setTextColor(kFgColor, kBgColor);
    M5.Display.setCursor(10, 40);
    M5.Display.print("No entries");
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
        M5.Display.fillRoundRect(8, y - 1, M5.Display.width() - 16, 11, 3, kFgColor);
        M5.Display.setTextColor(kBgColor, kFgColor);
      } else {
        M5.Display.setTextColor(fg, kBgColor);
      }
      String label = g_entries[i].isDir ? "[D] " : "    ";
      label += trimText(g_entries[i].name, 24);
      M5.Display.setCursor(10, y);
      M5.Display.printf("%c %s", selected ? '>' : ' ', label.c_str());
      y += 12;
    }
  }

  M5.Display.setTextColor(kAccentColor, kBgColor);
  M5.Display.setCursor(8, M5.Display.height() - kFooterH - 12);
  M5.Display.print(trimText(g_status, 28));
  drawFooter("PWR:UP", "A:OPEN", "B:NEXT");
}

static void drawPlaybackOverlay() {
  if (!g_paused) return;
  int w = M5.Display.width();
  int h = M5.Display.height();
  M5.Display.fillRoundRect(40, h / 2 - 12, w - 80, 24, 4, kBgColor);
  M5.Display.drawRoundRect(40, h / 2 - 12, w - 80, 24, 4, kAccentColor);
  M5.Display.setTextColor(kAccentColor, kBgColor);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(w / 2 - 18, h / 2 - 4);
  M5.Display.print("PAUSED");
}

static bool scanDir(const String &dirPath) {
  g_entry_count = 0;
  g_entry_index = 0;
  bool truncated = false;

  if (!ensureSdReady()) {
    g_status = "SD mount failed";
    return false;
  }

  g_fs = &SD;
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
  if (y >= M5.Display.height()) return false;
  M5.Display.pushImage(x, y, w, h, bitmap);
  return true;
}

static bool readNextFrame(File &f, uint8_t *buf, size_t bufSize, size_t &outLen) {
  outLen = 0;
  int prev = -1;
  int c = 0;
  while (f.available()) {
    c = f.read();
    if (prev == 0xFF && c == 0xD8) {
      buf[0] = 0xFF;
      buf[1] = 0xD8;
      outLen = 2;
      break;
    }
    prev = c;
  }
  if (outLen == 0) return false;

  prev = -1;
  while (f.available()) {
    c = f.read();
    if (outLen < bufSize) {
      buf[outLen++] = static_cast<uint8_t>(c);
    } else {
      return false;
    }
    if (prev == 0xFF && c == 0xD9) {
      return true;
    }
    prev = c;
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
  int w = M5.Display.width();
  int h = M5.Display.height();
  int box_h = 30;
  int y = h - box_h;
  M5.Display.fillRect(0, y, w, box_h, kBgColor);
  M5.Display.drawRect(0, y, w, box_h, kFgColor);
  if (text.isEmpty()) return;
  if (g_vi_font_loaded) {
    idk_vi_font::drawWrapped(M5.Display, text, 4, y + 4, w - 8, box_h - 8, kFgColor, kBgColor, 14);
  } else {
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(kFgColor, kBgColor);
    M5.Display.setCursor(4, y + 4);
    M5.Display.print(text);
  }
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
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3); // Left landscape
  M5.Display.setBrightness(180);
  g_vi_font_loaded = idk_vi_font::load(M5.Display);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApSsid, kApPass);
  g_udp.begin(kSubtitlePort);

  g_frame_buf = static_cast<uint8_t *>(ps_malloc(kFrameBufSize));
  if (!g_frame_buf) {
    g_frame_buf = static_cast<uint8_t *>(malloc(kFrameBufSize));
  }
  if (!g_frame_buf) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.setCursor(4, 4);
    M5.Display.print("Frame buffer alloc failed");
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
  M5.update();
  updateTenstarStatus();

  if (g_state == AppState::FILE_SELECT) {
    if (M5.BtnPWR.pressedFor(700)) {
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
    if (M5.BtnB.wasPressed() && g_entry_count > 0) {
      g_entry_index = (g_entry_index + 1) % g_entry_count;
      drawFileMenu();
    }
    if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed() && g_entry_count > 0) {
      g_entry_index = (g_entry_index + g_entry_count - 1) % g_entry_count;
      drawFileMenu();
    }
    if (M5.BtnA.wasPressed() && g_entry_count > 0) {
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
    if (M5.BtnPWR.pressedFor(700)) {
      if (g_file) g_file.close();
      g_paused = false;
      clearSubtitleState();
      g_state = AppState::FILE_SELECT;
      drawFileMenu();
      delay(10);
      return;
    }
    if (M5.BtnA.wasPressed()) {
      g_paused = !g_paused;
      drawPlaybackOverlay();
    }
    if (M5.BtnB.wasPressed()) {
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
    TJpgDec.drawJpg(0, 0, g_frame_buf, g_frame_len);
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

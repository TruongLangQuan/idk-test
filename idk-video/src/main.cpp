#include <M5Unified.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TJpg_Decoder.h>

static const uint32_t kFrameDelayMs = 100; // 10 fps default
static const size_t kFrameBufSize = 80 * 1024;
static const int kMaxFiles = 16;

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

static uint8_t *g_frame_buf = nullptr;
static size_t g_frame_len = 0;
static File g_file;
static bool g_paused = false;

static AppState g_state = AppState::FILE_SELECT;
static FileEntry g_entries[kMaxFiles];
static int g_entry_count = 0;
static int g_entry_index = 0;
static String g_status = "";
static String g_current_path = "";
static String g_dir = "/";
static fs::FS *g_fs = nullptr;
static bool g_sd_ready = false;

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

static void drawFileMenu() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("SD:%s", g_dir.c_str());
  M5.Display.setCursor(2, 14);
  M5.Display.print("A:open  B:next  PWR:prev");
  M5.Display.setCursor(2, 24);
  M5.Display.print("PWR hold:up");

  if (g_entry_count <= 0) {
    M5.Display.setCursor(2, 46);
    M5.Display.print("No entries");
  } else {
    int start = g_entry_index - 3;
    if (start < 0) start = 0;
    int end = start + 7;
    if (end > g_entry_count) end = g_entry_count;
    int y = 44;
    for (int i = start; i < end; ++i) {
      M5.Display.setTextColor(i == g_entry_index ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(6, y);
      const char *tag = g_entries[i].isDir ? "[D]" : "   ";
      M5.Display.printf("%c %s %s", i == g_entry_index ? '>' : ' ', tag, g_entries[i].name.c_str());
      y += 12;
    }
  }

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 126);
  M5.Display.print(g_status);
}

static bool scanDir(const String &dirPath) {
  g_entry_count = 0;
  g_entry_index = 0;

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
      g_entries[g_entry_count].name = name;
      g_entries[g_entry_count].path = joinPath(dirPath, name);
      g_entries[g_entry_count].isDir = true;
      g_entry_count++;
    } else if (isVideoFile(name)) {
      g_entries[g_entry_count].name = name;
      g_entries[g_entry_count].path = joinPath(dirPath, name);
      g_entries[g_entry_count].isDir = false;
      g_entry_count++;
    }
    f = root.openNextFile();
  }
  root.close();

  if (g_entry_count == 0) {
    g_status = "No entries";
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

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3); // Left landscape
  M5.Display.setBrightness(180);

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
        g_paused = false;
        g_state = AppState::PLAYING;
      } else {
        drawFileMenu();
      }
    }
    delay(10);
    return;
  }

  if (g_state == AppState::PLAYING) {
    if (M5.BtnPWR.wasPressed()) {
      if (g_file) g_file.close();
      g_state = AppState::FILE_SELECT;
      drawFileMenu();
      delay(10);
      return;
    }
    if (M5.BtnA.wasPressed()) {
      if (g_file) g_file.seek(0);
    }
    if (M5.BtnB.wasPressed()) {
      g_paused = !g_paused;
    }

    if (!g_file) {
      g_status = "File closed";
      g_state = AppState::FILE_SELECT;
      drawFileMenu();
      delay(10);
      return;
    }

    if (g_paused) {
      delay(20);
      return;
    }

    if (!readNextFrame(g_file, g_frame_buf, kFrameBufSize, g_frame_len)) {
      g_file.seek(0);
      return;
    }

    uint32_t t0 = millis();
    TJpgDec.drawJpg(0, 0, g_frame_buf, g_frame_len);
    uint32_t dt = millis() - t0;
    if (dt < kFrameDelayMs) {
      delay(kFrameDelayMs - dt);
    }
    return;
  }
}

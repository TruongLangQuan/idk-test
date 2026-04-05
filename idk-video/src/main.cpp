#include <M5Unified.h>
#include <FS.h>
#include <LittleFS.h>
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

enum class MediaSource {
  SD,
  LITTLEFS,
};

enum class AppState {
  SOURCE_SELECT,
  FILE_SELECT,
  PLAYING,
};

static uint8_t *g_frame_buf = nullptr;
static size_t g_frame_len = 0;
static File g_file;
static bool g_paused = false;

static MediaSource g_source = MediaSource::SD;
static AppState g_state = AppState::SOURCE_SELECT;
static String g_files[kMaxFiles];
static int g_file_count = 0;
static int g_file_index = 0;
static String g_status = "";
static String g_current_path = "";
static fs::FS *g_fs = nullptr;
static bool g_sd_ready = false;
static bool g_lfs_ready = false;

static const char *sourceName(MediaSource src) {
  return (src == MediaSource::SD) ? "SD" : "LittleFS";
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

static bool ensureLfsReady() {
  if (g_lfs_ready) return true;
  g_lfs_ready = LittleFS.begin();
  return g_lfs_ready;
}

static void drawSourceMenu() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(2, 2);
  M5.Display.print("idk-video Source");
  M5.Display.setCursor(2, 14);
  M5.Display.print("A:select  B:next  PWR:prev");

  const char *items[2] = {"SD Card", "LittleFS"};
  int idx = (g_source == MediaSource::SD) ? 0 : 1;
  int y = 36;
  for (int i = 0; i < 2; ++i) {
    M5.Display.setTextColor(i == idx ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(6, y + i * 14);
    M5.Display.printf("%c %s", i == idx ? '>' : ' ', items[i]);
  }

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 126);
  M5.Display.print(g_status);
}

static void drawFileMenu() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("Videos (%s)", sourceName(g_source));
  M5.Display.setCursor(2, 14);
  M5.Display.print("A:play  B:next  PWR:prev");
  M5.Display.setCursor(2, 24);
  M5.Display.print("PWR hold:back");

  if (g_file_count <= 0) {
    M5.Display.setCursor(2, 46);
    M5.Display.print("No videos found");
  } else {
    int start = g_file_index - 3;
    if (start < 0) start = 0;
    int end = start + 7;
    if (end > g_file_count) end = g_file_count;
    int y = 44;
    for (int i = start; i < end; ++i) {
      M5.Display.setTextColor(i == g_file_index ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
      M5.Display.setCursor(6, y);
      M5.Display.printf("%c %s", i == g_file_index ? '>' : ' ', g_files[i].c_str());
      y += 12;
    }
  }

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(2, 126);
  M5.Display.print(g_status);
}

static bool scanVideos(MediaSource src) {
  g_file_count = 0;
  g_file_index = 0;
  fs::FS *fs = nullptr;

  if (src == MediaSource::SD) {
    if (!ensureSdReady()) {
      g_status = "SD mount failed";
      return false;
    }
    fs = &SD;
  } else {
    if (!ensureLfsReady()) {
      g_status = "LittleFS mount failed";
      return false;
    }
    fs = &LittleFS;
  }

  g_fs = fs;
  File root = fs->open("/");
  if (!root) {
    g_status = "Open / failed";
    return false;
  }

  File f = root.openNextFile();
  while (f && g_file_count < kMaxFiles) {
    if (!f.isDirectory()) {
      String name = f.name();
      if (isVideoFile(name)) {
        g_files[g_file_count++] = name;
      }
    }
    f = root.openNextFile();
  }
  root.close();

  if (g_file_count == 0) {
    g_status = "No video files";
  } else {
    g_status = String("Found ") + g_file_count;
  }
  return true;
}

static bool openCurrentVideo() {
  if (!g_fs || g_file_count <= 0) return false;
  if (g_file) g_file.close();
  g_current_path = g_files[g_file_index];
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

  g_status = "Select source";
  drawSourceMenu();
}

void loop() {
  M5.update();

  if (g_state == AppState::SOURCE_SELECT) {
    if (M5.BtnB.wasPressed()) {
      g_source = (g_source == MediaSource::SD) ? MediaSource::LITTLEFS : MediaSource::SD;
      drawSourceMenu();
    }
    if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed()) {
      g_source = (g_source == MediaSource::SD) ? MediaSource::LITTLEFS : MediaSource::SD;
      drawSourceMenu();
    }
    if (M5.BtnA.wasPressed()) {
      g_status = "Scanning...";
      drawSourceMenu();
      if (scanVideos(g_source)) {
        g_state = AppState::FILE_SELECT;
        drawFileMenu();
      } else {
        drawSourceMenu();
      }
    }
    delay(10);
    return;
  }

  if (g_state == AppState::FILE_SELECT) {
    if (M5.BtnPWR.pressedFor(700)) {
      g_state = AppState::SOURCE_SELECT;
      drawSourceMenu();
      delay(200);
      return;
    }
    if (M5.BtnB.wasPressed() && g_file_count > 0) {
      g_file_index = (g_file_index + 1) % g_file_count;
      drawFileMenu();
    }
    if (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed() && g_file_count > 0) {
      g_file_index = (g_file_index + g_file_count - 1) % g_file_count;
      drawFileMenu();
    }
    if (M5.BtnA.wasPressed() && g_file_count > 0) {
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

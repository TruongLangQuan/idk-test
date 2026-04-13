#include <M5Unified.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

namespace {

static const int kMaxFiles = 128;
static const int kHeaderH = 20;
static const int kFooterH = 16;
static const uint16_t kFg = 0x07E0;
static const uint16_t kAccent = 0xF800;
static const uint16_t kBg = 0x0000;

// SD pins aligned with m5stickc plus2
static const int kSdCsPin = 14;
static const int kSdSckPin = 0;
static const int kSdMisoPin = 36;
static const int kSdMosiPin = 26;

struct FileEntry {
  String name;
  String path;
};

struct __attribute__((packed)) WavHeader {
  char RIFF[4];
  uint32_t chunk_size;
  char WAVEfmt[8];
  uint32_t fmt_chunk_size;
  uint16_t audiofmt;
  uint16_t channel;
  uint32_t sample_rate;
  uint32_t byte_per_sec;
  uint16_t block_size;
  uint16_t bit_per_sample;
};

struct __attribute__((packed)) SubChunk {
  char identifier[4];
  uint32_t chunk_size;
  uint8_t data[1];
};

static FileEntry g_entries[kMaxFiles];
static int g_entry_count = 0;
static int g_entry_index = 0;
static String g_status = "";
static String g_dir = "/";
static bool g_sd_ready = false;

static bool ensureSdReady() {
  if (g_sd_ready) return true;
  SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
  g_sd_ready = SD.begin(kSdCsPin, SPI);
  return g_sd_ready;
}

static String trimText(const String &text, int maxLen) {
  if (text.length() <= maxLen) return text;
  if (maxLen <= 3) return text.substring(0, maxLen);
  return text.substring(0, maxLen - 3) + "...";
}

static bool isWavFile(const String &name) {
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".wav");
}

static bool scanDir(const String &dirPath) {
  g_entry_count = 0;
  g_entry_index = 0;

  if (!ensureSdReady()) {
    g_status = "SD mount failed";
    return false;
  }

  File root = SD.open(dirPath.c_str());
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
    if (slash >= 0) name = name.substring(slash + 1);
    if (name.length() == 0) {
      f = root.openNextFile();
      continue;
    }
    if (!f.isDirectory() && isWavFile(name)) {
      g_entries[g_entry_count].name = name;
      g_entries[g_entry_count].path = (dirPath == "/") ? String("/") + name : dirPath + "/" + name;
      g_entry_count++;
    }
    f = root.openNextFile();
  }
  root.close();
  g_status = (g_entry_count > 0) ? String("Found ") + g_entry_count : "No wav";
  return true;
}

static void drawFrame() {
  int w = M5.Display.width();
  int h = M5.Display.height();
  M5.Display.fillScreen(kBg);
  M5.Display.drawRoundRect(3, 3, w - 6, h - 6, 5, kFg);
  M5.Display.drawLine(5, kHeaderH, w - 6, kHeaderH, kFg);
  M5.Display.drawLine(5, h - kFooterH - 2, w - 6, h - kFooterH - 2, kFg);
  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(8, 6);
  M5.Display.print("IDK-AUDIO");
}

static void drawMenu() {
  drawFrame();
  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setCursor(8, kHeaderH + 4);
  M5.Display.printf("SD:%s", trimText(g_dir, 26).c_str());

  if (g_entry_count == 0) {
    M5.Display.setTextColor(kFg, kBg);
    M5.Display.setCursor(10, 40);
    M5.Display.print("No .wav files");
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
      if (selected) {
        M5.Display.fillRoundRect(8, y - 1, M5.Display.width() - 16, 11, 3, kFg);
        M5.Display.setTextColor(kBg, kFg);
      } else {
        M5.Display.setTextColor(kFg, kBg);
      }
      String label = trimText(g_entries[i].name, 24);
      M5.Display.setCursor(10, y);
      M5.Display.printf("%c %s", selected ? '>' : ' ', label.c_str());
      y += 12;
    }
  }

  M5.Display.setTextColor(kAccent, kBg);
  M5.Display.setCursor(8, M5.Display.height() - kFooterH - 12);
  M5.Display.print(trimText(g_status, 28));
  M5.Display.setTextColor(kFg, kBg);
  M5.Display.setCursor(10, M5.Display.height() - kFooterH + 2);
  M5.Display.print("A:Play  B:Next");
}

static bool playWav(const String &path) {
  File file = SD.open(path.c_str(), FILE_READ);
  if (!file) {
    g_status = "Open failed";
    return false;
  }

  WavHeader header;
  if (file.read((uint8_t*)&header, sizeof(WavHeader)) != sizeof(WavHeader)) {
    file.close();
    g_status = "Bad header";
    return false;
  }

  if (memcmp(header.RIFF, "RIFF", 4) || memcmp(header.WAVEfmt, "WAVEfmt ", 8) || header.audiofmt != 1 ||
      header.bit_per_sample < 8 || header.bit_per_sample > 16 || header.channel == 0 || header.channel > 2) {
    file.close();
    g_status = "Unsupported wav";
    return false;
  }

  file.seek(offsetof(WavHeader, audiofmt) + header.fmt_chunk_size);
  SubChunk sub;
  file.read((uint8_t*)&sub, 8);
  while (memcmp(sub.identifier, "data", 4) != 0) {
    if (!file.seek(sub.chunk_size, SeekMode::SeekCur)) break;
    if (file.read((uint8_t*)&sub, 8) != 8) break;
  }
  if (memcmp(sub.identifier, "data", 4) != 0) {
    file.close();
    g_status = "No data chunk";
    return false;
  }

  const size_t buf_size = 1024;
  static uint8_t buf[buf_size];
  int32_t data_len = sub.chunk_size;
  bool is16 = (header.bit_per_sample >> 4);

  while (data_len > 0) {
    size_t len = data_len < (int32_t)buf_size ? data_len : buf_size;
    len = file.read(buf, len);
    if (len == 0) break;
    data_len -= len;

    if (is16) {
      M5.Speaker.playRaw((const int16_t*)buf, len >> 1, header.sample_rate, header.channel > 1, 1, 0);
    } else {
      M5.Speaker.playRaw((const uint8_t*)buf, len, header.sample_rate, header.channel > 1, 1, 0);
    }

    M5.update();
    if (M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed()) {
      break;
    }
  }

  file.close();
  return true;
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  M5.Display.setBrightness(180);
  M5.Speaker.setVolume(128);

  g_status = "Scanning...";
  scanDir(g_dir);
  drawMenu();
}

void loop() {
  M5.update();

  if (M5.BtnB.wasPressed() && g_entry_count > 0) {
    g_entry_index = (g_entry_index + 1) % g_entry_count;
    drawMenu();
  }
  if (M5.BtnPWR.wasPressed() && g_entry_count > 0) {
    g_entry_index = (g_entry_index + g_entry_count - 1) % g_entry_count;
    drawMenu();
  }
  if (M5.BtnA.wasPressed() && g_entry_count > 0) {
    g_status = "Playing...";
    drawMenu();
    playWav(g_entries[g_entry_index].path);
    g_status = "Done";
    drawMenu();
  }

  delay(10);
}

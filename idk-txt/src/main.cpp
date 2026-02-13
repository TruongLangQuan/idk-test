#include <M5Unified.h>
#include <cstring>
#include <lgfx/v1/misc/DataWrapper.hpp>
#include "generated/txt_asset.h"
#include "generated/vi12_font.h"

static size_t g_page = 0;
static constexpr size_t LINES_PER_PAGE = 9;
static bool g_font_loaded = false;

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

static MemoryFontWrapper g_font_wrap(vi12_font, vi12_font_len);

void drawPage() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextWrap(false, false);
  if (!g_font_loaded) {
    M5.Display.setFont(&fonts::Font2);
  }
  M5.Display.setCursor(2, 2);

  const size_t from = g_page * LINES_PER_PAGE;
  for (size_t i = 0; i < LINES_PER_PAGE; ++i) {
    const size_t idx = from + i;
    if (idx >= TXT_LINE_COUNT) break;
    M5.Display.println(TXT_LINES[idx]);
  }

  const size_t pages = (TXT_LINE_COUNT + LINES_PER_PAGE - 1) / LINES_PER_PAGE;
  M5.Display.fillRect(0, 124, 240, 11, TFT_BLACK);
  M5.Display.setCursor(2, 126);
  M5.Display.printf("Page %u/%u", (unsigned)(g_page + 1), (unsigned)(pages ? pages : 1));
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(3);
  g_font_loaded = g_font_wrap.open(nullptr) && M5.Display.loadFont(&g_font_wrap);
  drawPage();
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed() || M5.BtnA.pressedFor(350)) {
    if ((g_page + 1) * LINES_PER_PAGE < TXT_LINE_COUNT) {
      ++g_page;
      drawPage();
      delay(80);
    }
  }

  if (M5.BtnB.wasPressed() || M5.BtnB.pressedFor(350) ||
      (M5.BtnPWR.wasPressed() && !M5.BtnA.isPressed())) {
    if (g_page > 0) {
      --g_page;
      drawPage();
      delay(80);
    }
  }

  if (M5.BtnPWR.pressedFor(700)) {
    if (g_page != 0) {
      g_page = 0;
      drawPage();
      delay(120);
    }
  }

  delay(10);
}

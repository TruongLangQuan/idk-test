#pragma once

#include <M5GFX.h>
#include <cstring>
#include <lgfx/v1/misc/DataWrapper.hpp>

#include "../idk-txt/include/generated/vi12_font.h"

namespace idk_vi_font {

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

  void close() override {}
  int32_t tell() override { return static_cast<int32_t>(pos_); }

 private:
  const uint8_t* data_;
  size_t len_;
  size_t pos_;
};

static inline MemoryFontWrapper& fontWrapper() {
  static MemoryFontWrapper wrapper(vi12_font, vi12_font_len);
  return wrapper;
}

template <typename DisplayT>
static bool load(DisplayT& display) {
  auto& wrapper = fontWrapper();
  return wrapper.open(nullptr) && display.loadFont(&wrapper);
}

template <typename DisplayT>
static void unload(DisplayT& display) {
  display.unloadFont();
}

template <typename DisplayT>
static void drawWrapped(DisplayT& display, const String& text, int x, int y, int w, int h, uint16_t fg,
                        uint16_t bg, int line_h = 14) {
  display.fillRect(x, y, w, h, bg);
  display.setTextColor(fg, bg);
  display.setTextWrap(false, false);

  int cx = x;
  int cy = y;
  String word;

  auto flushWord = [&](bool with_space) {
    if (word.isEmpty()) return;
    String token = word;
    if (with_space) token += ' ';
    int token_w = display.textWidth(token);
    if (cx + token_w > x + w) {
      cx = x;
      cy += line_h;
    }
    if (cy + line_h > y + h) {
      word = "";
      return;
    }
    display.setCursor(cx, cy);
    display.print(token);
    cx += token_w;
    word = "";
  };

  for (size_t i = 0; i < text.length(); ++i) {
    char ch = text[i];
    if (ch == '\r') continue;
    if (ch == '\n') {
      flushWord(false);
      cx = x;
      cy += line_h;
      if (cy + line_h > y + h) break;
      continue;
    }
    if (ch == ' ') {
      flushWord(true);
      continue;
    }
    word += ch;
  }
  flushWord(false);
}

}  // namespace idk_vi_font

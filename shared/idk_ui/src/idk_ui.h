#pragma once

#include <M5Unified.h>
#include <WiFi.h>
#include <cctype>
#include <cstdint>
#include <lgfx/v1/misc/DataWrapper.hpp>

struct KnownWifi {
  const char* ssid;
  const char* pass;
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

bool loadFontFromMemory(const uint8_t* data, size_t len, MemoryFontWrapper& wrap);

String truncateUtf8(const String& s, size_t maxBytes);
String formatFloat(double value, int decimals = 4);

bool keyboardInput(String& out, const String& title, bool mask_input, bool allow_telex, bool& telexMode,
                   size_t maxLen = 64);

bool inputNumber(double& out, const String& title, bool& telexMode, bool allowNegative = true);

bool autoConnectKnownWifi(const KnownWifi* list, size_t count, String& status, uint32_t timeoutMs = 8000);

void drawWrapped(const String& text, int x, int y, int maxW, int maxLines);
void showResult(const char* title, const String& body);

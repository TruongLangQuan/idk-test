#pragma once

#include "ui_backend.h"

namespace idk {

class CydDisplayUI : public UIBackend {
 public:
  explicit CydDisplayUI(uint32_t intervalMs) : intervalMs_(intervalMs) {}

  void begin(const RuntimeConfig& config) override;
  void update(const TelemetryState& state) override;

 private:
  void drawLayout(const RuntimeConfig& config);
  void drawField(uint8_t row, const char* label, const char* value, uint16_t color = 0xFFFF);

  static constexpr uint8_t kFieldCount = 9;
  static constexpr uint8_t kValueMax = 48;

  uint32_t intervalMs_ = 200;
  uint32_t lastDrawMs_ = 0;
  char cachedValues_[kFieldCount][kValueMax]{};
};

}  // namespace idk

#pragma once

#include "ui_backend.h"

namespace idk {

class SerialTelemetryUI : public UIBackend {
 public:
  explicit SerialTelemetryUI(uint32_t intervalMs) : intervalMs_(intervalMs) {}

  void begin(const RuntimeConfig& config) override;
  void update(const TelemetryState& state) override;

 private:
  uint32_t intervalMs_ = 1000;
  uint32_t lastPrintMs_ = 0;
};

}  // namespace idk

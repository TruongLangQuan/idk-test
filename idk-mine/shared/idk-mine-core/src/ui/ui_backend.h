#pragma once

#include "config/runtime_config.h"
#include "telemetry/telemetry_state.h"

namespace idk {

class UIBackend {
 public:
  virtual ~UIBackend() = default;
  virtual void begin(const RuntimeConfig& config) = 0;
  virtual void update(const TelemetryState& state) = 0;
};

}  // namespace idk

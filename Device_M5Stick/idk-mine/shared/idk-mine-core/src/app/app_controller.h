#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "app/project_profile.h"
#include "config/runtime_config.h"
#include "miner/miner_engine.h"
#include "network/stratum_client.h"
#include "network/wifi_manager.h"
#include "telemetry/telemetry_state.h"
#include "ui/ui_backend.h"

namespace idk {

class AppController {
 public:
  explicit AppController(const ProjectProfile& profile);

  void begin();
  void loop();

 private:
  static void networkTaskEntry(void* ctx);
  static void uiTaskEntry(void* ctx);

  void networkTaskLoop();
  void uiTaskLoop();

  void updateTelemetry(const TelemetryState& snapshot);
  TelemetryState snapshotTelemetry();
  void setStatus(const char* statusText);

  ProjectProfile profile_;
  RuntimeConfig config_{};

  WifiManager wifi_;
  StratumClient pool_;
  MinerEngine miner_;

  UIBackend* ui_ = nullptr;

  TaskHandle_t networkTask_ = nullptr;
  TaskHandle_t uiTask_ = nullptr;

  SemaphoreHandle_t telemetryMutex_ = nullptr;
  TelemetryState telemetry_{};

  bool started_ = false;
};

}  // namespace idk

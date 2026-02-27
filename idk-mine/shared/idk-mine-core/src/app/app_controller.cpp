#include "app_controller.h"

#include <ArduinoOTA.h>
#include <LittleFS.h>

#include "ui/serial_telemetry_ui.h"

#if IDK_ENABLE_GUI
#include "ui/cyd_display_ui.h"
#endif

namespace idk {
namespace {

void safeCopy(char* dst, size_t dstSize, const char* src) {
  if (dst == nullptr || dstSize == 0) {
    return;
  }
  if (src == nullptr) {
    dst[0] = '\0';
    return;
  }
  strlcpy(dst, src, dstSize);
}

}  // namespace

AppController::AppController(const ProjectProfile& profile) : profile_(profile) {}

void AppController::begin() {
  if (started_) {
    return;
  }

  Serial.begin(115200);
  delay(200);

  char err[96] = {0};
  if (!loadDefaultsFromJson(profile_.defaultConfigJson, config_, err, sizeof(err))) {
    Serial.printf("[config] default parse failed: %s\n", err);
  }

  safeCopy(config_.projectName, sizeof(config_.projectName), profile_.projectName);
  safeCopy(config_.minerName, sizeof(config_.minerName), profile_.minerName);

  if (!profile_.enableOtaByDefault) {
    config_.enableOTA = false;
  }

  if (!LittleFS.begin(true)) {
    Serial.println("[config] LittleFS mount failed; using defaults");
  } else {
    if (mergeFromFile(LittleFS, "/runtime_config.json", config_, err, sizeof(err))) {
      Serial.println("[config] loaded /runtime_config.json");
    } else {
      Serial.printf("[config] %s\n", err);
    }
  }

  safeCopy(telemetry_.coin, sizeof(telemetry_.coin), coinToString(config_.defaultCoin));
  setStatus("booting");

  wifi_.begin(profile_.wifiSsid, profile_.wifiPassword, config_.wifiReconnectMs);

  pool_.begin(&config_);
  pool_.setCoin(config_.defaultCoin);
  pool_.setIdentity(walletForCoin(config_, config_.defaultCoin), config_.workerName, config_.minerName,
                    config_.poolPassword);

  const MinerMode mode = (profile_.variant == VariantKind::Lottery) ? MinerMode::Lottery : MinerMode::Mine;
  miner_.begin(config_, mode);

#if IDK_ENABLE_GUI
  if (profile_.device == DeviceKind::CYD) {
    ui_ = new CydDisplayUI(config_.uiUpdateMs);
  } else {
    ui_ = new SerialTelemetryUI(config_.telemetryIntervalMs);
  }
#else
  ui_ = new SerialTelemetryUI(config_.telemetryIntervalMs);
#endif

  if (ui_ != nullptr) {
    ui_->begin(config_);
  }

  telemetryMutex_ = xSemaphoreCreateMutex();

  if (config_.enableOTA) {
    ArduinoOTA.setHostname(config_.projectName);
    ArduinoOTA.onStart([this]() { setStatus("ota:start"); });
    ArduinoOTA.onEnd([this]() { setStatus("ota:end"); });
    ArduinoOTA.onError([this](ota_error_t error) {
      char buf[48];
      snprintf(buf, sizeof(buf), "ota:error:%u", static_cast<unsigned>(error));
      setStatus(buf);
    });
    ArduinoOTA.begin();
  }

  xTaskCreatePinnedToCore(networkTaskEntry, "idk-net", 6144, this, 2, &networkTask_, 0);
  xTaskCreatePinnedToCore(uiTaskEntry, "idk-ui", 4096, this, 1, &uiTask_, 1);

  started_ = true;
}

void AppController::loop() {
  delay(1000);
}

void AppController::networkTaskEntry(void* ctx) {
  static_cast<AppController*>(ctx)->networkTaskLoop();
}

void AppController::uiTaskEntry(void* ctx) {
  static_cast<AppController*>(ctx)->uiTaskLoop();
}

void AppController::networkTaskLoop() {
  uint32_t lastMetricsMs = 0;

  while (true) {
    const uint32_t now = millis();

    wifi_.loop(now);
    const bool wifiConnected = wifi_.connected();

    pool_.loop(now, wifiConnected);

    if (config_.enableOTA && wifiConnected) {
      ArduinoOTA.handle();
    }

    StratumJob job;
    if (pool_.takeLatestJob(job)) {
      const uint32_t fallbackTarget = (profile_.variant == VariantKind::Lottery) ? config_.lotteryTarget32 : config_.mineTarget32;
      miner_.updateJob(job, fallbackTarget);

      TelemetryState t = snapshotTelemetry();
      safeCopy(t.poolJob, sizeof(t.poolJob), job.jobId);
      safeCopy(t.poolTarget, sizeof(t.poolTarget), job.targetHex);
      t.target32 = (job.target32 == 0) ? fallbackTarget : job.target32;
      updateTelemetry(t);
    }

    ShareCandidate share;
    if (miner_.takeShareCandidate(share)) {
      pool_.submitPseudoShare(share.nonce, share.hash32);
    }

    if (lastMetricsMs == 0 || now - lastMetricsMs >= config_.telemetryIntervalMs) {
      TelemetryState t = snapshotTelemetry();

      t.totalHash = miner_.totalHashes();
      t.bestDiff = miner_.bestDifficulty();
      t.blockFound = miner_.blocksFound();
      t.currentHashrate = miner_.currentHashrate(now);

      t.wifiConnected = wifiConnected;
      t.poolConnected = pool_.connected();
      t.reconnectCount = wifi_.reconnectCount() + pool_.reconnectCount();
      t.acceptedShares = pool_.acceptedShares();
      t.rejectedShares = pool_.rejectedShares();

      if (!wifiConnected) {
        safeCopy(t.status, sizeof(t.status), wifi_.statusText());
      } else {
        safeCopy(t.status, sizeof(t.status), pool_.statusText());
      }

      updateTelemetry(t);
      lastMetricsMs = now;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void AppController::uiTaskLoop() {
  while (true) {
    if (ui_ != nullptr) {
      const TelemetryState t = snapshotTelemetry();
      ui_->update(t);
    }

    const uint32_t interval = (config_.uiUpdateMs == 0) ? 200 : config_.uiUpdateMs;
    vTaskDelay(pdMS_TO_TICKS(interval));
  }
}

void AppController::updateTelemetry(const TelemetryState& snapshot) {
  if (telemetryMutex_ == nullptr) {
    telemetry_ = snapshot;
    return;
  }

  if (xSemaphoreTake(telemetryMutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
    telemetry_ = snapshot;
    xSemaphoreGive(telemetryMutex_);
  }
}

TelemetryState AppController::snapshotTelemetry() {
  if (telemetryMutex_ == nullptr) {
    return telemetry_;
  }

  TelemetryState out = telemetry_;
  if (xSemaphoreTake(telemetryMutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
    out = telemetry_;
    xSemaphoreGive(telemetryMutex_);
  }
  return out;
}

void AppController::setStatus(const char* statusText) {
  TelemetryState t = snapshotTelemetry();
  safeCopy(t.status, sizeof(t.status), statusText);
  updateTelemetry(t);
}

}  // namespace idk

#include "serial_telemetry_ui.h"

namespace idk {

void SerialTelemetryUI::begin(const RuntimeConfig& config) {
  intervalMs_ = (intervalMs_ == 0) ? config.telemetryIntervalMs : intervalMs_;
  lastPrintMs_ = 0;

  Serial.println("=== idk-mine headless telemetry ===");
  Serial.printf("project=%s miner=%s worker=%s default_coin=%s\n", config.projectName, config.minerName,
                config.workerName, coinToString(config.defaultCoin));
}

void SerialTelemetryUI::update(const TelemetryState& state) {
  const uint32_t now = millis();
  if (lastPrintMs_ != 0 && (now - lastPrintMs_ < intervalMs_)) {
    return;
  }
  lastPrintMs_ = now;

  Serial.printf(
      "coin=%s wifi=%d pool=%d hash_total=%llu best_diff=%.6f pool_job=%s pool_target=%s target32=%lu "
      "blocks=%lu hashrate=%.2fH/s accepted=%lu rejected=%lu status=%s\n",
      state.coin, static_cast<int>(state.wifiConnected), static_cast<int>(state.poolConnected),
      static_cast<unsigned long long>(state.totalHash), state.bestDiff, state.poolJob, state.poolTarget,
      static_cast<unsigned long>(state.target32), static_cast<unsigned long>(state.blockFound),
      state.currentHashrate, static_cast<unsigned long>(state.acceptedShares),
      static_cast<unsigned long>(state.rejectedShares), state.status);
}

}  // namespace idk

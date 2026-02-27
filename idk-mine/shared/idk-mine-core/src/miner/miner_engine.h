#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <atomic>

#include "config/runtime_config.h"
#include "network/stratum_client.h"

namespace idk {

enum class MinerMode : uint8_t {
  Lottery = 0,
  Mine = 1,
};

struct ShareCandidate {
  uint32_t nonce;
  uint32_t hash32;
};

class MinerEngine {
 public:
  void begin(const RuntimeConfig& cfg, MinerMode mode);
  void stop();

  void updateJob(const StratumJob& job, uint32_t fallbackTarget32);

  bool takeShareCandidate(ShareCandidate& out);

  uint64_t totalHashes() const;
  uint32_t bestHash() const;
  float bestDifficulty() const;
  uint32_t blocksFound() const;
  float currentHashrate(uint32_t nowMs);

 private:
  static void workerEntry(void* ctx);
  void workerLoop(uint8_t workerIndex);

  static uint32_t pseudoHash(uint32_t seed, uint32_t nonce);
  static bool updateAtomicMin(std::atomic<uint32_t>& target, uint32_t value);

  static constexpr uint8_t kMaxWorkers = 2;

  RuntimeConfig config_{};
  MinerMode mode_ = MinerMode::Lottery;
  uint8_t workerCount_ = 1;

  std::atomic<bool> running_{false};
  std::atomic<uint64_t> totalHashes_{0};
  std::atomic<uint32_t> bestHash_{0xFFFFFFFFu};
  std::atomic<uint32_t> blocksFound_{0};

  std::atomic<uint32_t> target32_{0x0000FFFFu};
  std::atomic<uint32_t> jobSeed_{0};

  std::atomic<uint8_t> pendingShare_{0};
  std::atomic<uint32_t> pendingNonce_{0};
  std::atomic<uint32_t> pendingHash_{0};

  TaskHandle_t workers_[kMaxWorkers] = {nullptr, nullptr};

  uint64_t lastHashTotal_ = 0;
  uint32_t lastHashrateMs_ = 0;
  float cachedHashrate_ = 0.0f;
};

}  // namespace idk

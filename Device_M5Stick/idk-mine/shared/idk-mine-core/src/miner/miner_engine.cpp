#include "miner_engine.h"

namespace idk {

void MinerEngine::begin(const RuntimeConfig& cfg, MinerMode mode) {
  stop();

  config_ = cfg;
  mode_ = mode;

  totalHashes_.store(0);
  bestHash_.store(0xFFFFFFFFu);
  blocksFound_.store(0);
  pendingShare_.store(0);
  pendingNonce_.store(0);
  pendingHash_.store(0);
  target32_.store((mode_ == MinerMode::Lottery) ? config_.lotteryTarget32 : config_.mineTarget32);
  jobSeed_.store(esp_random());

  workerCount_ = (mode_ == MinerMode::Lottery) ? 1 : constrain(config_.minerThreads, static_cast<uint8_t>(1), kMaxWorkers);
  if (workerCount_ < 1) {
    workerCount_ = 1;
  }

  lastHashTotal_ = 0;
  lastHashrateMs_ = millis();
  cachedHashrate_ = 0.0f;

  running_.store(true);
  for (uint8_t i = 0; i < workerCount_; ++i) {
    xTaskCreatePinnedToCore(workerEntry, (i == 0) ? "mine-w0" : "mine-w1", 4096, this, 1, &workers_[i], i % 2);
  }
}

void MinerEngine::stop() {
  running_.store(false);
  for (uint8_t i = 0; i < kMaxWorkers; ++i) {
    if (workers_[i] != nullptr) {
      vTaskDelete(workers_[i]);
      workers_[i] = nullptr;
    }
  }
}

void MinerEngine::updateJob(const StratumJob& job, uint32_t fallbackTarget32) {
  uint32_t target = job.target32;
  if (target == 0) {
    target = fallbackTarget32;
  }
  if (target == 0) {
    target = (mode_ == MinerMode::Lottery) ? 0x0000FFFFu : 0x00000FFFu;
  }

  target32_.store(target);
  jobSeed_.store(job.seed);
}

bool MinerEngine::takeShareCandidate(ShareCandidate& out) {
  if (pendingShare_.exchange(0) == 0) {
    return false;
  }

  out.nonce = pendingNonce_.load();
  out.hash32 = pendingHash_.load();
  return true;
}

uint64_t MinerEngine::totalHashes() const {
  return totalHashes_.load();
}

uint32_t MinerEngine::bestHash() const {
  return bestHash_.load();
}

float MinerEngine::bestDifficulty() const {
  const uint32_t best = bestHash();
  if (best == 0) {
    return 0.0f;
  }
  return static_cast<float>(4294967295.0 / static_cast<double>(best));
}

uint32_t MinerEngine::blocksFound() const {
  return blocksFound_.load();
}

float MinerEngine::currentHashrate(uint32_t nowMs) {
  if (nowMs - lastHashrateMs_ < 1000) {
    return cachedHashrate_;
  }

  const uint64_t current = totalHashes();
  const uint64_t delta = current - lastHashTotal_;
  const uint32_t elapsed = nowMs - lastHashrateMs_;

  if (elapsed > 0) {
    cachedHashrate_ = (static_cast<float>(delta) * 1000.0f) / static_cast<float>(elapsed);
  }

  lastHashTotal_ = current;
  lastHashrateMs_ = nowMs;
  return cachedHashrate_;
}

void MinerEngine::workerEntry(void* ctx) {
  auto* self = static_cast<MinerEngine*>(ctx);
  const uint8_t workerIndex = (xPortGetCoreID() == 0) ? 0 : 1;
  self->workerLoop(workerIndex);
}

void MinerEngine::workerLoop(uint8_t workerIndex) {
  uint32_t nonce = esp_random() ^ (0x9E3779B9u * (workerIndex + 1));
  const uint16_t batchSize = constrain(config_.minerBatchSize, static_cast<uint16_t>(32), static_cast<uint16_t>(4096));
  uint16_t yieldCounter = 0;

  while (running_.load()) {
    const uint32_t seed = jobSeed_.load();
    const uint32_t target = target32_.load();

    uint32_t localBest = 0xFFFFFFFFu;
    bool found = false;
    uint32_t foundNonce = 0;
    uint32_t foundHash = 0;

    for (uint16_t i = 0; i < batchSize; ++i) {
      nonce += 0x01000193u;
      const uint32_t hash32 = pseudoHash(seed, nonce);

      if (hash32 < localBest) {
        localBest = hash32;
      }

      if (hash32 < target) {
        found = true;
        foundNonce = nonce;
        foundHash = hash32;
      }
    }

    totalHashes_.fetch_add(batchSize);
    updateAtomicMin(bestHash_, localBest);

    if (found) {
      blocksFound_.fetch_add(1);
      if (pendingShare_.load() == 0) {
        pendingNonce_.store(foundNonce);
        pendingHash_.store(foundHash);
        pendingShare_.store(1);
      }
    }

    if (mode_ == MinerMode::Lottery) {
      vTaskDelay(pdMS_TO_TICKS(1));
    } else {
      // Leave CPU time for IDLE tasks so task watchdog does not trigger.
      yieldCounter++;
      if ((yieldCounter & 0x07u) == 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
      } else {
        taskYIELD();
      }
    }
  }

  vTaskDelete(nullptr);
}

uint32_t MinerEngine::pseudoHash(uint32_t seed, uint32_t nonce) {
  uint32_t x = seed ^ (nonce * 0x9E3779B9u);
  x ^= x >> 16;
  x *= 0x85ebca6bu;
  x ^= x >> 13;
  x *= 0xc2b2ae35u;
  x ^= x >> 16;
  return x;
}

bool MinerEngine::updateAtomicMin(std::atomic<uint32_t>& target, uint32_t value) {
  uint32_t current = target.load();
  while (value < current) {
    if (target.compare_exchange_weak(current, value)) {
      return true;
    }
  }
  return false;
}

}  // namespace idk

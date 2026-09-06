#pragma once

#include <Arduino.h>
#include <FS.h>

namespace idk {

enum class CoinType : uint8_t {
  BTC = 0,
  LTC = 1,
};

struct PoolEndpointConfig {
  char host[64];
  uint16_t port;
  bool tls;
};

struct RuntimeConfig {
  char projectName[32];
  char minerName[32];
  char workerName[32];

  CoinType defaultCoin;
  bool enableOTA;

  char btcWallet[96];
  char ltcWallet[96];

  PoolEndpointConfig btcPool;
  PoolEndpointConfig ltcPool;

  uint8_t minerThreads;
  uint16_t minerBatchSize;
  uint32_t telemetryIntervalMs;
  uint32_t uiUpdateMs;
  uint32_t wifiReconnectMs;
  uint32_t poolRetryMs;
  uint32_t keepAliveMs;

  uint32_t lotteryTarget32;
  uint32_t mineTarget32;

  char poolPassword[16];
};

bool loadDefaultsFromJson(const char* json, RuntimeConfig& out, char* err, size_t errSize);
bool mergeFromJson(const char* json, RuntimeConfig& inOut, char* err, size_t errSize);
bool mergeFromFile(fs::FS& fs, const char* path, RuntimeConfig& inOut, char* err, size_t errSize);

const PoolEndpointConfig& endpointForCoin(const RuntimeConfig& cfg, CoinType coin);
const char* walletForCoin(const RuntimeConfig& cfg, CoinType coin);
const char* coinToString(CoinType coin);
bool coinFromString(const char* value, CoinType& out);

}  // namespace idk

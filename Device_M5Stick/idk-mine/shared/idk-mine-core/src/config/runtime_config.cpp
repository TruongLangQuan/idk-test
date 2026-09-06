#include "runtime_config.h"

#include <ArduinoJson.h>

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

uint16_t readPort(const JsonVariantConst& v, uint16_t fallback) {
  if (v.is<uint16_t>()) {
    return v.as<uint16_t>();
  }
  if (v.is<int>()) {
    const int value = v.as<int>();
    if (value > 0 && value <= 65535) {
      return static_cast<uint16_t>(value);
    }
  }
  return fallback;
}

void mergePool(JsonVariantConst poolNode, PoolEndpointConfig& out) {
  if (poolNode.isNull()) {
    return;
  }

  const char* host = poolNode["host"] | nullptr;
  if (host != nullptr) {
    safeCopy(out.host, sizeof(out.host), host);
  }

  out.port = readPort(poolNode["port"], out.port);

  if (!poolNode["tls"].isNull()) {
    out.tls = poolNode["tls"].as<bool>();
  }
}

bool applyDocument(JsonDocument& doc, RuntimeConfig& cfg, char* err, size_t errSize) {
  const char* projectName = doc["project_name"] | nullptr;
  if (projectName != nullptr) {
    safeCopy(cfg.projectName, sizeof(cfg.projectName), projectName);
  }

  const char* minerName = doc["miner_name"] | nullptr;
  if (minerName != nullptr) {
    safeCopy(cfg.minerName, sizeof(cfg.minerName), minerName);
  }

  const char* workerName = doc["worker_name"] | nullptr;
  if (workerName != nullptr) {
    safeCopy(cfg.workerName, sizeof(cfg.workerName), workerName);
  }

  if (!doc["enable_ota"].isNull()) {
    cfg.enableOTA = doc["enable_ota"].as<bool>();
  }

  const char* password = doc["pool_password"] | nullptr;
  if (password != nullptr) {
    safeCopy(cfg.poolPassword, sizeof(cfg.poolPassword), password);
  }

  const char* defaultCoin = doc["default_coin"] | nullptr;
  if (defaultCoin != nullptr) {
    CoinType parsed;
    if (!coinFromString(defaultCoin, parsed)) {
      snprintf(err, errSize, "invalid default_coin: %s", defaultCoin);
      return false;
    }
    cfg.defaultCoin = parsed;
  }

  JsonVariantConst wallets = doc["wallets"];
  if (!wallets.isNull()) {
    const char* btc = wallets["btc"] | nullptr;
    if (btc != nullptr) {
      safeCopy(cfg.btcWallet, sizeof(cfg.btcWallet), btc);
    }

    const char* ltc = wallets["ltc"] | nullptr;
    if (ltc != nullptr) {
      safeCopy(cfg.ltcWallet, sizeof(cfg.ltcWallet), ltc);
    }
  }

  JsonVariantConst pool = doc["pool"];
  if (!pool.isNull()) {
    mergePool(pool["btc"], cfg.btcPool);
    mergePool(pool["ltc"], cfg.ltcPool);
  }

  JsonVariantConst timing = doc["timing"];
  if (!timing.isNull()) {
    if (!timing["telemetry_ms"].isNull()) {
      cfg.telemetryIntervalMs = timing["telemetry_ms"].as<uint32_t>();
    }
    if (!timing["ui_ms"].isNull()) {
      cfg.uiUpdateMs = timing["ui_ms"].as<uint32_t>();
    }
    if (!timing["wifi_reconnect_ms"].isNull()) {
      cfg.wifiReconnectMs = timing["wifi_reconnect_ms"].as<uint32_t>();
    }
    if (!timing["pool_retry_ms"].isNull()) {
      cfg.poolRetryMs = timing["pool_retry_ms"].as<uint32_t>();
    }
    if (!timing["keepalive_ms"].isNull()) {
      cfg.keepAliveMs = timing["keepalive_ms"].as<uint32_t>();
    }
  }

  JsonVariantConst miner = doc["miner"];
  if (!miner.isNull()) {
    if (!miner["threads"].isNull()) {
      cfg.minerThreads = static_cast<uint8_t>(constrain(miner["threads"].as<uint8_t>(), 1, 2));
    }
    if (!miner["batch_size"].isNull()) {
      cfg.minerBatchSize = constrain(miner["batch_size"].as<uint16_t>(), 32, 4096);
    }
    if (!miner["lottery_target32"].isNull()) {
      cfg.lotteryTarget32 = miner["lottery_target32"].as<uint32_t>();
    }
    if (!miner["mine_target32"].isNull()) {
      cfg.mineTarget32 = miner["mine_target32"].as<uint32_t>();
    }
  }

  return true;
}

}  // namespace

bool loadDefaultsFromJson(const char* json, RuntimeConfig& out, char* err, size_t errSize) {
  memset(&out, 0, sizeof(out));

  safeCopy(out.projectName, sizeof(out.projectName), "idk-mine");
  safeCopy(out.minerName, sizeof(out.minerName), "idk-nagger");
  safeCopy(out.workerName, sizeof(out.workerName), "esp32");
  out.defaultCoin = CoinType::BTC;
  out.enableOTA = true;

  safeCopy(out.btcWallet, sizeof(out.btcWallet), "bc1qul2pvt0l5deykfznzkzhfjf3yj3cla2cssd54e");
  safeCopy(out.ltcWallet, sizeof(out.ltcWallet), "ltc1qcl9h4kffh8w8nr77u8la5ykn4cg59ms2lteply");

  safeCopy(out.btcPool.host, sizeof(out.btcPool.host), "public-pool.io");
  out.btcPool.port = 3333;
  out.btcPool.tls = false;

  safeCopy(out.ltcPool.host, sizeof(out.ltcPool.host), "rx.unmineable.com");
  out.ltcPool.port = 3333;
  out.ltcPool.tls = false;

  out.minerThreads = 2;
  out.minerBatchSize = 1024;
  out.telemetryIntervalMs = 1000;
  out.uiUpdateMs = 200;
  out.wifiReconnectMs = 5000;
  out.poolRetryMs = 4000;
  out.keepAliveMs = 30000;

  out.lotteryTarget32 = 0x0000FFFF;
  out.mineTarget32 = 0x00000FFF;

  safeCopy(out.poolPassword, sizeof(out.poolPassword), "x");

  if (json == nullptr || json[0] == '\0') {
    return true;
  }

  return mergeFromJson(json, out, err, errSize);
}

bool mergeFromJson(const char* json, RuntimeConfig& inOut, char* err, size_t errSize) {
  if (json == nullptr || json[0] == '\0') {
    return true;
  }

  JsonDocument doc;
  const DeserializationError jsonErr = deserializeJson(doc, json);
  if (jsonErr) {
    snprintf(err, errSize, "json parse failed: %s", jsonErr.c_str());
    return false;
  }

  return applyDocument(doc, inOut, err, errSize);
}

bool mergeFromFile(fs::FS& fs, const char* path, RuntimeConfig& inOut, char* err, size_t errSize) {
  if (!fs.exists(path)) {
    snprintf(err, errSize, "config file not found: %s", path);
    return false;
  }

  File file = fs.open(path, "r");
  if (!file) {
    snprintf(err, errSize, "failed to open %s", path);
    return false;
  }

  const size_t size = file.size();
  if (size == 0 || size > 4096) {
    file.close();
    snprintf(err, errSize, "invalid config size");
    return false;
  }

  String payload;
  payload.reserve(size + 1);
  while (file.available()) {
    payload += static_cast<char>(file.read());
  }
  file.close();

  return mergeFromJson(payload.c_str(), inOut, err, errSize);
}

const PoolEndpointConfig& endpointForCoin(const RuntimeConfig& cfg, CoinType coin) {
  return (coin == CoinType::BTC) ? cfg.btcPool : cfg.ltcPool;
}

const char* walletForCoin(const RuntimeConfig& cfg, CoinType coin) {
  return (coin == CoinType::BTC) ? cfg.btcWallet : cfg.ltcWallet;
}

const char* coinToString(CoinType coin) {
  return (coin == CoinType::BTC) ? "BTC" : "LTC";
}

bool coinFromString(const char* value, CoinType& out) {
  if (value == nullptr) {
    return false;
  }

  if (strcasecmp(value, "BTC") == 0) {
    out = CoinType::BTC;
    return true;
  }
  if (strcasecmp(value, "LTC") == 0) {
    out = CoinType::LTC;
    return true;
  }

  return false;
}

}  // namespace idk

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config/runtime_config.h"

namespace idk {

struct StratumJob {
  char jobId[48];
  char targetHex[24];
  uint32_t target32;
  uint32_t seed;
};

class StratumClient {
 public:
  void begin(const RuntimeConfig* config);
  void setCoin(CoinType coin);
  void setIdentity(const char* wallet, const char* worker, const char* minerName, const char* password);
  void loop(uint32_t nowMs, bool wifiConnected);

  bool connected() const;
  bool authorized() const;
  bool hasActiveJob() const;
  uint32_t reconnectCount() const;
  uint32_t acceptedShares() const;
  uint32_t rejectedShares() const;
  const char* statusText() const;

  bool takeLatestJob(StratumJob& out);
  void submitPseudoShare(uint32_t nonce, uint32_t hash32);

 private:
  void disconnect();
  bool connectSocket(uint32_t nowMs);
  void sendSubscribe();
  void sendAuthorize();
  void sendJsonLine(const char* line);
  void processInput(uint32_t nowMs);
  void processLine(const char* line);
  void tryParseNotify(const JsonVariantConst& params);

  static uint32_t fastHash32(const char* text);
  static uint32_t parseTarget32(const char* targetHex);

  const RuntimeConfig* config_ = nullptr;
  CoinType coin_ = CoinType::BTC;

  char wallet_[96];
  char worker_[32];
  char minerName_[32];
  char password_[16];

  bool usingTls_ = false;
  bool subscribed_ = false;
  bool authorized_ = false;
  bool hasValidJob_ = false;
  bool hasPendingJob_ = false;
  bool waitingAuthorizeResult_ = false;

  uint32_t reconnectCount_ = 0;
  uint32_t acceptedShares_ = 0;
  uint32_t rejectedShares_ = 0;
  uint32_t lastConnectAttemptMs_ = 0;
  uint32_t lastIoMs_ = 0;
  uint32_t nextRequestId_ = 3;

  StratumJob latestJob_;

  char status_[64];
  char lineBuf_[1536];
  size_t lineLen_ = 0;

  uint32_t authorizeRequestId_ = 2;
};

}  // namespace idk

#include "stratum_client.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

namespace idk {
namespace {

WiFiClient gTcpClient;
WiFiClientSecure gTlsClient;
Client* gActiveClient = nullptr;

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

const char* endpointTag(const PoolEndpointConfig& endpoint) {
  return endpoint.tls ? "stratum+tls" : "stratum+tcp";
}

}  // namespace

void StratumClient::begin(const RuntimeConfig* config) {
  config_ = config;
  memset(&latestJob_, 0, sizeof(latestJob_));
  safeCopy(latestJob_.jobId, sizeof(latestJob_.jobId), "-");
  safeCopy(latestJob_.targetHex, sizeof(latestJob_.targetHex), "-");
  hasValidJob_ = false;
  safeCopy(status_, sizeof(status_), "pool:idle");
}

void StratumClient::setCoin(CoinType coin) {
  if (coin_ == coin) {
    return;
  }

  coin_ = coin;
  disconnect();
}

void StratumClient::setIdentity(const char* wallet, const char* worker, const char* minerName,
                                const char* password) {
  safeCopy(wallet_, sizeof(wallet_), wallet);
  safeCopy(worker_, sizeof(worker_), worker);
  safeCopy(minerName_, sizeof(minerName_), minerName);
  safeCopy(password_, sizeof(password_), password);
}

void StratumClient::loop(uint32_t nowMs, bool wifiConnected) {
  if (config_ == nullptr) {
    safeCopy(status_, sizeof(status_), "pool:no-config");
    return;
  }

  if (!wifiConnected) {
    disconnect();
    safeCopy(status_, sizeof(status_), "pool:wait-wifi");
    return;
  }

  if (!connected()) {
    connectSocket(nowMs);
    return;
  }

  if (!subscribed_) {
    sendSubscribe();
    subscribed_ = true;
  }

  if (!authorized_) {
    sendAuthorize();
  }

  processInput(nowMs);

  // Keepalive RPC disabled: some pools reject custom methods and return
  // repeated rpc-error responses.

  if (!gActiveClient->connected()) {
    disconnect();
    safeCopy(status_, sizeof(status_), "pool:disconnected");
  }
}

bool StratumClient::connected() const {
  return (gActiveClient != nullptr) && gActiveClient->connected();
}

bool StratumClient::authorized() const {
  return connected() && authorized_;
}

bool StratumClient::hasActiveJob() const {
  return hasValidJob_;
}

uint32_t StratumClient::reconnectCount() const {
  return reconnectCount_;
}

uint32_t StratumClient::acceptedShares() const {
  return acceptedShares_;
}

uint32_t StratumClient::rejectedShares() const {
  return rejectedShares_;
}

const char* StratumClient::statusText() const {
  return status_;
}

bool StratumClient::takeLatestJob(StratumJob& out) {
  if (!hasPendingJob_) {
    return false;
  }

  out = latestJob_;
  hasPendingJob_ = false;
  return true;
}

void StratumClient::submitPseudoShare(uint32_t nonce, uint32_t hash32) {
  if (!connected() || !authorized_ || !hasValidJob_) {
    return;
  }

  char user[140];
  if (worker_[0] == '\0') {
    snprintf(user, sizeof(user), "%s", wallet_);
  } else {
    snprintf(user, sizeof(user), "%s.%s", wallet_, worker_);
  }

  const uint32_t requestId = nextRequestId_++;

  char payload[320];
  snprintf(payload, sizeof(payload),
           "{\"id\":%lu,\"method\":\"mining.submit\",\"params\":[\"%s\",\"%s\",\"%08lx\",\"%08lx\",\"%08lx\"]}",
           static_cast<unsigned long>(requestId), user, latestJob_.jobId,
           static_cast<unsigned long>(nonce ^ 0x5A5A5A5AUL), static_cast<unsigned long>(millis()),
           static_cast<unsigned long>(hash32));

  sendJsonLine(payload);
}

void StratumClient::disconnect() {
  if (gActiveClient != nullptr) {
    gActiveClient->stop();
  }
  gActiveClient = nullptr;
  usingTls_ = false;
  subscribed_ = false;
  authorized_ = false;
  hasValidJob_ = false;
  waitingAuthorizeResult_ = false;
  lineLen_ = 0;
}

bool StratumClient::connectSocket(uint32_t nowMs) {
  if (nowMs - lastConnectAttemptMs_ < config_->poolRetryMs) {
    return false;
  }

  lastConnectAttemptMs_ = nowMs;

  const PoolEndpointConfig& endpoint = endpointForCoin(*config_, coin_);
  if (endpoint.host[0] == '\0' || endpoint.port == 0) {
    safeCopy(status_, sizeof(status_), "pool:bad-endpoint");
    return false;
  }

  gActiveClient = endpoint.tls ? static_cast<Client*>(&gTlsClient) : static_cast<Client*>(&gTcpClient);
  usingTls_ = endpoint.tls;

  if (usingTls_) {
    gTlsClient.setInsecure();
    gTlsClient.setTimeout(2000);
  } else {
    gTcpClient.setTimeout(2000);
  }

  const bool ok = gActiveClient->connect(endpoint.host, endpoint.port);
  reconnectCount_++;
  if (!ok) {
    safeCopy(status_, sizeof(status_), "pool:connect-failed");
    gActiveClient = nullptr;
    return false;
  }

  subscribed_ = false;
  authorized_ = false;
  waitingAuthorizeResult_ = false;
  lineLen_ = 0;
  lastIoMs_ = nowMs;

  snprintf(status_, sizeof(status_), "pool:connected %s:%u", endpointTag(endpoint), endpoint.port);
  return true;
}

void StratumClient::sendSubscribe() {
  if (!connected()) {
    return;
  }

  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"id\":1,\"method\":\"mining.subscribe\",\"params\":[\"%s\"]}", minerName_);
  sendJsonLine(payload);
}

void StratumClient::sendAuthorize() {
  if (!connected() || waitingAuthorizeResult_) {
    return;
  }

  char user[140];
  if (worker_[0] == '\0') {
    snprintf(user, sizeof(user), "%s", wallet_);
  } else {
    snprintf(user, sizeof(user), "%s.%s", wallet_, worker_);
  }

  char payload[240];
  snprintf(payload, sizeof(payload),
           "{\"id\":%lu,\"method\":\"mining.authorize\",\"params\":[\"%s\",\"%s\"]}",
           static_cast<unsigned long>(authorizeRequestId_), user, password_);
  sendJsonLine(payload);
  waitingAuthorizeResult_ = true;
}

void StratumClient::sendJsonLine(const char* line) {
  if (!connected() || line == nullptr) {
    return;
  }

  gActiveClient->print(line);
  gActiveClient->print("\n");
}

void StratumClient::processInput(uint32_t nowMs) {
  if (!connected()) {
    return;
  }

  while (gActiveClient->available()) {
    const int raw = gActiveClient->read();
    if (raw < 0) {
      break;
    }

    const char ch = static_cast<char>(raw);
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      lineBuf_[lineLen_] = '\0';
      if (lineLen_ > 0) {
        processLine(lineBuf_);
      }
      lineLen_ = 0;
      lastIoMs_ = nowMs;
      continue;
    }

    if (lineLen_ + 1 < sizeof(lineBuf_)) {
      lineBuf_[lineLen_++] = ch;
    } else {
      lineLen_ = 0;
      safeCopy(status_, sizeof(status_), "pool:rx-overflow");
    }
  }
}

void StratumClient::processLine(const char* line) {
  JsonDocument doc;
  const DeserializationError err = deserializeJson(doc, line);
  if (err) {
    safeCopy(status_, sizeof(status_), "pool:json-parse-error");
    return;
  }

  const uint32_t id = doc["id"] | 0;

  JsonVariantConst errorNode = doc["error"];
  if (!errorNode.isNull()) {
    if (id == authorizeRequestId_) {
      authorized_ = false;
      waitingAuthorizeResult_ = false;
    }

    if (id >= 3) {
      rejectedShares_++;
    }

    int errorCode = 0;
    const char* errorText = "rpc-error";
    if (errorNode.is<JsonArrayConst>()) {
      JsonArrayConst arr = errorNode.as<JsonArrayConst>();
      errorCode = arr[0] | 0;
      errorText = arr[1] | errorText;
    } else if (errorNode.is<JsonObjectConst>()) {
      JsonObjectConst obj = errorNode.as<JsonObjectConst>();
      errorCode = obj["code"] | 0;
      errorText = obj["message"] | errorText;
    } else if (errorNode.is<const char*>()) {
      errorText = errorNode.as<const char*>();
    }

    if (errorCode != 0) {
      snprintf(status_, sizeof(status_), "pool:rpc %d", errorCode);
    } else {
      snprintf(status_, sizeof(status_), "pool:%s", errorText);
    }
    return;
  }

  if (id == authorizeRequestId_) {
    waitingAuthorizeResult_ = false;
    authorized_ = doc["result"].is<bool>() ? doc["result"].as<bool>() : false;
    safeCopy(status_, sizeof(status_), authorized_ ? "pool:authorized" : "pool:authorize-failed");
  }

  const char* method = doc["method"] | nullptr;
  if (method != nullptr) {
    if (strcmp(method, "mining.notify") == 0) {
      tryParseNotify(doc["params"]);
      return;
    }

    if (strcmp(method, "mining.set_difficulty") == 0) {
      safeCopy(status_, sizeof(status_), "pool:set-difficulty");
      return;
    }
  }

  if (id >= 3 && doc["result"].is<bool>()) {
    if (doc["result"].as<bool>()) {
      acceptedShares_++;
    } else {
      rejectedShares_++;
    }
  }
}

void StratumClient::tryParseNotify(const JsonVariantConst& params) {
  if (params.isNull() || !params.is<JsonArrayConst>()) {
    safeCopy(status_, sizeof(status_), "pool:notify-invalid");
    return;
  }

  JsonArrayConst arr = params.as<JsonArrayConst>();

  const char* jobId = arr[0] | "job";
  safeCopy(latestJob_.jobId, sizeof(latestJob_.jobId), jobId);

  const char* targetA = arr[6] | nullptr;
  const char* targetB = arr[2] | nullptr;
  const char* selectedTarget = (targetA != nullptr) ? targetA : targetB;
  if (selectedTarget == nullptr) {
    selectedTarget = "0000ffff";
  }

  safeCopy(latestJob_.targetHex, sizeof(latestJob_.targetHex), selectedTarget);
  latestJob_.target32 = parseTarget32(latestJob_.targetHex);
  latestJob_.seed = fastHash32(jobId) ^ fastHash32(latestJob_.targetHex) ^ millis();

  hasValidJob_ = true;
  hasPendingJob_ = true;
  safeCopy(status_, sizeof(status_), "pool:new-job");
}

uint32_t StratumClient::fastHash32(const char* text) {
  if (text == nullptr) {
    return 0;
  }

  uint32_t hash = 2166136261u;
  while (*text != '\0') {
    hash ^= static_cast<uint8_t>(*text++);
    hash *= 16777619u;
  }
  return hash;
}

uint32_t StratumClient::parseTarget32(const char* targetHex) {
  if (targetHex == nullptr || targetHex[0] == '\0') {
    return 0;
  }

  const char* start = targetHex;
  if (start[0] == '0' && (start[1] == 'x' || start[1] == 'X')) {
    start += 2;
  }

  char compact[9];
  memset(compact, 0, sizeof(compact));

  size_t n = 0;
  while (start[n] != '\0' && n < 8) {
    const char c = start[n];
    const bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    if (!isHex) {
      break;
    }
    compact[n] = c;
    n++;
  }

  if (n == 0) {
    return 0;
  }

  return static_cast<uint32_t>(strtoul(compact, nullptr, 16));
}

}  // namespace idk

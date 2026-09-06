#include "wifi_manager.h"

namespace idk {

void WifiManager::begin(const char* ssid, const char* password, uint32_t reconnectIntervalMs) {
  ssid_ = ssid;
  password_ = password;
  reconnectIntervalMs_ = (reconnectIntervalMs == 0) ? 5000 : reconnectIntervalMs;

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  started_ = true;
  startConnect(millis());
}

void WifiManager::loop(uint32_t nowMs) {
  if (!started_ || connected()) {
    return;
  }

  if (nowMs - lastAttemptMs_ >= reconnectIntervalMs_) {
    startConnect(nowMs);
  }
}

bool WifiManager::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

const char* WifiManager::statusText() const {
  switch (WiFi.status()) {
    case WL_CONNECTED:
      return "wifi:connected";
    case WL_CONNECT_FAILED:
      return "wifi:failed";
    case WL_CONNECTION_LOST:
      return "wifi:lost";
    case WL_DISCONNECTED:
      return "wifi:disconnected";
    default:
      return "wifi:connecting";
  }
}

const char* WifiManager::ipText() {
  static char ipBuf[20];
  if (!connected()) {
    strlcpy(ipBuf, "0.0.0.0", sizeof(ipBuf));
    return ipBuf;
  }

  const IPAddress ip = WiFi.localIP();
  snprintf(ipBuf, sizeof(ipBuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  return ipBuf;
}

uint32_t WifiManager::reconnectCount() const {
  return reconnectCount_;
}

void WifiManager::startConnect(uint32_t nowMs) {
  lastAttemptMs_ = nowMs;
  reconnectCount_++;

  if (ssid_ == nullptr || ssid_[0] == '\0') {
    return;
  }

  WiFi.disconnect(false, false);
  WiFi.begin(ssid_, (password_ == nullptr) ? "" : password_);
}

}  // namespace idk

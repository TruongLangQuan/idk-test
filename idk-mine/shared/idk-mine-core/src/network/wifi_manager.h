#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace idk {

class WifiManager {
 public:
  void begin(const char* ssid, const char* password, uint32_t reconnectIntervalMs);
  void loop(uint32_t nowMs);

  bool connected() const;
  const char* statusText() const;
  const char* ipText();
  uint32_t reconnectCount() const;

 private:
  void startConnect(uint32_t nowMs);

  const char* ssid_ = nullptr;
  const char* password_ = nullptr;
  uint32_t reconnectIntervalMs_ = 5000;
  uint32_t lastAttemptMs_ = 0;
  uint32_t reconnectCount_ = 0;
  bool started_ = false;
};

}  // namespace idk

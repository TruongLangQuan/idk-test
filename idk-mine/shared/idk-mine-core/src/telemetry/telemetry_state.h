#pragma once

#include <Arduino.h>

namespace idk {

struct TelemetryState {
  uint64_t totalHash = 0;
  float bestDiff = 0.0f;
  char poolJob[48] = "-";
  char poolTarget[24] = "-";
  uint32_t target32 = 0;
  uint32_t blockFound = 0;
  float currentHashrate = 0.0f;

  bool wifiConnected = false;
  bool poolConnected = false;
  char coin[4] = "BTC";

  uint32_t reconnectCount = 0;
  uint32_t acceptedShares = 0;
  uint32_t rejectedShares = 0;

  char status[64] = "boot";
};

}  // namespace idk

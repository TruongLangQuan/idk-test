#pragma once

namespace idk {

enum class DeviceKind : unsigned char {
  CYD = 0,
  HEADLESS = 1,
};

enum class VariantKind : unsigned char {
  Lottery = 0,
  Mine = 1,
};

struct ProjectProfile {
  const char* projectName;
  const char* minerName;
  const char* wifiSsid;
  const char* wifiPassword;
  const char* defaultConfigJson;
  DeviceKind device;
  VariantKind variant;
  bool enableOtaByDefault;
};

}  // namespace idk

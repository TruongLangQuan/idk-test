#include "project_profile_factory.h"

namespace {

static const char kDefaultConfigJson[] = R"json(
{
  "project_name": "idk-cyd-mine",
  "miner_name": "idk-nagger-5",
  "worker_name": "cyd-mine",
  "default_coin": "LTC",
  "enable_ota": true,
  "pool_password": "x",
  "wallets": {
    "btc": "bc1qul2pvt0l5deykfznzkzhfjf3yj3cla2cssd54e",
    "ltc": "ltc1qcl9h4kffh8w8nr77u8la5ykn4cg59ms2lteply"
  },
  "pool": {
    "btc": {
      "host": "public-pool.io",
      "port": 4333,
      "tls": true
    },
    "ltc": {
      "host": "rx.unmineable.com",
      "port": 443,
      "tls": true
    }
  },
  "timing": {
    "telemetry_ms": 1000,
    "ui_ms": 200,
    "wifi_reconnect_ms": 5000,
    "pool_retry_ms": 4000,
    "keepalive_ms": 30000
  },
  "miner": {
    "threads": 2,
    "batch_size": 1024,
    "lottery_target32": 65535,
    "mine_target32": 4095
  }
}
)json";

}  // namespace

idk::ProjectProfile makeProjectProfile() {
  idk::ProjectProfile profile{};
  profile.projectName = "idk-cyd-mine";
  profile.minerName = "idk-nagger-5";
  profile.wifiSsid = "Quan Le";
  profile.wifiPassword = "15032011";
  profile.defaultConfigJson = kDefaultConfigJson;
  profile.device = idk::DeviceKind::CYD;
  profile.variant = idk::VariantKind::Mine;
  profile.enableOtaByDefault = true;
  return profile;
}

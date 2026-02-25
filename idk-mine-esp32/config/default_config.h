/* ===========================================================================
 * idk-nagger-4 : Default Configuration
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * This file contains default compile-time configuration values.
 * To override these for your own hardware, create config/user_config.h
 * (which is gitignored) and #define the values you want to change
 * BEFORE this file is included.
 * ======================================================================== */

#ifndef DEFAULT_CONFIG_H
#define DEFAULT_CONFIG_H

/* ---- Miner Identity ---------------------------------------------------- */
#ifndef MINER_NAME
#define MINER_NAME              "idk-nagger-4"
#endif

#ifndef MINER_VERSION
#define MINER_VERSION           "0.1.0"
#endif

/* ---- WiFi Credentials -------------------------------------------------- */
/* WARNING: Plaintext WiFi credentials. For production use, move these to
 * NVS encrypted storage or a runtime config.json. See README.md for details. */
#ifndef WIFI_SSID
#define WIFI_SSID               "Quan Le"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD           "15032011"
#endif

/* ---- WiFi Reconnection ------------------------------------------------- */
#ifndef WIFI_MAX_RETRY
#define WIFI_MAX_RETRY          10
#endif

#ifndef WIFI_RETRY_BASE_MS
#define WIFI_RETRY_BASE_MS      1000
#endif

#ifndef WIFI_RETRY_MAX_MS
#define WIFI_RETRY_MAX_MS       60000
#endif

/* ---- Wallet Addresses -------------------------------------------------- */
#ifndef BTC_WALLET_ADDRESS
#define BTC_WALLET_ADDRESS      "bc1qul2pvt0l5deykfznzkzhfjf3yj3cla2cssd54e"
#endif

#ifndef LTC_WALLET_ADDRESS
#define LTC_WALLET_ADDRESS      "ltc1qcl9h4kffh8w8nr77u8la5ykn4cg59ms2lteply"
#endif

/* ---- Mining Mode -------------------------------------------------------- */
/* MINE_MODE_BTC  = 0  (double-SHA256)
 * MINE_MODE_LTC  = 1  (scrypt)
 * MINE_MODE_SIM  = 2  (simulated / test mode) */
#ifndef DEFAULT_MINE_MODE
#define DEFAULT_MINE_MODE       2   /* Default to simulated mode for safety */
#endif

/* ---- Pool Configuration ------------------------------------------------ */
#ifndef POOL_HOST
#define POOL_HOST               "192.168.1.100"  /* Default: local test pool */
#endif

#ifndef POOL_PORT
#define POOL_PORT               3333
#endif

#ifndef POOL_USE_SIMULATED
#define POOL_USE_SIMULATED      1   /* 1 = use built-in simulated pool */
#endif

/* ---- Difficulty --------------------------------------------------------- */
/* Simulated difficulty target — adjustable for testing.
 * Lower = easier to find shares. Value = number of leading zero bits. */
#ifndef SIM_DIFFICULTY_BITS
#define SIM_DIFFICULTY_BITS     16
#endif

/* ---- Logging ------------------------------------------------------------ */
/* LOG_LEVEL: 0=ERROR, 1=WARN, 2=INFO, 3=DEBUG */
#ifndef MINER_LOG_LEVEL
#define MINER_LOG_LEVEL         2   /* INFO by default */
#endif

/* ---- Performance -------------------------------------------------------- */
/* CPU frequency override macro. Set to 160 for thermal tests. */
#ifndef CPU_FREQ_MHZ
#define CPU_FREQ_MHZ            240
#endif

/* Number of miner worker tasks (one per core on dual-core ESP32) */
#ifndef MINER_NUM_WORKERS
#define MINER_NUM_WORKERS       2
#endif

/* Miner task stack size in bytes */
#ifndef MINER_TASK_STACK_SIZE
#define MINER_TASK_STACK_SIZE   8192
#endif

/* ---- Telemetry ---------------------------------------------------------- */
/* Interval in seconds between serial JSON status lines */
#ifndef TELEMETRY_INTERVAL_SEC
#define TELEMETRY_INTERVAL_SEC  10
#endif

/* ---- Scrypt Parameters (LTC) ------------------------------------------- */
/* Full Litecoin scrypt: N=1024, r=1, p=1, keylen=32
 * Requires ~128 KB RAM per instance. If PSRAM is not available,
 * the firmware will use a reduced N value or disable LTC mode. */
#ifndef SCRYPT_N
#define SCRYPT_N                1024
#endif

#ifndef SCRYPT_R
#define SCRYPT_R                1
#endif

#ifndef SCRYPT_P
#define SCRYPT_P                1
#endif

/* ---- Feature Flags ------------------------------------------------------ */
/* Set to 0 to compile out heavy scrypt support (saves RAM on non-PSRAM boards) */
#ifndef FEATURE_SCRYPT_ENABLED
#define FEATURE_SCRYPT_ENABLED  1
#endif

/* OTA update support (stub, for future use) */
#ifndef FEATURE_OTA_ENABLED
#define FEATURE_OTA_ENABLED     0
#endif

/* Web server for dev (disabled by default) */
#ifndef FEATURE_WEBSERVER_ENABLED
#define FEATURE_WEBSERVER_ENABLED 0
#endif

#endif /* DEFAULT_CONFIG_H */

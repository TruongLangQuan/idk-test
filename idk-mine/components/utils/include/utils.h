/* ===========================================================================
 * idk-nagger-4 : Utils — Public API
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Logging macros, configuration structures, JSON config loader, and
 * watchdog helpers for the ESP32 miner firmware.
 * ======================================================================== */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Miner Identity ---------------------------------------------------- */
#define MINER_NAME              "idk-nagger-4"
#define MINER_VERSION           "1.0.0"

/* ---- Default Configuration Values -------------------------------------- */
/* WiFi */
#define DEFAULT_WIFI_SSID             "Quan Le"
#define DEFAULT_WIFI_PASSWORD         "15032011"
#define DEFAULT_WIFI_MAX_RETRY        10
#define DEFAULT_WIFI_TIMEOUT_SEC      30

/* Wallets */
#define DEFAULT_BTC_WALLET   "bc1qul2pvt0l5deykfznzkzhfjf3yj3cla2cssd54e"
#define DEFAULT_LTC_WALLET   "ltc1qcl9h4kffh8w8nr77u8la5ykn4cg59ms2lteply"

/* Pool */
#define DEFAULT_POOL_HOST            "auto"
#define DEFAULT_POOL_PORT            3333
#define DEFAULT_USE_POOL_PROXY       1

/* Mining */
#define DEFAULT_MINE_MODE            0   /* 0=BTC(SHA256d), 1=LTC(scrypt), 2=SIM */
#define DEFAULT_SIM_DIFFICULTY_BITS  16
#define DEFAULT_NUM_WORKERS          2
#define MINER_TASK_STACK_SIZE        8192

/* Telemetry */
#define DEFAULT_TELEMETRY_SEC        10

/* HTTP API */
#define DEFAULT_HTTP_ENABLED         1
#define DEFAULT_HTTP_PORT            8080

/* Scrypt parameters */
#define SCRYPT_N    1024
#define SCRYPT_R    1
#define SCRYPT_P    1

/* Feature flags */
#ifndef FEATURE_SCRYPT_ENABLED
#define FEATURE_SCRYPT_ENABLED  1
#endif

/* ---- Mining mode enum -------------------------------------------------- */
typedef enum {
    MINE_MODE_BTC = 0,  /* SHA-256d (Bitcoin) */
    MINE_MODE_LTC = 1,  /* scrypt (Litecoin)  */
    MINE_MODE_SIM = 2   /* Simulated/test     */
} mine_mode_t;

/* ---- Runtime configuration structure ----------------------------------- */
typedef struct {
    /* Identity */
    char        miner_name[32];
    char        miner_version[16];

    /* WiFi */
    char        wifi_ssid[33];
    char        wifi_password[65];
    uint8_t     wifi_max_retry;
    uint32_t    wifi_timeout_sec;

    /* Wallets */
    char        btc_wallet[64];
    char        ltc_wallet[64];

    /* Pool */
    char        pool_host[128];
    uint16_t    pool_port;
    bool        pool_simulated;
    bool        use_pool_proxy;

    /* Mining */
    mine_mode_t mine_mode;
    uint8_t     sim_difficulty_bits;
    uint8_t     num_workers;

    /* Telemetry */
    uint16_t    telemetry_interval_sec;

    /* HTTP API */
    bool        http_enabled;
    uint16_t    http_port;
    char        http_token[64];

    /* Logging */
    uint8_t     log_level;
} miner_config_t;

/* ---- Mining job structure ---------------------------------------------- */
typedef struct {
    char        job_id[64];
    uint8_t     prev_hash[32];
    uint8_t     merkle_root[32];
    uint32_t    version;
    uint32_t    ntime;
    uint32_t    nbits;
    uint8_t     header[80];
    bool        valid;
} mining_job_t;

/* ---- Mining result structure ------------------------------------------- */
typedef struct {
    bool        found;
    uint32_t    nonce;
    uint8_t     hash[32];
    uint32_t    hashes_done;
} mining_result_t;

/* ---- Miner statistics -------------------------------------------------- */
typedef struct {
    uint64_t    total_hashes;
    uint32_t    shares_found;
    uint32_t    shares_accepted;
    uint32_t    shares_rejected;
    double      hashrate;
    uint32_t    uptime_sec;
    uint8_t     active_workers;
} miner_stats_t;

/* ---- Config API -------------------------------------------------------- */

/** Initialize config with defaults (from compile-time or JSON). */
void config_init(void);

/** Get read-only config pointer. */
const miner_config_t *config_get(void);

/** Get mutable config pointer (for runtime overrides). */
miner_config_t *config_get_mutable(void);

/** Get the active wallet address based on mine_mode. */
const char *config_get_active_wallet(void);

/** Print current config to log. */
void config_print(void);

/* ---- JSON Helpers ------------------------------------------------------ */

/** Simple JSON string extractor (no external deps). */
bool json_get_string(const char *json, const char *key, char *out, size_t outlen);
bool json_get_int(const char *json, const char *key, int *out);
bool json_get_bool(const char *json, const char *key, bool *out);

/* ---- Logging Macros ---------------------------------------------------- */
#ifdef ESP_PLATFORM
#include "esp_log.h"
#define MLOG_I(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define MLOG_W(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#define MLOG_E(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#define MLOG_D(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)
#else
#define MLOG_I(tag, fmt, ...) printf("[INFO][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define MLOG_W(tag, fmt, ...) printf("[WARN][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define MLOG_E(tag, fmt, ...) printf("[ERROR][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define MLOG_D(tag, fmt, ...) /* suppress debug on host */
#endif

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */

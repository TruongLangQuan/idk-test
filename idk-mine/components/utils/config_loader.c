/* ===========================================================================
 * idk-nagger-4 : Config Loader — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Initializes the global miner configuration from compile-time defaults.
 * The JSON config file (config/wifi_and_pool.json) is used as a reference;
 * values are baked in at compile time for ESP32 (no filesystem needed).
 *
 * For runtime JSON loading, see the TODO below for SPIFFS/LittleFS support.
 * ======================================================================== */

#include "utils.h"
#include <string.h>
#include <stdio.h>

/* ---- Global Configuration Instance ------------------------------------- */
static miner_config_t g_config;

void config_init(void)
{
    memset(&g_config, 0, sizeof(g_config));

    /* Identity */
    strncpy(g_config.miner_name, MINER_NAME, sizeof(g_config.miner_name) - 1);
    strncpy(g_config.miner_version, MINER_VERSION, sizeof(g_config.miner_version) - 1);

    /* WiFi */
    strncpy(g_config.wifi_ssid, DEFAULT_WIFI_SSID, sizeof(g_config.wifi_ssid) - 1);
    strncpy(g_config.wifi_password, DEFAULT_WIFI_PASSWORD, sizeof(g_config.wifi_password) - 1);
    g_config.wifi_max_retry = DEFAULT_WIFI_MAX_RETRY;
    g_config.wifi_timeout_sec = DEFAULT_WIFI_TIMEOUT_SEC;

    /* Wallets */
    strncpy(g_config.btc_wallet, DEFAULT_BTC_WALLET, sizeof(g_config.btc_wallet) - 1);
    strncpy(g_config.ltc_wallet, DEFAULT_LTC_WALLET, sizeof(g_config.ltc_wallet) - 1);

    /* Pool */
    strncpy(g_config.pool_host, DEFAULT_POOL_HOST, sizeof(g_config.pool_host) - 1);
    g_config.pool_port = DEFAULT_POOL_PORT;
    g_config.pool_simulated = (DEFAULT_USE_POOL_PROXY != 0);
    g_config.use_pool_proxy = (DEFAULT_USE_POOL_PROXY != 0);

    /* Mining */
    g_config.mine_mode = (mine_mode_t)DEFAULT_MINE_MODE;
    g_config.sim_difficulty_bits = DEFAULT_SIM_DIFFICULTY_BITS;
    g_config.num_workers = DEFAULT_NUM_WORKERS;

    /* Telemetry */
    g_config.telemetry_interval_sec = DEFAULT_TELEMETRY_SEC;

    /* HTTP API */
    g_config.http_enabled = (DEFAULT_HTTP_ENABLED != 0);
    g_config.http_port = DEFAULT_HTTP_PORT;
    strncpy(g_config.http_token, "generate_secure_token_on_first_boot",
            sizeof(g_config.http_token) - 1);

    /* Logging */
    g_config.log_level = 2; /* INFO */
}

const miner_config_t *config_get(void)
{
    return &g_config;
}

miner_config_t *config_get_mutable(void)
{
    return &g_config;
}

const char *config_get_active_wallet(void)
{
    switch (g_config.mine_mode) {
        case MINE_MODE_LTC:
            return g_config.ltc_wallet;
        case MINE_MODE_BTC:
        case MINE_MODE_SIM:
        default:
            return g_config.btc_wallet;
    }
}

void config_print(void)
{
    const char *mode_str;
    switch (g_config.mine_mode) {
        case MINE_MODE_BTC: mode_str = "BTC (SHA256d)"; break;
        case MINE_MODE_LTC: mode_str = "LTC (scrypt)";  break;
        case MINE_MODE_SIM: mode_str = "Simulated";      break;
        default:            mode_str = "Unknown";         break;
    }

    MLOG_I("CONFIG", "===== %s v%s Configuration =====",
             g_config.miner_name, g_config.miner_version);
    MLOG_I("CONFIG", "  WiFi SSID      : %s", g_config.wifi_ssid);
    MLOG_I("CONFIG", "  Mine mode      : %s", mode_str);
    MLOG_I("CONFIG", "  Pool           : %s:%u (proxy=%d)",
             g_config.pool_host, g_config.pool_port, g_config.use_pool_proxy);
    MLOG_I("CONFIG", "  Wallet (active): %s", config_get_active_wallet());
    MLOG_I("CONFIG", "  Workers        : %u", g_config.num_workers);
    MLOG_I("CONFIG", "  HTTP API       : %s (port %u)",
             g_config.http_enabled ? "ON" : "OFF", g_config.http_port);
    MLOG_I("CONFIG", "  Difficulty bits : %u", g_config.sim_difficulty_bits);
}

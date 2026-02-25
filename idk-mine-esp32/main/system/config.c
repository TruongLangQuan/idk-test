/* ===========================================================================
 * idk-nagger-4 : System Configuration — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 * ======================================================================== */

#include "config.h"
#include <string.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "CONFIG";
#else
/* Host build stubs */
#define ESP_LOGI(tag, fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#endif

/* ---- Global Configuration Instance ------------------------------------- */
static miner_config_t g_config;

/* ---- config_init ------------------------------------------------------- */
void config_init(void)
{
    memset(&g_config, 0, sizeof(g_config));

    /* Identity */
    strncpy(g_config.miner_name, MINER_NAME, sizeof(g_config.miner_name) - 1);
    strncpy(g_config.miner_version, MINER_VERSION, sizeof(g_config.miner_version) - 1);

    /* WiFi */
    strncpy(g_config.wifi_ssid, WIFI_SSID, sizeof(g_config.wifi_ssid) - 1);
    strncpy(g_config.wifi_password, WIFI_PASSWORD, sizeof(g_config.wifi_password) - 1);
    g_config.wifi_max_retry = WIFI_MAX_RETRY;

    /* Wallets */
    strncpy(g_config.btc_wallet, BTC_WALLET_ADDRESS, sizeof(g_config.btc_wallet) - 1);
    strncpy(g_config.ltc_wallet, LTC_WALLET_ADDRESS, sizeof(g_config.ltc_wallet) - 1);

    /* Pool */
    strncpy(g_config.pool_host, POOL_HOST, sizeof(g_config.pool_host) - 1);
    g_config.pool_port       = POOL_PORT;
    g_config.pool_simulated  = (POOL_USE_SIMULATED != 0);

    /* Mining */
    g_config.mine_mode           = (mine_mode_t)DEFAULT_MINE_MODE;
    g_config.sim_difficulty_bits = SIM_DIFFICULTY_BITS;
    g_config.num_workers         = MINER_NUM_WORKERS;

    /* Logging */
    g_config.log_level = MINER_LOG_LEVEL;

    /* Telemetry */
    g_config.telemetry_interval_sec = TELEMETRY_INTERVAL_SEC;
}

/* ---- config_get -------------------------------------------------------- */
const miner_config_t* config_get(void)
{
    return &g_config;
}

/* ---- config_get_mutable ------------------------------------------------ */
miner_config_t* config_get_mutable(void)
{
    return &g_config;
}

/* ---- config_get_active_wallet ------------------------------------------ */
const char* config_get_active_wallet(void)
{
    switch (g_config.mine_mode) {
        case MINE_MODE_BTC:
        case MINE_MODE_SIM:
            return g_config.btc_wallet;
        case MINE_MODE_LTC:
            return g_config.ltc_wallet;
        default:
            return g_config.btc_wallet;
    }
}

/* ---- config_print ------------------------------------------------------ */
void config_print(void)
{
    const char *mode_str;
    switch (g_config.mine_mode) {
        case MINE_MODE_BTC: mode_str = "BTC (SHA256d)"; break;
        case MINE_MODE_LTC: mode_str = "LTC (scrypt)";  break;
        case MINE_MODE_SIM: mode_str = "Simulated";      break;
        default:            mode_str = "Unknown";         break;
    }

    ESP_LOGI(TAG, "===== %s v%s Configuration =====",
             g_config.miner_name, g_config.miner_version);
    ESP_LOGI(TAG, "  WiFi SSID      : %s", g_config.wifi_ssid);
    ESP_LOGI(TAG, "  Mine mode      : %s", mode_str);
    ESP_LOGI(TAG, "  Pool           : %s:%u (sim=%d)",
             g_config.pool_host, g_config.pool_port, g_config.pool_simulated);
    ESP_LOGI(TAG, "  Wallet (active): %s", config_get_active_wallet());
    ESP_LOGI(TAG, "  Workers        : %u", g_config.num_workers);
    ESP_LOGI(TAG, "  Log level      : %u", g_config.log_level);
    ESP_LOGI(TAG, "  Difficulty bits : %u", g_config.sim_difficulty_bits);
}

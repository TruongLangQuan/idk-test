/* ===========================================================================
 * idk-nagger-4 : System Configuration — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Central configuration manager. Includes compile-time defaults and provides
 * a runtime configuration structure that can be modified via serial or
 * config.json at boot.
 * ======================================================================== */

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* Include user overrides first (if they exist), then defaults */
#if __has_include("user_config.h")
#include "user_config.h"
#endif
#include "default_config.h"

/* ---- Mining Mode Enum -------------------------------------------------- */
typedef enum {
    MINE_MODE_BTC = 0,   /* Bitcoin: double-SHA256 */
    MINE_MODE_LTC = 1,   /* Litecoin: scrypt      */
    MINE_MODE_SIM = 2    /* Simulated / test mode  */
} mine_mode_t;

/* ---- Runtime Configuration Structure ----------------------------------- */
typedef struct {
    /* Miner identity */
    char        miner_name[32];
    char        miner_version[16];

    /* WiFi */
    char        wifi_ssid[64];
    char        wifi_password[64];
    uint8_t     wifi_max_retry;

    /* Wallet addresses */
    char        btc_wallet[128];
    char        ltc_wallet[128];

    /* Pool settings */
    char        pool_host[128];
    uint16_t    pool_port;
    bool        pool_simulated;

    /* Mining */
    mine_mode_t mine_mode;
    uint8_t     sim_difficulty_bits;
    uint8_t     num_workers;

    /* Logging */
    uint8_t     log_level;  /* 0=ERROR, 1=WARN, 2=INFO, 3=DEBUG */

    /* Telemetry */
    uint16_t    telemetry_interval_sec;
} miner_config_t;

/* ---- Public API -------------------------------------------------------- */

/**
 * @brief Initialize the global configuration from compile-time defaults.
 *        Call this once at startup before any other subsystem init.
 */
void config_init(void);

/**
 * @brief Get a pointer to the current runtime configuration (read-only).
 * @return Pointer to the global miner_config_t.
 */
const miner_config_t* config_get(void);

/**
 * @brief Get a mutable pointer (for runtime reconfiguration).
 * @return Pointer to modifiable config.
 */
miner_config_t* config_get_mutable(void);

/**
 * @brief Print the current configuration to serial (at INFO level).
 */
void config_print(void);

/**
 * @brief Return the active wallet address for the current mining mode.
 * @return Pointer to the wallet address string.
 */
const char* config_get_active_wallet(void);

#endif /* SYSTEM_CONFIG_H */

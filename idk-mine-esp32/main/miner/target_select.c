/* ===========================================================================
 * idk-nagger-4 : Target Select — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 * ======================================================================== */

#include "target_select.h"
#include "../crypto/sha256_opt.h"
#include "../crypto/scrypt_wrapper.h"
#include <string.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "TARGET";
#define LOG_I(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...) printf("[INFO][TARGET] " fmt "\n", ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR][TARGET] " fmt "\n", ##__VA_ARGS__)
#endif

static mine_mode_t s_current_mode = MINE_MODE_SIM;
static uint32_t    s_scrypt_n = SCRYPT_N;

bool target_select_init(mine_mode_t mode)
{
    s_current_mode = mode;

    switch (mode) {
        case MINE_MODE_BTC:
            LOG_I("Algorithm: double-SHA256 (Bitcoin)");
            return true;

        case MINE_MODE_LTC:
#if FEATURE_SCRYPT_ENABLED
            /* Check if full scrypt is feasible */
            if (!scrypt_check_memory(SCRYPT_N, SCRYPT_R)) {
                s_scrypt_n = scrypt_max_feasible_n();
                if (s_scrypt_n < 2) {
                    LOG_E("LTC mode: insufficient memory for scrypt (need PSRAM)");
                    LOG_E("Falling back to simulated mode");
                    s_current_mode = MINE_MODE_SIM;
                    return true;
                }
                LOG_I("LTC mode: reduced scrypt N from %u to %u (memory limit)",
                      (unsigned)SCRYPT_N, (unsigned)s_scrypt_n);
            } else {
                s_scrypt_n = SCRYPT_N;
                LOG_I("Algorithm: scrypt (Litecoin) N=%u r=%u p=%u",
                      (unsigned)SCRYPT_N, SCRYPT_R, SCRYPT_P);
            }
            return true;
#else
            LOG_E("LTC mode: scrypt compiled out (FEATURE_SCRYPT_ENABLED=0)");
            LOG_E("Falling back to simulated mode");
            s_current_mode = MINE_MODE_SIM;
            return true;
#endif

        case MINE_MODE_SIM:
            LOG_I("Algorithm: double-SHA256 (Simulated mode)");
            return true;

        default:
            LOG_E("Unknown mining mode %d", mode);
            return false;
    }
}

bool target_select_hash(const uint8_t *header, uint8_t *hash)
{
    switch (s_current_mode) {
        case MINE_MODE_BTC:
        case MINE_MODE_SIM:
            sha256d_compute(header, 80, hash);
            return true;

        case MINE_MODE_LTC:
#if FEATURE_SCRYPT_ENABLED
        {
            scrypt_err_t err = scrypt_compute(
                header, 80,
                header, 80,
                s_scrypt_n, SCRYPT_R, SCRYPT_P,
                hash, 32
            );
            return (err == SCRYPT_OK);
        }
#else
            return false;
#endif

        default:
            return false;
    }
}

const char* target_select_algo_name(void)
{
    switch (s_current_mode) {
        case MINE_MODE_BTC: return "SHA256d";
        case MINE_MODE_LTC: return "scrypt";
        case MINE_MODE_SIM: return "SHA256d (sim)";
        default:            return "unknown";
    }
}

mine_mode_t target_select_get_mode(void)
{
    return s_current_mode;
}

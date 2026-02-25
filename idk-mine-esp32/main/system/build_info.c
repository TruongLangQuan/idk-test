/* ===========================================================================
 * idk-nagger-4 : Build Information
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Provides compile-time metadata (git commit, build date) injected
 * by CMakeLists.txt via -D flags. Used for telemetry and debugging.
 * ======================================================================== */

#include <stdio.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "BUILD";
#else
#define ESP_LOGI(tag, fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#endif

/* These are defined by CMakeLists.txt at compile time */
#ifndef BUILD_GIT_COMMIT
#define BUILD_GIT_COMMIT "unknown"
#endif

#ifndef BUILD_DATE
#define BUILD_DATE "unknown"
#endif

/* ---- Public accessors -------------------------------------------------- */

const char* build_info_get_commit(void)
{
    return BUILD_GIT_COMMIT;
}

const char* build_info_get_date(void)
{
    return BUILD_DATE;
}

const char* build_info_get_miner_name(void)
{
    return "idk-nagger-4";
}

void build_info_print(void)
{
    ESP_LOGI(TAG, "======================================");
    ESP_LOGI(TAG, "  idk-nagger-4 Lottery Miner");
    ESP_LOGI(TAG, "  Git commit : %s", BUILD_GIT_COMMIT);
    ESP_LOGI(TAG, "  Build date : %s", BUILD_DATE);
    ESP_LOGI(TAG, "======================================");
}

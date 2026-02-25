/* ===========================================================================
 * idk-nagger-4 : Application Entry Point
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Firmware entry point for the ESP32-WROOM-32U lottery miner.
 *
 * Boot sequence:
 *   1. Initialize logging and print build info
 *   2. Load configuration from compile-time defaults
 *   3. Run crypto self-tests (SHA-256, scrypt)
 *   4. Connect to WiFi with exponential backoff
 *   5. Initialize miner core (target selector, protocol adapter)
 *   6. Start mining workers
 *   7. Enter main loop (telemetry + watchdog)
 *
 * Tested with: esp-idf v5.2.2
 * Target: ESP32-WROOM-32U (Xtensa dual-core LX6, 240 MHz)
 * ======================================================================== */

#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_heap_caps.h"
#endif

#include "system/config.h"
#include "system/flash_writer.h"
#include "crypto/sha256_opt.h"
#include "crypto/scrypt_wrapper.h"
#include "net/wifi_manager.h"
#include "miner/miner_core.h"
#include "miner/target_select.h"

/* Forward declarations */
extern void build_info_print(void);
extern const char* build_info_get_commit(void);
extern const char* build_info_get_date(void);

#ifdef ESP_PLATFORM

static const char *TAG = "MAIN";

/* ---- app_main: firmware entry point ------------------------------------ */
void app_main(void)
{
    /* ================================================================
     * STEP 1: Banner and build info
     * ================================================================ */
    printf("\n\n");
    build_info_print();
    printf("\n");

    /* ================================================================
     * STEP 2: Initialize configuration
     * ================================================================ */
    config_init();
    config_print();

    const miner_config_t *cfg = config_get();

    /* ================================================================
     * STEP 3: System info
     * ================================================================ */
    ESP_LOGI(TAG, "Free heap: %"PRIu32" bytes", (uint32_t)esp_get_free_heap_size());
    ESP_LOGI(TAG, "Min free heap: %"PRIu32" bytes", (uint32_t)esp_get_minimum_free_heap_size());

#if CONFIG_ESP32_SPIRAM_SUPPORT
    size_t psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "PSRAM: %zu bytes", psram);
#else
    ESP_LOGI(TAG, "PSRAM: not enabled");
#endif

    /* Print partition table */
    flash_writer_print_partitions();

    /* ================================================================
     * STEP 4: Crypto self-tests
     * ================================================================ */
    ESP_LOGI(TAG, "Running crypto self-tests...");
    if (sha256_self_test() != 0) {
        ESP_LOGE(TAG, "SHA-256 self-test FAILED! Halting.");
        while (1) { vTaskDelay(pdMS_TO_TICKS(10000)); }
    }

#if FEATURE_SCRYPT_ENABLED
    if (scrypt_self_test() != 0) {
        ESP_LOGW(TAG, "Scrypt self-test failed (non-fatal, LTC mode may not work)");
    }
#endif

    /* ================================================================
     * STEP 5: Connect to WiFi
     * ================================================================ */
    ESP_LOGI(TAG, "Connecting to WiFi '%s'...", cfg->wifi_ssid);
    bool wifi_ok = wifi_manager_init(
        cfg->wifi_ssid,
        cfg->wifi_password,
        cfg->wifi_max_retry
    );

    if (wifi_ok) {
        char ip[16];
        wifi_manager_get_ip(ip, sizeof(ip));
        ESP_LOGI(TAG, "WiFi connected. IP: %s", ip);
    } else {
        ESP_LOGW(TAG, "WiFi not connected. Mining in offline/simulated mode.");
        /* Force simulated mode if WiFi failed and not already simulated */
        if (!cfg->pool_simulated) {
            miner_config_t *mcfg = config_get_mutable();
            mcfg->pool_simulated = true;
            ESP_LOGW(TAG, "Switched to simulated pool (no network)");
        }
    }

    /* ================================================================
     * STEP 6: Initialize miner core
     * ================================================================ */
    ESP_LOGI(TAG, "Initializing miner core...");
    if (!miner_core_init()) {
        ESP_LOGE(TAG, "Miner core init failed! Restarting in 10s...");
        vTaskDelay(pdMS_TO_TICKS(10000));
        esp_restart();
    }

    /* ================================================================
     * STEP 7: Start mining
     * ================================================================ */
    ESP_LOGI(TAG, "====================================");
    ESP_LOGI(TAG, "  %s is GO!", cfg->miner_name);
    ESP_LOGI(TAG, "  Mode: %s", target_select_algo_name());
    ESP_LOGI(TAG, "  Workers: %u", cfg->num_workers);
    ESP_LOGI(TAG, "  Wallet: %s", config_get_active_wallet());
    ESP_LOGI(TAG, "====================================");

    miner_core_start();

    /* ================================================================
     * STEP 8: Main loop — keep alive, monitor health
     * ================================================================ */
    uint32_t loop_count = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));

        /* Periodic health check */
        loop_count++;
        if (loop_count % 12 == 0) { /* Every ~60 seconds */
            ESP_LOGI(TAG, "Health: heap=%"PRIu32" min_heap=%"PRIu32" wifi=%s",
                     (uint32_t)esp_get_free_heap_size(),
                     (uint32_t)esp_get_minimum_free_heap_size(),
                     wifi_manager_is_connected() ? "OK" : "DISCONNECTED");
        }

        /* Check WiFi status */
        if (!wifi_manager_is_connected() && !cfg->pool_simulated) {
            ESP_LOGW(TAG, "WiFi lost — miner continues with cached job");
        }
    }
}

#else
/* ---- Host build: simple main() for testing ----------------------------- */
#include <stdlib.h>

int main(int argc, char *argv[])
{
    printf("\n=== idk-nagger-4 (Host Build) ===\n\n");

    config_init();
    config_print();

    printf("\nRunning crypto self-tests...\n");
    if (sha256_self_test() != 0) {
        printf("SHA-256 self-test FAILED!\n");
        return 1;
    }

#if FEATURE_SCRYPT_ENABLED
    scrypt_self_test();
#endif

    printf("\nRunning SHA-256 benchmark (100k hashes)...\n");
    sha256_benchmark(100000);

    printf("\nHost build complete. Use esp-idf to build for ESP32.\n");
    return 0;
}

#endif /* ESP_PLATFORM */

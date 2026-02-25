/* ===========================================================================
 * idk-nagger-4 : Miner Core — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Worker Architecture:
 *   - A "job manager" task fetches jobs from the pool via protocol_adapter.
 *   - N "worker" tasks (pinned to different CPU cores) mine nonce ranges.
 *   - Workers split the 32-bit nonce space evenly.
 *   - When a share is found, the worker submits it through protocol_adapter.
 *   - A "telemetry" task periodically prints JSON status to serial.
 *
 * Rate limiting:
 *   - Jobs are rotated every ~30 seconds or when a share is found.
 *   - At most 1 submit per second to avoid flooding the pool.
 *
 * Watchdog:
 *   - Workers reset the task watchdog to prevent WDT resets.
 * ======================================================================== */

#include "miner_core.h"
#include "lottery.h"
#include "target_select.h"
#include "protocol_adapter.h"
#include "../system/config.h"
#include <string.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"

static const char *TAG = "MINER";

/* ---- State ------------------------------------------------------------- */
static volatile bool     s_running = false;
static mining_job_t      s_current_job;
static SemaphoreHandle_t s_job_mutex;
static volatile uint64_t s_total_hashes = 0;
static volatile uint32_t s_shares_found = 0;
static int64_t           s_start_time = 0;
static TaskHandle_t      s_worker_handles[4]; /* max 4 workers */
static TaskHandle_t      s_telemetry_handle = NULL;

/* Batch size: number of nonces per inner loop iteration.
 * Larger = less overhead but less responsive to stop signals. */
#define NONCE_BATCH_SIZE 10000

/* Job refresh interval */
#define JOB_REFRESH_MS  30000

/* ---- Worker Task ------------------------------------------------------- */
typedef struct {
    uint8_t  worker_id;
    uint32_t nonce_start;
    uint32_t nonce_stride;
} worker_params_t;

static void miner_worker_task(void *arg)
{
    worker_params_t *params = (worker_params_t *)arg;
    uint8_t header[80];
    uint32_t nonce = params->nonce_start;
    const miner_config_t *cfg = config_get();

    ESP_LOGI(TAG, "Worker %u started (nonce_start=0x%08x, stride=%u)",
             params->worker_id, (unsigned)nonce, (unsigned)params->nonce_stride);

    /* Register with task watchdog */
    esp_task_wdt_add(NULL);

    while (s_running) {
        /* Copy current job header (under mutex) */
        if (xSemaphoreTake(s_job_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (!s_current_job.valid) {
                xSemaphoreGive(s_job_mutex);
                vTaskDelay(pdMS_TO_TICKS(500));
                continue;
            }
            memcpy(header, s_current_job.header, 80);
            xSemaphoreGive(s_job_mutex);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        /* Mine a batch of nonces */
        lottery_result_t result;
        lottery_mine_batch(header, nonce, NONCE_BATCH_SIZE,
                          cfg->sim_difficulty_bits, &result);

        s_total_hashes += result.hashes_done;

        if (result.found) {
            s_shares_found++;
            ESP_LOGI(TAG, "Worker %u found share! nonce=0x%08x",
                     params->worker_id, (unsigned)result.nonce);

            /* Submit through protocol adapter */
            if (xSemaphoreTake(s_job_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                protocol_adapter_submit(&s_current_job, result.nonce);
                xSemaphoreGive(s_job_mutex);
            }
        }

        /* Advance nonce by stride (each worker has its own range) */
        nonce += params->nonce_stride;

        /* Feed the watchdog */
        esp_task_wdt_reset();

        /* Yield briefly to prevent starving other tasks */
        taskYIELD();
    }

    esp_task_wdt_delete(NULL);
    ESP_LOGI(TAG, "Worker %u stopped", params->worker_id);
    free(params);
    vTaskDelete(NULL);
}

/* ---- Telemetry Task ---------------------------------------------------- */
static void telemetry_task(void *arg)
{
    (void)arg;
    const miner_config_t *cfg = config_get();

    while (s_running) {
        vTaskDelay(pdMS_TO_TICKS(cfg->telemetry_interval_sec * 1000));
        if (s_running) {
            miner_core_print_telemetry();
        }
    }
    vTaskDelete(NULL);
}

/* ---- Job Manager (runs in main context or separate task) --------------- */
static void job_manager_task(void *arg)
{
    (void)arg;

    while (s_running) {
        mining_job_t new_job;
        if (protocol_adapter_get_job(&new_job)) {
            if (xSemaphoreTake(s_job_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                memcpy(&s_current_job, &new_job, sizeof(mining_job_t));
                xSemaphoreGive(s_job_mutex);
                ESP_LOGI(TAG, "New job: %s", new_job.job_id);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(JOB_REFRESH_MS));
    }
    vTaskDelete(NULL);
}

/* ---- Public API -------------------------------------------------------- */

bool miner_core_init(void)
{
    s_job_mutex = xSemaphoreCreateMutex();
    if (!s_job_mutex) {
        ESP_LOGE(TAG, "Failed to create job mutex");
        return false;
    }

    memset(&s_current_job, 0, sizeof(s_current_job));
    s_total_hashes = 0;
    s_shares_found = 0;
    s_running = false;

    /* Initialize target selector */
    const miner_config_t *cfg = config_get();
    if (!target_select_init(cfg->mine_mode)) {
        ESP_LOGE(TAG, "Failed to initialize target selector");
        return false;
    }

    /* Initialize protocol adapter (pool connection) */
    if (!protocol_adapter_init()) {
        ESP_LOGW(TAG, "Pool connection failed; will retry on start");
    }

    ESP_LOGI(TAG, "Miner core initialized");
    return true;
}

void miner_core_start(void)
{
    if (s_running) return;

    s_running = true;
    s_start_time = esp_timer_get_time();

    const miner_config_t *cfg = config_get();
    uint8_t num_workers = cfg->num_workers;
    if (num_workers > 4) num_workers = 4;

    ESP_LOGI(TAG, "Starting %u worker(s), algo=%s, difficulty=%u bits",
             num_workers, target_select_algo_name(), cfg->sim_difficulty_bits);

    /* Fetch initial job */
    mining_job_t initial_job;
    if (protocol_adapter_get_job(&initial_job)) {
        memcpy(&s_current_job, &initial_job, sizeof(mining_job_t));
    }

    /* Spawn worker tasks */
    for (uint8_t i = 0; i < num_workers; i++) {
        worker_params_t *params = malloc(sizeof(worker_params_t));
        if (!params) {
            ESP_LOGE(TAG, "Failed to allocate worker params");
            continue;
        }
        params->worker_id = i;
        params->nonce_start = (uint32_t)i * (0xFFFFFFFF / num_workers);
        params->nonce_stride = NONCE_BATCH_SIZE * num_workers;

        char task_name[16];
        snprintf(task_name, sizeof(task_name), "miner_w%u", i);

        BaseType_t ret = xTaskCreatePinnedToCore(
            miner_worker_task,
            task_name,
            cfg->num_workers > 0 ? MINER_TASK_STACK_SIZE : 4096,
            params,
            5,  /* Priority: above idle, below system */
            &s_worker_handles[i],
            i % 2  /* Pin to core 0 or 1 */
        );

        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create worker task %u", i);
            free(params);
        }
    }

    /* Start job manager */
    xTaskCreate(job_manager_task, "job_mgr", 4096, NULL, 4, NULL);

    /* Start telemetry */
    xTaskCreate(telemetry_task, "telemetry", 4096, NULL, 3, &s_telemetry_handle);

    ESP_LOGI(TAG, "Mining started!");
}

void miner_core_stop(void)
{
    ESP_LOGI(TAG, "Stopping miner...");
    s_running = false;
    /* Workers will self-terminate when they see s_running == false */
    vTaskDelay(pdMS_TO_TICKS(2000)); /* Give workers time to exit */
    protocol_adapter_shutdown();
    ESP_LOGI(TAG, "Miner stopped");
}

void miner_core_get_stats(miner_stats_t *stats)
{
    memset(stats, 0, sizeof(*stats));

    int64_t now = esp_timer_get_time();
    double elapsed_sec = (double)(now - s_start_time) / 1000000.0;
    if (elapsed_sec < 0.001) elapsed_sec = 0.001;

    stats->total_hashes = s_total_hashes;
    stats->shares_found = s_shares_found;
    stats->hashrate = (double)s_total_hashes / elapsed_sec;
    stats->uptime_sec = (uint32_t)elapsed_sec;
    stats->active_workers = config_get()->num_workers;

    protocol_adapter_get_stats(
        &stats->shares_accepted,
        &stats->shares_rejected,
        NULL
    );
}

void miner_core_print_telemetry(void)
{
    miner_stats_t stats;
    miner_core_get_stats(&stats);

    /* Print compact JSON for serial telemetry */
    printf("{\"miner\":\"%s\",\"algo\":\"%s\",\"hashrate\":%.1f,"
           "\"hashes\":%llu,\"found\":%u,\"accepted\":%u,"
           "\"rejected\":%u,\"uptime\":%u}\n",
           MINER_NAME,
           target_select_algo_name(),
           stats.hashrate,
           (unsigned long long)stats.total_hashes,
           (unsigned)stats.shares_found,
           (unsigned)stats.shares_accepted,
           (unsigned)stats.shares_rejected,
           (unsigned)stats.uptime_sec);
}

bool miner_core_is_running(void)
{
    return s_running;
}

#else
/* ---- Host stubs for compilation ---------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <time.h>

static int s_running = 0;

bool miner_core_init(void) {
    printf("[MINER] Core initialized (host stub)\n");
    return true;
}
void miner_core_start(void) {
    s_running = 1;
    printf("[MINER] Mining started (host stub - no workers)\n");
}
void miner_core_stop(void) {
    s_running = 0;
    printf("[MINER] Mining stopped (host stub)\n");
}
void miner_core_get_stats(miner_stats_t *stats) {
    memset(stats, 0, sizeof(*stats));
}
void miner_core_print_telemetry(void) {
    printf("{\"miner\":\"idk-nagger-4\",\"hashrate\":0,\"hashes\":0,"
           "\"accepted\":0,\"rejected\":0,\"uptime\":0}\n");
}
bool miner_core_is_running(void) { return s_running; }

#endif /* ESP_PLATFORM */

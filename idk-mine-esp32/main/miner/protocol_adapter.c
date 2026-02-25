/* ===========================================================================
 * idk-nagger-4 : Protocol Adapter — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 * ======================================================================== */

#include "protocol_adapter.h"
#include "../system/config.h"
#include <string.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "PROTO";
#define LOG_I(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...) printf("[INFO][PROTO] " fmt "\n", ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR][PROTO] " fmt "\n", ##__VA_ARGS__)
#endif

static pool_client_t s_pool;

bool protocol_adapter_init(void)
{
    const miner_config_t *cfg = config_get();

    pool_client_init(&s_pool, cfg->pool_host, cfg->pool_port, cfg->pool_simulated);

    const char *wallet = config_get_active_wallet();
    bool ok = pool_client_connect(&s_pool, wallet, cfg->miner_name);

    if (!ok) {
        LOG_E("Failed to connect to pool %s:%u", cfg->pool_host, cfg->pool_port);
        return false;
    }

    LOG_I("Protocol adapter initialized (simulated=%d)", cfg->pool_simulated);
    return true;
}

bool protocol_adapter_get_job(mining_job_t *job)
{
    return pool_client_get_job(&s_pool, job);
}

bool protocol_adapter_submit(const mining_job_t *job, uint32_t nonce)
{
    return pool_client_submit_share(&s_pool, job, nonce);
}

void protocol_adapter_get_stats(uint32_t *accepted, uint32_t *rejected,
                                uint32_t *submitted)
{
    if (accepted)  *accepted  = s_pool.shares_accepted;
    if (rejected)  *rejected  = s_pool.shares_rejected;
    if (submitted) *submitted = s_pool.shares_submitted;
}

bool protocol_adapter_is_connected(void)
{
    return s_pool.connected;
}

void protocol_adapter_shutdown(void)
{
    pool_client_disconnect(&s_pool);
    LOG_I("Protocol adapter shut down");
}

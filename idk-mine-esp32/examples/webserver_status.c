/* ===========================================================================
 * idk-nagger-4 : Example Web Server (Disabled by Default)
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * A minimal HTTP status page for development. Disabled by default
 * (FEATURE_WEBSERVER_ENABLED=0). To enable:
 *   1. Set FEATURE_WEBSERVER_ENABLED to 1 in your config
 *   2. Add a call to webserver_start() after WiFi connects in app_main.c
 *
 * Provides a single endpoint:
 *   GET /status → JSON with miner stats
 *
 * WARNING: No authentication. Do not expose to public networks.
 * ======================================================================== */

#ifdef ESP_PLATFORM
#if FEATURE_WEBSERVER_ENABLED

#include "esp_http_server.h"
#include "esp_log.h"
#include "../main/miner/miner_core.h"
#include "../main/miner/target_select.h"
#include <stdio.h>

static const char *TAG = "WEBSERVER";
static httpd_handle_t s_server = NULL;

/* GET /status handler */
static esp_err_t status_handler(httpd_req_t *req)
{
    miner_stats_t stats;
    miner_core_get_stats(&stats);

    char json[512];
    snprintf(json, sizeof(json),
             "{"
             "\"miner\":\"idk-nagger-4\","
             "\"algo\":\"%s\","
             "\"hashrate\":%.1f,"
             "\"hashes\":%llu,"
             "\"accepted\":%u,"
             "\"rejected\":%u,"
             "\"uptime\":%u,"
             "\"running\":%s"
             "}",
             target_select_algo_name(),
             stats.hashrate,
             (unsigned long long)stats.total_hashes,
             (unsigned)stats.shares_accepted,
             (unsigned)stats.shares_rejected,
             (unsigned)stats.uptime_sec,
             miner_core_is_running() ? "true" : "false"
    );

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, -1);
    return ESP_OK;
}

void webserver_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    if (httpd_start(&s_server, &config) == ESP_OK) {
        httpd_uri_t status_uri = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = status_handler,
        };
        httpd_register_uri_handler(s_server, &status_uri);
        ESP_LOGI(TAG, "Web server started on port 80");
        ESP_LOGI(TAG, "  GET /status → miner stats JSON");
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
    }
}

void webserver_stop(void)
{
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }
}

#endif /* FEATURE_WEBSERVER_ENABLED */
#endif /* ESP_PLATFORM */

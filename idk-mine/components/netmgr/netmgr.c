/* ===========================================================================
 * idk-nagger-4 : Network Manager — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * WiFi STA mode with event-driven reconnection and exponential backoff.
 * Includes fallback SoftAP mode for initial configuration.
 * ======================================================================== */

#include "netmgr.h"
#include <string.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
/* ---- ESP32 Implementation ---------------------------------------------- */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

static const char *TAG = "NETMGR";

/* Event group bits */
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static EventGroupHandle_t s_wifi_events;
static uint8_t  s_max_retry = 10;
static uint8_t  s_retry_count = 0;
static bool     s_connected = false;
static char     s_ip_str[16] = {0};
static bool     s_initialized = false;

/* ---- Event handler ----------------------------------------------------- */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA started, connecting...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_connected = false;
        if (s_retry_count < s_max_retry) {
            /* Exponential backoff: base * 2^retry, capped at max */
            uint32_t delay_ms = NETMGR_RETRY_BASE_MS * (1u << s_retry_count);
            if (delay_ms > NETMGR_RETRY_MAX_MS) delay_ms = NETMGR_RETRY_MAX_MS;

            ESP_LOGW(TAG, "Disconnected. Retry %d/%d in %"PRIu32"ms...",
                     s_retry_count + 1, s_max_retry, delay_ms);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
            s_retry_count++;
            esp_wifi_connect();
        } else {
            ESP_LOGE(TAG, "Max retries exhausted. Will retry in background.");
            xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);

            /* Background retry — try every 60 seconds */
            vTaskDelay(pdMS_TO_TICKS(60000));
            s_retry_count = 0;
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(s_ip_str, sizeof(s_ip_str), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Connected! IP: %s", s_ip_str);
        s_connected = true;
        s_retry_count = 0;
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    }
}

/* ---- Initialize NVS (required by WiFi driver) -------------------------- */
static void ensure_nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS and re-initializing");
        nvs_flash_erase();
        nvs_flash_init();
    }
}

/* ---- Public API -------------------------------------------------------- */

bool netmgr_wifi_init(const char *ssid, const char *password, uint8_t max_retry)
{
    s_max_retry = max_retry;
    s_retry_count = 0;
    s_connected = false;

    ensure_nvs_init();

    s_wifi_events = xEventGroupCreate();

    /* Network interface init (only once) */
    if (!s_initialized) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        s_initialized = true;
    }

    esp_netif_create_default_wifi_sta();

    /* WiFi driver init */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Register event handlers */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    /* Configure STA mode */
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password,
            sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Waiting for WiFi connection to '%s'...", ssid);

    /* Wait for either connected or failed */
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_events,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE,
        pdMS_TO_TICKS(120000) /* 2 min timeout */
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi connected successfully");
        return true;
    }

    ESP_LOGW(TAG, "Initial WiFi connection failed, background retry enabled");
    return false;
}

bool netmgr_start_fallback_ap(const char *ap_ssid, const char *ap_password)
{
    ESP_LOGI(TAG, "Starting fallback SoftAP: %s", ap_ssid);

    /* Stop STA mode first */
    esp_wifi_stop();

    esp_netif_create_default_wifi_ap();

    wifi_config_t ap_config = {0};
    strncpy((char *)ap_config.ap.ssid, ap_ssid, sizeof(ap_config.ap.ssid) - 1);
    ap_config.ap.ssid_len = strlen(ap_ssid);
    ap_config.ap.max_connection = 2;
    ap_config.ap.channel = 1;

    if (ap_password && strlen(ap_password) >= 8) {
        strncpy((char *)ap_config.ap.password, ap_password,
                sizeof(ap_config.ap.password) - 1);
        ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "SoftAP started. Connect to '%s' to configure.", ap_ssid);
    return true;
}

bool netmgr_is_connected(void)
{
    return s_connected;
}

bool netmgr_get_ip(char *buf, size_t buflen)
{
    if (!s_connected || s_ip_str[0] == '\0') return false;
    strncpy(buf, s_ip_str, buflen - 1);
    buf[buflen - 1] = '\0';
    return true;
}

void netmgr_deinit(void)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    s_connected = false;
}

#else
/* ---- Host stub for compilation ----------------------------------------- */

static bool s_host_connected = false;

bool netmgr_wifi_init(const char *ssid, const char *password, uint8_t max_retry)
{
    (void)max_retry;
    (void)password;
    printf("[NETMGR] Host stub: simulating connection to '%s'\n", ssid);
    s_host_connected = true;
    return true;
}

bool netmgr_start_fallback_ap(const char *ap_ssid, const char *ap_password)
{
    (void)ap_password;
    printf("[NETMGR] Host stub: fallback AP '%s' (simulated)\n", ap_ssid);
    return true;
}

bool netmgr_is_connected(void) { return s_host_connected; }

bool netmgr_get_ip(char *buf, size_t buflen)
{
    strncpy(buf, "127.0.0.1", buflen - 1);
    return s_host_connected;
}

void netmgr_deinit(void) { s_host_connected = false; }

#endif /* ESP_PLATFORM */

/* ===========================================================================
 * idk-nagger-4 : Network Manager — Public API
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * WiFi STA manager with DHCP, exponential backoff reconnection,
 * and fallback AP mode for configuration.
 * ======================================================================== */

#ifndef NETMGR_H
#define NETMGR_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Reconnection parameters */
#define NETMGR_RETRY_BASE_MS   1000
#define NETMGR_RETRY_MAX_MS    60000
#define NETMGR_DEFAULT_TIMEOUT 30

/**
 * Initialize WiFi in STA mode, connect to the given AP.
 * Blocks until connected or max retries exhausted.
 *
 * @param ssid       WiFi SSID (null-terminated, max 31 chars)
 * @param password   WiFi password (null-terminated, max 63 chars)
 * @param max_retry  Maximum connection attempts before background retry
 * @return true if connected, false if initial connect failed (retries continue)
 */
bool netmgr_wifi_init(const char *ssid, const char *password, uint8_t max_retry);

/**
 * Start a fallback SoftAP for configuration.
 * Only called when STA connection fails after timeout.
 *
 * @param ap_ssid     SSID for the SoftAP (e.g., "idk-nagger-4-setup")
 * @param ap_password Password for SoftAP (min 8 chars, or NULL for open)
 * @return true on success
 */
bool netmgr_start_fallback_ap(const char *ap_ssid, const char *ap_password);

/**
 * Check if WiFi STA is currently connected.
 */
bool netmgr_is_connected(void);

/**
 * Get the current IP address (STA mode).
 * @param buf     Buffer to write IP string into
 * @param buflen  Size of buffer (min 16 bytes)
 * @return true if IP is available
 */
bool netmgr_get_ip(char *buf, size_t buflen);

/**
 * Disconnect and deinitialize WiFi.
 */
void netmgr_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* NETMGR_H */

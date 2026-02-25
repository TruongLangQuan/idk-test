/* ===========================================================================
 * idk-nagger-4 : WiFi Manager — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Handles WiFi station initialization, auto-connect, and reconnection
 * with exponential backoff. After max retries, enters a low-power
 * retry loop. No captive portal / SoftAP.
 * ======================================================================== */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the WiFi subsystem and begin connecting.
 *
 * This function initializes NVS, TCP/IP, WiFi driver, and starts
 * the connection process. It blocks until either:
 *   - WiFi is connected and IP is obtained, OR
 *   - Maximum retries are exhausted (enters background retry loop)
 *
 * @param ssid      SSID to connect to.
 * @param password  WiFi password.
 * @param max_retry Maximum connection attempts before entering low-power mode.
 * @return true if initial connection succeeded, false if retries exhausted.
 */
bool wifi_manager_init(const char *ssid, const char *password, uint8_t max_retry);

/**
 * @brief Check if WiFi is currently connected.
 * @return true if connected with IP.
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Get the current IP address as a string.
 * @param buf     Output buffer.
 * @param buflen  Size of buffer.
 * @return true if IP is available.
 */
bool wifi_manager_get_ip(char *buf, size_t buflen);

/**
 * @brief Disconnect and deinitialize WiFi.
 */
void wifi_manager_deinit(void);

#endif /* WIFI_MANAGER_H */

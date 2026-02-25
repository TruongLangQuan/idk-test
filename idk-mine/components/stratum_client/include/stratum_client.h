/* ===========================================================================
 * idk-nagger-4 : Stratum Client — Public API
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Minimal Stratum v1 JSON-RPC client for connecting to mining pools.
 * Supports both simulated mode (offline) and real network connections.
 *
 * Stratum v1 flow:
 *   1. TCP connect to pool
 *   2. mining.subscribe → get session ID
 *   3. mining.authorize → authenticate with wallet.worker
 *   4. mining.notify → receive new jobs
 *   5. mining.submit → submit found shares
 *
 * NOTE: XMRig is NOT supported on ESP32. This is a lightweight, custom
 * implementation designed for the resource constraints of embedded systems.
 * ======================================================================== */

#ifndef STRATUM_CLIENT_H
#define STRATUM_CLIENT_H

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Stratum client state ---------------------------------------------- */
typedef struct {
    char     host[128];
    uint16_t port;
    int      sock_fd;
    bool     connected;
    bool     authorized;
    bool     simulated;
    char     worker_name[128];

    /* Statistics */
    uint32_t shares_submitted;
    uint32_t shares_accepted;
    uint32_t shares_rejected;

    /* Reconnect state */
    uint8_t  reconnect_attempts;
    uint32_t reconnect_delay_ms;
} stratum_client_t;

/**
 * Initialize the stratum client structure.
 */
void stratum_init(stratum_client_t *client, const char *host,
                  uint16_t port, bool simulated);

/**
 * Connect to pool and perform subscribe + authorize handshake.
 * @return true if connected and authorized
 */
bool stratum_connect(stratum_client_t *client, const char *wallet,
                     const char *worker_name);

/**
 * Request a new mining job from the pool.
 * In simulated mode, generates a fake job locally.
 * @return true if a valid job was received
 */
bool stratum_get_job(stratum_client_t *client, mining_job_t *job);

/**
 * Submit a share (found nonce) to the pool.
 * @return true if the share was accepted
 */
bool stratum_submit_share(stratum_client_t *client,
                          const mining_job_t *job, uint32_t nonce);

/**
 * Attempt to reconnect to the pool with exponential backoff.
 * @return true if reconnected
 */
bool stratum_reconnect(stratum_client_t *client);

/**
 * Disconnect from the pool.
 */
void stratum_disconnect(stratum_client_t *client);

#ifdef __cplusplus
}
#endif

#endif /* STRATUM_CLIENT_H */

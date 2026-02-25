/* ===========================================================================
 * idk-nagger-4 : Pool Client — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Minimal Stratum-like JSON-RPC pool client. Supports:
 *   - Connecting to a real mining pool (Stratum v1)
 *   - Connecting to the host_simulator.py test pool
 *   - Built-in simulated pool mode (no network needed)
 *
 * Protocol flow (simplified Stratum v1):
 *   1. mining.subscribe  → get session info
 *   2. mining.authorize  → authenticate with wallet.worker
 *   3. mining.notify     → receive new job
 *   4. mining.submit     → submit share (nonce that meets target)
 * ======================================================================== */

#ifndef POOL_CLIENT_H
#define POOL_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

/* ---- Job Structure ----------------------------------------------------- */
typedef struct {
    char     job_id[64];        /* Pool-assigned job ID */
    uint8_t  prev_hash[32];     /* Previous block hash */
    uint8_t  merkle_root[32];   /* Merkle root */
    uint32_t version;           /* Block version */
    uint32_t nbits;             /* Encoded difficulty target */
    uint32_t ntime;             /* Block timestamp */
    uint8_t  header[80];        /* Assembled block header for hashing */
    bool     valid;             /* True if this job has been populated */
} mining_job_t;

/* ---- Pool Client State ------------------------------------------------- */
typedef struct {
    int      sock_fd;           /* TCP socket file descriptor (-1 if disconn) */
    char     host[128];         /* Pool hostname or IP */
    uint16_t port;              /* Pool port */
    bool     simulated;         /* True = use built-in sim, no network */
    bool     connected;         /* True if socket is connected */
    bool     authorized;        /* True if mining.authorize succeeded */
    char     worker_name[64];   /* Worker name (miner_name.wallet) */
    uint32_t shares_submitted;  /* Total shares submitted */
    uint32_t shares_accepted;   /* Accepted shares */
    uint32_t shares_rejected;   /* Rejected shares */
} pool_client_t;

/**
 * @brief Initialize the pool client.
 * @param client     Pointer to pool_client_t to initialize.
 * @param host       Pool hostname/IP.
 * @param port       Pool port.
 * @param simulated  If true, use built-in simulated pool (no network).
 */
void pool_client_init(pool_client_t *client, const char *host, uint16_t port,
                      bool simulated);

/**
 * @brief Connect to the pool and perform subscribe + authorize.
 * @param client       Initialized pool client.
 * @param wallet       Wallet address for authorization.
 * @param worker_name  Worker name (e.g., "idk-nagger-4").
 * @return true on successful connection and authorization.
 */
bool pool_client_connect(pool_client_t *client, const char *wallet,
                         const char *worker_name);

/**
 * @brief Request a new mining job from the pool.
 * @param client  Connected pool client.
 * @param job     Output: populated mining job.
 * @return true if a valid job was received.
 */
bool pool_client_get_job(pool_client_t *client, mining_job_t *job);

/**
 * @brief Submit a share (nonce) to the pool.
 * @param client  Connected pool client.
 * @param job     The job this share is for.
 * @param nonce   The nonce that produced a valid hash.
 * @return true if the share was accepted.
 */
bool pool_client_submit_share(pool_client_t *client, const mining_job_t *job,
                              uint32_t nonce);

/**
 * @brief Disconnect from the pool.
 */
void pool_client_disconnect(pool_client_t *client);

#endif /* POOL_CLIENT_H */

/* ===========================================================================
 * idk-nagger-4 : Protocol Adapter — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Bridges the miner core to the pool client. Abstracts job fetching
 * and share submission so the miner doesn't need to know whether
 * it's talking to a real pool, test pool, or simulated pool.
 * ======================================================================== */

#ifndef PROTOCOL_ADAPTER_H
#define PROTOCOL_ADAPTER_H

#include "../net/pool_client.h"
#include <stdbool.h>

/**
 * @brief Initialize the protocol adapter and connect to the pool.
 * @return true if pool connection/authorization succeeded.
 */
bool protocol_adapter_init(void);

/**
 * @brief Fetch a new job from the pool.
 * @param job  Output: populated mining job.
 * @return true if a valid job was obtained.
 */
bool protocol_adapter_get_job(mining_job_t *job);

/**
 * @brief Submit a found share to the pool.
 * @param job    The job the share belongs to.
 * @param nonce  The winning nonce.
 * @return true if the share was accepted.
 */
bool protocol_adapter_submit(const mining_job_t *job, uint32_t nonce);

/**
 * @brief Get pool client statistics.
 * @param accepted   Output: accepted shares count.
 * @param rejected   Output: rejected shares count.
 * @param submitted  Output: total shares submitted.
 */
void protocol_adapter_get_stats(uint32_t *accepted, uint32_t *rejected,
                                uint32_t *submitted);

/**
 * @brief Check if the pool connection is alive.
 */
bool protocol_adapter_is_connected(void);

/**
 * @brief Shutdown the protocol adapter.
 */
void protocol_adapter_shutdown(void);

#endif /* PROTOCOL_ADAPTER_H */

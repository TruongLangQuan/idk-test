/* ===========================================================================
 * idk-nagger-4 : Miner Core — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * The miner core manages worker tasks, job lifecycle, and telemetry.
 * It spawns N FreeRTOS tasks (one per CPU core on dual-core ESP32)
 * that independently mine different nonce ranges.
 *
 * Inspired by NerdMiner v2 and Bitsy Miner's compact worker loop
 * design — rate-limited submits and lean job rotation.
 * ======================================================================== */

#ifndef MINER_CORE_H
#define MINER_CORE_H

#include <stdint.h>
#include <stdbool.h>

/* ---- Miner Statistics -------------------------------------------------- */
typedef struct {
    uint64_t total_hashes;          /* Total hashes computed */
    uint32_t shares_found;          /* Shares that met local target */
    uint32_t shares_accepted;       /* Shares accepted by pool */
    uint32_t shares_rejected;       /* Shares rejected by pool */
    double   hashrate;              /* Current hash rate (H/s) */
    uint32_t uptime_sec;            /* Uptime in seconds */
    uint8_t  active_workers;        /* Number of running worker tasks */
} miner_stats_t;

/**
 * @brief Initialize the miner core. Call after config_init() and WiFi.
 * @return true on success.
 */
bool miner_core_init(void);

/**
 * @brief Start mining. Spawns worker tasks.
 */
void miner_core_start(void);

/**
 * @brief Stop mining. Kills worker tasks gracefully.
 */
void miner_core_stop(void);

/**
 * @brief Get current miner statistics.
 * @param stats  Output: populated stats structure.
 */
void miner_core_get_stats(miner_stats_t *stats);

/**
 * @brief Print a JSON telemetry line to serial.
 *
 * Format:
 * {"miner":"idk-nagger-4","hashrate":30000.0,"hashes":1500000,
 *  "accepted":5,"rejected":0,"uptime":300}
 */
void miner_core_print_telemetry(void);

/**
 * @brief Check if the miner is currently running.
 */
bool miner_core_is_running(void);

#endif /* MINER_CORE_H */

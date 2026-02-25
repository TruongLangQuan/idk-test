/* ===========================================================================
 * idk-nagger-4 : Target Select — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Abstracts BTC vs LTC mode selection. Provides a uniform hashing
 * interface so the miner core doesn't need to know which algorithm
 * is active.
 * ======================================================================== */

#ifndef TARGET_SELECT_H
#define TARGET_SELECT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../system/config.h"

/**
 * @brief Initialize the target selector for the given mining mode.
 * @param mode  Mining mode (BTC, LTC, or SIM).
 * @return true if the mode is supported and initialized.
 */
bool target_select_init(mine_mode_t mode);

/**
 * @brief Hash an 80-byte block header using the active algorithm.
 *
 * For BTC/SIM: double-SHA256
 * For LTC: scrypt(N=1024, r=1, p=1)
 *
 * @param header  80-byte block header with nonce set.
 * @param hash    Output: 32-byte hash result.
 * @return true on success, false on error (e.g., scrypt OOM).
 */
bool target_select_hash(const uint8_t *header, uint8_t *hash);

/**
 * @brief Get the name of the currently active algorithm.
 * @return String like "SHA256d", "scrypt", or "SHA256d (sim)".
 */
const char* target_select_algo_name(void);

/**
 * @brief Get the current mining mode.
 */
mine_mode_t target_select_get_mode(void);

#endif /* TARGET_SELECT_H */

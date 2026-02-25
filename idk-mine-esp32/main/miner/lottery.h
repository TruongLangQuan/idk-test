/* ===========================================================================
 * idk-nagger-4 : Lottery Mining — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Implements the "lottery" mining approach: we iterate through nonces
 * looking for a hash that meets a (configurable) difficulty target.
 * Unlike professional miners, we use a simplified difficulty threshold
 * (number of leading zero bits) to make testing practical.
 *
 * Inspired by NerdMiner v2 and Bitsy Miner: we accept that finding
 * a real Bitcoin block is astronomically unlikely, but provide the
 * framework to attempt it. The real value is in benchmarking and
 * learning the mining workflow.
 * ======================================================================== */

#ifndef LOTTERY_H
#define LOTTERY_H

#include <stdint.h>
#include <stdbool.h>

/* ---- Lottery Mining Result --------------------------------------------- */
typedef struct {
    uint32_t nonce;             /* Winning nonce (if found) */
    uint8_t  hash[32];          /* Hash that met the target */
    uint32_t hashes_done;       /* Total hashes computed in this batch */
    bool     found;             /* True if a valid share was found */
} lottery_result_t;

/**
 * @brief Run a batch of lottery mining attempts on a block header.
 *
 * Iterates from `start_nonce` for `batch_size` nonces, hashing the
 * header with each nonce and checking against the difficulty target.
 *
 * @param header          80-byte block header (nonce field will be modified).
 * @param start_nonce     First nonce to try.
 * @param batch_size      Number of nonces to try in this batch.
 * @param difficulty_bits Number of leading zero bits required.
 * @param result          Output: result of the mining batch.
 * @return true if a valid share was found in this batch.
 */
bool lottery_mine_batch(
    uint8_t *header,
    uint32_t start_nonce,
    uint32_t batch_size,
    uint8_t  difficulty_bits,
    lottery_result_t *result
);

/**
 * @brief Check if a hash meets the difficulty target.
 *
 * @param hash             32-byte hash.
 * @param difficulty_bits  Number of leading zero bits required.
 * @return true if the hash meets or exceeds the target.
 */
bool lottery_check_target(const uint8_t *hash, uint8_t difficulty_bits);

/**
 * @brief Convert a hash to a hex string for display.
 *
 * @param hash    32-byte hash.
 * @param hex_out Output buffer, must be at least 65 bytes.
 */
void lottery_hash_to_hex(const uint8_t *hash, char *hex_out);

#endif /* LOTTERY_H */

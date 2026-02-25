/* ===========================================================================
 * idk-nagger-4 : Scrypt Wrapper — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Scrypt key derivation function wrapper for Litecoin mining.
 * Litecoin uses scrypt with N=1024, r=1, p=1, keylen=32.
 *
 * Memory requirement: 128 * N * r = 128 KB per scrypt call.
 * On ESP32 without PSRAM (~320 KB total free heap), this is tight.
 * With PSRAM (up to 4 MB), it's comfortable.
 *
 * This module detects available memory at runtime and:
 *   - Uses full scrypt if enough RAM is available
 *   - Falls back to reduced N (lower difficulty) with a warning
 *   - Returns an error if scrypt is completely infeasible
 *
 * TODO: For production LTC mining (still impractical on ESP32),
 *       consider an assembly-optimized scrypt or offloading via
 *       an external co-processor.
 * ======================================================================== */

#ifndef SCRYPT_WRAPPER_H
#define SCRYPT_WRAPPER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ---- Scrypt Result Codes ----------------------------------------------- */
typedef enum {
    SCRYPT_OK         = 0,
    SCRYPT_ERR_NOMEM  = -1,  /* Not enough memory for scrypt buffers */
    SCRYPT_ERR_PARAM  = -2,  /* Invalid parameters */
    SCRYPT_DISABLED   = -3   /* Feature compiled out */
} scrypt_err_t;

/**
 * @brief Compute scrypt hash (Litecoin POW).
 *
 * @param password  Input block header (80 bytes for Litecoin).
 * @param passlen   Length of password.
 * @param salt      Salt (same as password for Litecoin).
 * @param saltlen   Length of salt.
 * @param N         CPU/memory cost parameter (1024 for LTC).
 * @param r         Block size parameter (1 for LTC).
 * @param p         Parallelism parameter (1 for LTC).
 * @param output    Output buffer (32 bytes for LTC).
 * @param outlen    Output length.
 * @return SCRYPT_OK on success, error code on failure.
 */
scrypt_err_t scrypt_compute(
    const uint8_t *password, size_t passlen,
    const uint8_t *salt, size_t saltlen,
    uint32_t N, uint32_t r, uint32_t p,
    uint8_t *output, size_t outlen
);

/**
 * @brief Check if scrypt is feasible with current memory.
 *
 * @param N  Cost parameter to check.
 * @param r  Block size.
 * @return true if enough memory is available.
 */
bool scrypt_check_memory(uint32_t N, uint32_t r);

/**
 * @brief Get the maximum feasible N value for current memory.
 * @return Maximum N value that can be used.
 */
uint32_t scrypt_max_feasible_n(void);

/**
 * @brief Run a self-test with a known test vector.
 * @return 0 on success, -1 on failure.
 */
int scrypt_self_test(void);

#endif /* SCRYPT_WRAPPER_H */

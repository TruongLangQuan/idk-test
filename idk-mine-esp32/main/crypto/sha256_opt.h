/* ===========================================================================
 * idk-nagger-4 : Optimized SHA-256 — Header
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Provides a unified SHA-256 interface. On ESP32, routes through the
 * hardware crypto accelerator via mbedTLS. On host builds, uses a
 * portable C implementation for testing.
 *
 * For Bitcoin mining, the critical operation is double-SHA256:
 *   hash = SHA256(SHA256(block_header))
 * ======================================================================== */

#ifndef SHA256_OPT_H
#define SHA256_OPT_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Compute SHA-256 of the input data.
 *
 * @param data   Input bytes.
 * @param len    Length of input in bytes.
 * @param digest Output buffer — must be at least 32 bytes.
 */
void sha256_compute(const uint8_t *data, size_t len, uint8_t *digest);

/**
 * @brief Compute double-SHA256 (Bitcoin standard): SHA256(SHA256(data)).
 *
 * @param data   Input bytes (typically an 80-byte block header).
 * @param len    Length of input in bytes.
 * @param digest Output buffer — must be at least 32 bytes.
 */
void sha256d_compute(const uint8_t *data, size_t len, uint8_t *digest);

/**
 * @brief Run a quick self-test to verify SHA-256 correctness.
 * @return 0 on success, -1 on failure.
 */
int sha256_self_test(void);

/**
 * @brief Measure SHA-256 throughput (hashes/second).
 *
 * Performs `iterations` double-SHA256 operations on a dummy block header
 * and returns the measured hash rate.
 *
 * @param iterations  Number of hashing rounds.
 * @return Hashes per second.
 */
double sha256_benchmark(uint32_t iterations);

#endif /* SHA256_OPT_H */

/* ===========================================================================
 * idk-nagger-4 : Crypto Benchmark (Host)
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Measures SHA-256 and scrypt throughput on the host machine.
 * Results are indicative of software performance; ESP32 HW-accelerated
 * SHA-256 will be significantly faster.
 * ======================================================================== */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "crypto/sha256_opt.h"
#include "crypto/scrypt_wrapper.h"

static void bench_sha256(uint32_t iterations)
{
    printf("--- SHA-256 Benchmark ---\n");
    printf("  Iterations: %u\n", iterations);

    uint8_t block[80];
    uint8_t digest[32];
    memset(block, 0xAA, sizeof(block));

    clock_t start = clock();
    for (uint32_t i = 0; i < iterations; i++) {
        block[76] = (uint8_t)(i & 0xFF);
        block[77] = (uint8_t)((i >> 8) & 0xFF);
        block[78] = (uint8_t)((i >> 16) & 0xFF);
        block[79] = (uint8_t)((i >> 24) & 0xFF);
        sha256_compute(block, 80, digest);
    }
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("  Single SHA-256: %.1f H/s (%.3f sec)\n",
           iterations / elapsed, elapsed);

    /* Double SHA-256 */
    start = clock();
    for (uint32_t i = 0; i < iterations; i++) {
        block[76] = (uint8_t)(i & 0xFF);
        sha256d_compute(block, 80, digest);
    }
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("  Double SHA-256: %.1f H/s (%.3f sec)\n",
           iterations / elapsed, elapsed);
    printf("\n");
}

static void bench_scrypt(uint32_t iterations)
{
#if FEATURE_SCRYPT_ENABLED
    printf("--- Scrypt Benchmark ---\n");
    printf("  Iterations: %u (N=1024, r=1, p=1)\n", iterations);

    uint8_t block[80];
    uint8_t output[32];
    memset(block, 0xBB, sizeof(block));

    clock_t start = clock();
    for (uint32_t i = 0; i < iterations; i++) {
        block[76] = (uint8_t)(i & 0xFF);
        scrypt_compute(block, 80, block, 80, 1024, 1, 1, output, 32);
    }
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("  Scrypt (N=1024): %.1f H/s (%.3f sec)\n",
           iterations / elapsed, elapsed);
    printf("  Memory per call: ~128 KB\n");
    printf("\n");
#else
    (void)iterations;
    printf("--- Scrypt Benchmark: DISABLED (FEATURE_SCRYPT_ENABLED=0) ---\n\n");
#endif
}

static void bench_memory(void)
{
    printf("--- Memory Usage ---\n");
    printf("  Block header:     80 bytes\n");
    printf("  SHA-256 digest:   32 bytes\n");
    printf("  Scrypt V buffer: %d KB (N=%d, r=%d)\n",
           (128 * 1024 * 1) / 1024, 1024, 1);
    printf("  Scrypt B buffer: %d bytes\n", 128 * 1);
    printf("\n");
    printf("  ESP32 internal SRAM: ~320 KB total\n");
    printf("  ESP32 + PSRAM:       up to 4 MB\n");
    printf("\n");
}

int main(void)
{
    printf("\n============================================\n");
    printf("  idk-nagger-4 Crypto Benchmarks\n");
    printf("============================================\n\n");

    /* SHA-256: 500k iterations (fast) */
    bench_sha256(500000);

    /* Scrypt: only 10 iterations (very slow) */
    bench_scrypt(10);

    /* Memory usage info */
    bench_memory();

    printf("============================================\n");
    printf("  Benchmark complete.\n");
    printf("============================================\n");

    return 0;
}

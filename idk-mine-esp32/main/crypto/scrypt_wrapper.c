/* ===========================================================================
 * idk-nagger-4 : Scrypt Wrapper — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * This is a SIMPLIFIED, PORTABLE scrypt implementation for testing and
 * educational purposes. It implements the core scrypt algorithm with
 * PBKDF2-SHA256 as the underlying PRF.
 *
 * TRADE-OFFS:
 *   - This is NOT assembly-optimized; throughput is very low on ESP32.
 *   - On ESP32 without PSRAM, N is capped to fit available heap.
 *   - Full Litecoin scrypt (N=1024) needs ~128 KB; feasible with PSRAM.
 *
 * TODO: Plug in an optimized scrypt library (e.g., Colin Percival's
 *       reference or an Xtensa-optimized variant) for better throughput.
 * ======================================================================== */

#include "scrypt_wrapper.h"
#include "sha256_opt.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if FEATURE_SCRYPT_ENABLED

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "esp_log.h"
static const char *TAG = "SCRYPT";
#define LOG_W(fmt, ...) ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#else
#define LOG_W(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#define LOG_I(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#endif

/* ---- HMAC-SHA256 ------------------------------------------------------- */
static void hmac_sha256(const uint8_t *key, size_t keylen,
                        const uint8_t *data, size_t datalen,
                        uint8_t *out)
{
    uint8_t k_ipad[64], k_opad[64];
    uint8_t tk[32];

    /* If key is longer than 64 bytes, hash it */
    if (keylen > 64) {
        sha256_compute(key, keylen, tk);
        key = tk;
        keylen = 32;
    }

    memset(k_ipad, 0x36, 64);
    memset(k_opad, 0x5c, 64);
    for (size_t i = 0; i < keylen; i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    /* inner hash: SHA256(k_ipad || data) */
    size_t inner_len = 64 + datalen;
    uint8_t *inner = (uint8_t *)malloc(inner_len);
    if (!inner) return;
    memcpy(inner, k_ipad, 64);
    memcpy(inner + 64, data, datalen);
    uint8_t inner_hash[32];
    sha256_compute(inner, inner_len, inner_hash);
    free(inner);

    /* outer hash: SHA256(k_opad || inner_hash) */
    uint8_t outer[64 + 32];
    memcpy(outer, k_opad, 64);
    memcpy(outer + 64, inner_hash, 32);
    sha256_compute(outer, 96, out);
}

/* ---- PBKDF2-SHA256 ----------------------------------------------------- */
static void pbkdf2_sha256(const uint8_t *password, size_t passlen,
                          const uint8_t *salt, size_t saltlen,
                          uint32_t iterations,
                          uint8_t *output, size_t outlen)
{
    uint32_t block_count = (uint32_t)((outlen + 31) / 32);
    size_t salt_int_len = saltlen + 4;
    uint8_t *salt_int = (uint8_t *)malloc(salt_int_len);
    if (!salt_int) return;
    memcpy(salt_int, salt, saltlen);

    for (uint32_t i = 1; i <= block_count; i++) {
        /* Append block index (big-endian) */
        salt_int[saltlen + 0] = (uint8_t)(i >> 24);
        salt_int[saltlen + 1] = (uint8_t)(i >> 16);
        salt_int[saltlen + 2] = (uint8_t)(i >> 8);
        salt_int[saltlen + 3] = (uint8_t)(i);

        uint8_t U[32], T[32];
        hmac_sha256(password, passlen, salt_int, salt_int_len, U);
        memcpy(T, U, 32);

        for (uint32_t j = 1; j < iterations; j++) {
            uint8_t U_prev[32];
            memcpy(U_prev, U, 32);
            hmac_sha256(password, passlen, U_prev, 32, U);
            for (int k = 0; k < 32; k++) T[k] ^= U[k];
        }

        size_t offset = (size_t)(i - 1) * 32;
        size_t copy_len = (outlen - offset < 32) ? outlen - offset : 32;
        memcpy(output + offset, T, copy_len);
    }
    free(salt_int);
}

/* ---- Salsa20/8 core ---------------------------------------------------- */
#define R(a,b) (((a) << (b)) | ((a) >> (32 - (b))))

static void salsa20_8(uint32_t B[16])
{
    uint32_t x[16];
    memcpy(x, B, 64);

    for (int i = 0; i < 8; i += 2) {
        /* Column round */
        x[ 4] ^= R(x[ 0]+x[12], 7);  x[ 8] ^= R(x[ 4]+x[ 0], 9);
        x[12] ^= R(x[ 8]+x[ 4],13);  x[ 0] ^= R(x[12]+x[ 8],18);
        x[ 9] ^= R(x[ 5]+x[ 1], 7);  x[13] ^= R(x[ 9]+x[ 5], 9);
        x[ 1] ^= R(x[13]+x[ 9],13);  x[ 5] ^= R(x[ 1]+x[13],18);
        x[14] ^= R(x[10]+x[ 6], 7);  x[ 2] ^= R(x[14]+x[10], 9);
        x[ 6] ^= R(x[ 2]+x[14],13);  x[10] ^= R(x[ 6]+x[ 2],18);
        x[ 3] ^= R(x[15]+x[11], 7);  x[ 7] ^= R(x[ 3]+x[15], 9);
        x[11] ^= R(x[ 7]+x[ 3],13);  x[15] ^= R(x[11]+x[ 7],18);
        /* Row round */
        x[ 1] ^= R(x[ 0]+x[ 3], 7);  x[ 2] ^= R(x[ 1]+x[ 0], 9);
        x[ 3] ^= R(x[ 2]+x[ 1],13);  x[ 0] ^= R(x[ 3]+x[ 2],18);
        x[ 6] ^= R(x[ 5]+x[ 4], 7);  x[ 7] ^= R(x[ 6]+x[ 5], 9);
        x[ 4] ^= R(x[ 7]+x[ 6],13);  x[ 5] ^= R(x[ 4]+x[ 7],18);
        x[11] ^= R(x[10]+x[ 9], 7);  x[ 8] ^= R(x[11]+x[10], 9);
        x[ 9] ^= R(x[ 8]+x[11],13);  x[10] ^= R(x[ 9]+x[ 8],18);
        x[12] ^= R(x[15]+x[14], 7);  x[13] ^= R(x[12]+x[15], 9);
        x[14] ^= R(x[13]+x[12],13);  x[15] ^= R(x[14]+x[13],18);
    }

    for (int i = 0; i < 16; i++) B[i] += x[i];
}

#undef R

/* ---- scrypt BlockMix --------------------------------------------------- */
static void scrypt_block_mix(uint32_t *B, uint32_t *Y, uint32_t r)
{
    uint32_t X[16];
    uint32_t two_r = 2 * r;

    memcpy(X, &B[(two_r - 1) * 16], 64);

    for (uint32_t i = 0; i < two_r; i++) {
        for (int j = 0; j < 16; j++) X[j] ^= B[i * 16 + j];
        salsa20_8(X);
        memcpy(&Y[i * 16], X, 64);
    }

    /* Re-order: even blocks then odd blocks */
    for (uint32_t i = 0; i < r; i++) {
        memcpy(&B[i * 16], &Y[(2 * i) * 16], 64);
    }
    for (uint32_t i = 0; i < r; i++) {
        memcpy(&B[(i + r) * 16], &Y[(2 * i + 1) * 16], 64);
    }
}

/* ---- scrypt ROMix -------------------------------------------------------- */
static int scrypt_romix(uint32_t *B, uint32_t r, uint32_t N, uint32_t *V)
{
    uint32_t block_words = 32 * r;  /* words per block */
    uint32_t *Y = (uint32_t *)malloc(block_words * sizeof(uint32_t));
    if (!Y) return -1;

    /* Step 1: Build lookup table V */
    for (uint32_t i = 0; i < N; i++) {
        memcpy(&V[i * block_words], B, block_words * sizeof(uint32_t));
        scrypt_block_mix(B, Y, r);
    }

    /* Step 2: Mix with random lookups */
    for (uint32_t i = 0; i < N; i++) {
        uint32_t j = B[block_words - 16] & (N - 1);
        for (uint32_t k = 0; k < block_words; k++) {
            B[k] ^= V[j * block_words + k];
        }
        scrypt_block_mix(B, Y, r);
    }

    free(Y);
    return 0;
}

/* ---- Public API -------------------------------------------------------- */

bool scrypt_check_memory(uint32_t N, uint32_t r)
{
    size_t needed = (size_t)128 * r * N + (size_t)256 * r;
#ifdef ESP_PLATFORM
    size_t avail = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    LOG_I("Scrypt memory check: need %zu, available %zu", needed, avail);
    return avail >= needed;
#else
    (void)needed;
    return true;  /* Assume host has enough memory */
#endif
}

uint32_t scrypt_max_feasible_n(void)
{
#ifdef ESP_PLATFORM
    size_t avail = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    /* Reserve 32 KB for other tasks */
    if (avail < 32768) return 0;
    avail -= 32768;
    /* N = avail / (128 * r), with r=1 */
    uint32_t max_n = (uint32_t)(avail / 128);
    /* Round down to power of 2 */
    uint32_t n = 1;
    while (n * 2 <= max_n) n *= 2;
    return n;
#else
    return 1024;
#endif
}

scrypt_err_t scrypt_compute(
    const uint8_t *password, size_t passlen,
    const uint8_t *salt, size_t saltlen,
    uint32_t N, uint32_t r, uint32_t p,
    uint8_t *output, size_t outlen)
{
    /* Validate N is power of 2 */
    if (N == 0 || (N & (N - 1)) != 0) {
        LOG_E("scrypt: N must be a power of 2 (got %u)", (unsigned)N);
        return SCRYPT_ERR_PARAM;
    }

    size_t block_size = 128 * r;
    size_t B_size = block_size * p;

    /* Check memory feasibility */
    if (!scrypt_check_memory(N, r)) {
        uint32_t fallback_n = scrypt_max_feasible_n();
        if (fallback_n < 2) {
            LOG_E("scrypt: Not enough memory (need %luKB)", (unsigned long)((128 * r * N) / 1024));
            return SCRYPT_ERR_NOMEM;
        }
        LOG_W("scrypt: Reducing N from %u to %u due to memory limits", (unsigned)N, (unsigned)fallback_n);
        N = fallback_n;
    }

    /* Step 1: B = PBKDF2(password, salt, 1, p * 128 * r) */
    uint8_t *B = (uint8_t *)malloc(B_size);
    if (!B) {
        LOG_E("scrypt: Failed to allocate B buffer (%zu bytes)", B_size);
        return SCRYPT_ERR_NOMEM;
    }
    pbkdf2_sha256(password, passlen, salt, saltlen, 1, B, B_size);

    /* Step 2: Allocate V (the large lookup table) */
    size_t V_size = (size_t)128 * r * N;
    uint32_t *V = (uint32_t *)malloc(V_size);
    if (!V) {
        LOG_E("scrypt: Failed to allocate V buffer (%zu bytes)", V_size);
        free(B);
        return SCRYPT_ERR_NOMEM;
    }

    /* Step 3: ROMix each block */
    for (uint32_t i = 0; i < p; i++) {
        int ret = scrypt_romix((uint32_t *)(B + i * block_size), r, N, V);
        if (ret != 0) {
            free(V);
            free(B);
            return SCRYPT_ERR_NOMEM;
        }
    }

    free(V);

    /* Step 4: output = PBKDF2(password, B, 1, outlen) */
    pbkdf2_sha256(password, passlen, B, B_size, 1, output, outlen);
    free(B);

    return SCRYPT_OK;
}

int scrypt_self_test(void)
{
    /* Test vector from RFC 7914:
     * scrypt("", "", 16, 1, 1, 64) =
     * 77d6576238657b203b19ca42c18a0497f9b3fc3d9e0e3f5ec3ae978c84e0e3c3
     * ... (first 32 bytes) */
    const uint8_t expected_first4[4] = { 0x77, 0xd6, 0x57, 0x62 };
    uint8_t output[64];

    scrypt_err_t err = scrypt_compute(
        (const uint8_t *)"", 0,
        (const uint8_t *)"", 0,
        16, 1, 1,
        output, 64
    );

    if (err != SCRYPT_OK) {
        LOG_E("Scrypt self-test: computation failed (%d)", err);
        return -1;
    }

    if (memcmp(output, expected_first4, 4) != 0) {
        LOG_E("Scrypt self-test: FAILED (output mismatch)");
        return -1;
    }

    LOG_I("Scrypt self-test: PASSED");
    return 0;
}

#else /* FEATURE_SCRYPT_ENABLED == 0 */

scrypt_err_t scrypt_compute(
    const uint8_t *password, size_t passlen,
    const uint8_t *salt, size_t saltlen,
    uint32_t N, uint32_t r, uint32_t p,
    uint8_t *output, size_t outlen)
{
    (void)password; (void)passlen; (void)salt; (void)saltlen;
    (void)N; (void)r; (void)p; (void)output; (void)outlen;
    return SCRYPT_DISABLED;
}

bool scrypt_check_memory(uint32_t N, uint32_t r) { (void)N; (void)r; return false; }
uint32_t scrypt_max_feasible_n(void) { return 0; }
int scrypt_self_test(void) {
    printf("[SCRYPT] Feature disabled at compile time\n");
    return 0;
}

#endif /* FEATURE_SCRYPT_ENABLED */

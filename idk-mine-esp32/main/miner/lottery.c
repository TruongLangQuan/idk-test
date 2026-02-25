/* ===========================================================================
 * idk-nagger-4 : Lottery Mining — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Core mining loop: iterate nonces, hash, check difficulty.
 *
 * Performance notes for ESP32 @ 240 MHz:
 *   - SHA256d: ~25-40 kH/s (with HW acceleration)
 *   - Scrypt:  ~1-5 H/s (very slow, memory-bound)
 *
 * At 40 kH/s vs Bitcoin mainnet difficulty (~80+ leading zero bits),
 * the expected time to find a block is on the order of 10^60 years.
 * This is truly a "lottery" — hence the name.
 * ======================================================================== */

#include "lottery.h"
#include "target_select.h"
#include <string.h>
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "LOTTERY";
#define LOG_D(fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
#else
#define LOG_D(fmt, ...) /* suppress on host */
#endif

/* ---- lottery_check_target ---------------------------------------------- */
bool lottery_check_target(const uint8_t *hash, uint8_t difficulty_bits)
{
    /*
     * Bitcoin hashes are compared as 256-bit little-endian numbers.
     * The "difficulty" is expressed as the number of leading zero bits
     * when the hash is interpreted as a big-endian number.
     *
     * In the hash array:
     *   hash[0..31] where hash[31] is the most significant byte (LE).
     *
     * We check from hash[31] downward for zero bits.
     */
    int full_bytes = difficulty_bits / 8;
    int remaining_bits = difficulty_bits % 8;

    /* Check full zero bytes from the MSB end */
    for (int i = 0; i < full_bytes; i++) {
        if (hash[31 - i] != 0x00) return false;
    }

    /* Check remaining bits in the partial byte */
    if (remaining_bits > 0 && full_bytes < 32) {
        uint8_t mask = (uint8_t)(0xFF << (8 - remaining_bits));
        if ((hash[31 - full_bytes] & mask) != 0) return false;
    }

    return true;
}

/* ---- lottery_mine_batch ------------------------------------------------ */
bool lottery_mine_batch(
    uint8_t *header,
    uint32_t start_nonce,
    uint32_t batch_size,
    uint8_t  difficulty_bits,
    lottery_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->found = false;

    for (uint32_t i = 0; i < batch_size; i++) {
        uint32_t nonce = start_nonce + i;

        /* Set nonce in the header (bytes 76-79, little-endian) */
        header[76] = (uint8_t)(nonce);
        header[77] = (uint8_t)(nonce >> 8);
        header[78] = (uint8_t)(nonce >> 16);
        header[79] = (uint8_t)(nonce >> 24);

        /* Hash using the active algorithm */
        uint8_t hash[32];
        if (!target_select_hash(header, hash)) {
            /* Hash function failed (e.g., scrypt OOM) — skip */
            result->hashes_done = i;
            return false;
        }

        result->hashes_done = i + 1;

        /* Check if this hash meets the target */
        if (lottery_check_target(hash, difficulty_bits)) {
            result->nonce = nonce;
            memcpy(result->hash, hash, 32);
            result->found = true;

            LOG_D("Share found! nonce=0x%08x", (unsigned)nonce);
            return true;
        }
    }

    return false;
}

/* ---- lottery_hash_to_hex ----------------------------------------------- */
void lottery_hash_to_hex(const uint8_t *hash, char *hex_out)
{
    static const char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < 32; i++) {
        hex_out[i * 2]     = hex_chars[(hash[i] >> 4) & 0x0F];
        hex_out[i * 2 + 1] = hex_chars[hash[i] & 0x0F];
    }
    hex_out[64] = '\0';
}

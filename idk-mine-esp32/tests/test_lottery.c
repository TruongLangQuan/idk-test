/* ===========================================================================
 * idk-nagger-4 : Lottery/Miner Logic Unit Tests (Host)
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Compile and run on host (no ESP32 required):
 *   gcc -o test_lottery test_lottery.c \
 *       ../main/crypto/sha256_opt.c \
 *       ../main/miner/lottery.c \
 *       ../main/miner/target_select.c \
 *       ../main/crypto/scrypt_wrapper.c \
 *       ../main/system/config.c \
 *       -I../main -I../config -DFEATURE_SCRYPT_ENABLED=1 -lm
 *   ./test_lottery
 * ======================================================================== */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "miner/lottery.h"
#include "miner/target_select.h"
#include "system/config.h"
#include "crypto/sha256_opt.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %s ... ", name)
#define PASS()     do { printf("PASSED\n"); tests_passed++; } while(0)
#define FAIL(msg)  do { printf("FAILED: %s\n", msg); tests_failed++; } while(0)

/* ---- Test 1: Target check with zero difficulty (always passes) --------- */
static void test_target_zero_difficulty(void)
{
    TEST("Target check: 0-bit difficulty (always valid)");
    uint8_t hash[32];
    memset(hash, 0xFF, 32); /* All 1s — worst case */

    if (lottery_check_target(hash, 0)) PASS();
    else FAIL("0-bit difficulty should accept any hash");
}

/* ---- Test 2: Target check with 8-bit difficulty ------------------------ */
static void test_target_8bit(void)
{
    TEST("Target check: 8-bit difficulty");
    uint8_t hash[32];
    memset(hash, 0, 32);

    /* hash[31] = 0x00 → 8 leading zero bits → should pass */
    hash[31] = 0x00;
    hash[30] = 0xFF;
    if (!lottery_check_target(hash, 8)) { FAIL("should pass with 8 zero bits"); return; }

    /* hash[31] = 0x01 → only 7 leading zero bits → should fail */
    hash[31] = 0x01;
    if (lottery_check_target(hash, 8)) { FAIL("should fail with only 7 zero bits"); return; }

    PASS();
}

/* ---- Test 3: Target check with 16-bit difficulty ----------------------- */
static void test_target_16bit(void)
{
    TEST("Target check: 16-bit difficulty");
    uint8_t hash[32];
    memset(hash, 0xFF, 32);
    hash[31] = 0x00;
    hash[30] = 0x00;

    if (!lottery_check_target(hash, 16)) { FAIL("should pass with 16 zero bits"); return; }

    hash[30] = 0x01;
    if (lottery_check_target(hash, 16)) { FAIL("should fail with only 15 zero bits"); return; }

    PASS();
}

/* ---- Test 4: Hash to hex conversion ------------------------------------ */
static void test_hash_to_hex(void)
{
    TEST("Hash to hex conversion");
    uint8_t hash[32] = {
        0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    char hex[65];
    lottery_hash_to_hex(hash, hex);

    if (strncmp(hex, "abcdef0123456789", 16) == 0 && strlen(hex) == 64)
        PASS();
    else
        FAIL(hex);
}

/* ---- Test 5: Mine batch with easy difficulty (should find share) -------- */
static void test_mine_batch_easy(void)
{
    TEST("Mine batch: 1-bit difficulty (should find share quickly)");

    config_init();
    target_select_init(MINE_MODE_SIM);

    uint8_t header[80];
    memset(header, 0xAA, sizeof(header));
    /* Set version, prev_hash, merkle_root, etc. */
    header[0] = 0x00; header[1] = 0x00; header[2] = 0x00; header[3] = 0x20;

    lottery_result_t result;
    bool found = lottery_mine_batch(header, 0, 1000, 1, &result);

    if (found && result.found && result.hashes_done <= 1000) PASS();
    else FAIL("should find a share with 1-bit difficulty in 1000 tries");
}

/* ---- Test 6: Mine batch tracking hashes done --------------------------- */
static void test_mine_batch_count(void)
{
    TEST("Mine batch: hash count tracking");

    config_init();
    target_select_init(MINE_MODE_SIM);

    uint8_t header[80];
    memset(header, 0xBB, sizeof(header));

    lottery_result_t result;
    /* Use very high difficulty so no share is found → full batch runs */
    lottery_mine_batch(header, 0, 500, 128, &result);

    if (result.hashes_done == 500) PASS();
    else {
        char msg[64];
        snprintf(msg, sizeof(msg), "expected 500, got %u", result.hashes_done);
        FAIL(msg);
    }
}

/* ---- Test 7: Target select initialization ------------------------------ */
static void test_target_select_btc(void)
{
    TEST("Target select: BTC mode init");
    if (target_select_init(MINE_MODE_BTC)) PASS();
    else FAIL("BTC mode init failed");
}

static void test_target_select_sim(void)
{
    TEST("Target select: SIM mode init");
    if (target_select_init(MINE_MODE_SIM)) PASS();
    else FAIL("SIM mode init failed");
}

/* ---- Test 8: Target select hashing ------------------------------------- */
static void test_target_select_hash(void)
{
    TEST("Target select: hash produces output");
    target_select_init(MINE_MODE_BTC);

    uint8_t header[80];
    memset(header, 0x42, sizeof(header));
    uint8_t hash[32];

    if (target_select_hash(header, hash)) {
        /* Check hash is not all zeros (extremely unlikely) */
        int all_zero = 1;
        for (int i = 0; i < 32; i++) {
            if (hash[i] != 0) { all_zero = 0; break; }
        }
        if (!all_zero) PASS();
        else FAIL("hash is all zeros");
    } else {
        FAIL("hash function returned false");
    }
}

/* ---- Test 9: Config initialization ------------------------------------- */
static void test_config_init(void)
{
    TEST("Config: initialization and miner name");
    config_init();
    const miner_config_t *cfg = config_get();

    if (strcmp(cfg->miner_name, "idk-nagger-4") == 0) PASS();
    else FAIL("miner name mismatch");
}

static void test_config_wallet(void)
{
    TEST("Config: wallet addresses set");
    config_init();
    const miner_config_t *cfg = config_get();

    if (strlen(cfg->btc_wallet) > 10 && strlen(cfg->ltc_wallet) > 10) PASS();
    else FAIL("wallet addresses not set");
}

/* ---- Main -------------------------------------------------------------- */
int main(void)
{
    printf("\n=== idk-nagger-4 Lottery/Miner Unit Tests ===\n\n");

    test_target_zero_difficulty();
    test_target_8bit();
    test_target_16bit();
    test_hash_to_hex();
    test_mine_batch_easy();
    test_mine_batch_count();
    test_target_select_btc();
    test_target_select_sim();
    test_target_select_hash();
    test_config_init();
    test_config_wallet();

    printf("\n  Results: %d passed, %d failed\n\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}

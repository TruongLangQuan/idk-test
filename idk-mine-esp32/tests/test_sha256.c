/* ===========================================================================
 * idk-nagger-4 : SHA-256 Unit Tests (Host)
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Compile and run on host (no ESP32 required):
 *   gcc -o test_sha256 test_sha256.c ../main/crypto/sha256_opt.c \
 *       -I../main -I../config -DFEATURE_SCRYPT_ENABLED=1 -lm
 *   ./test_sha256
 * ======================================================================== */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "crypto/sha256_opt.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %s ... ", name)
#define PASS()     do { printf("PASSED\n"); tests_passed++; } while(0)
#define FAIL(msg)  do { printf("FAILED: %s\n", msg); tests_failed++; } while(0)

/* ---- Test 1: SHA-256("abc") -------------------------------------------- */
static void test_sha256_abc(void)
{
    TEST("SHA256(\"abc\")");
    const uint8_t expected[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    uint8_t digest[32];
    sha256_compute((const uint8_t *)"abc", 3, digest);

    if (memcmp(digest, expected, 32) == 0) PASS();
    else FAIL("digest mismatch");
}

/* ---- Test 2: SHA-256("") empty string ---------------------------------- */
static void test_sha256_empty(void)
{
    TEST("SHA256(\"\")");
    const uint8_t expected[32] = {
        0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
    };
    uint8_t digest[32];
    sha256_compute((const uint8_t *)"", 0, digest);

    if (memcmp(digest, expected, 32) == 0) PASS();
    else FAIL("digest mismatch");
}

/* ---- Test 3: Double SHA-256 -------------------------------------------- */
static void test_sha256d(void)
{
    TEST("SHA256d(\"abc\")");
    /* SHA256d("abc") = SHA256(SHA256("abc"))
     * SHA256("abc") = ba7816bf...
     * SHA256(ba7816bf...) = 4f8b42c2... */
    uint8_t digest[32];
    sha256d_compute((const uint8_t *)"abc", 3, digest);

    /* Verify it's not the same as single SHA256 */
    const uint8_t single[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    if (memcmp(digest, single, 32) != 0) PASS();
    else FAIL("double-SHA256 should differ from single");
}

/* ---- Test 4: SHA-256 of longer input ----------------------------------- */
static void test_sha256_longer(void)
{
    TEST("SHA256(448-bit message)");
    /* SHA256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") */
    const char *input = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const uint8_t expected[32] = {
        0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
        0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
        0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
        0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
    };
    uint8_t digest[32];
    sha256_compute((const uint8_t *)input, strlen(input), digest);

    if (memcmp(digest, expected, 32) == 0) PASS();
    else FAIL("digest mismatch");
}

/* ---- Test 5: Self-test function ---------------------------------------- */
static void test_self_test(void)
{
    TEST("sha256_self_test()");
    if (sha256_self_test() == 0) PASS();
    else FAIL("self-test returned non-zero");
}

/* ---- Test 6: 80-byte block header (mining simulation) ------------------ */
static void test_sha256d_block_header(void)
{
    TEST("SHA256d(80-byte block header)");
    uint8_t header[80];
    memset(header, 0xAA, sizeof(header));
    /* Set a specific nonce */
    header[76] = 0x42;
    header[77] = 0x00;
    header[78] = 0x00;
    header[79] = 0x00;

    uint8_t digest1[32], digest2[32];
    sha256d_compute(header, 80, digest1);

    /* Same input should produce same output */
    sha256d_compute(header, 80, digest2);
    if (memcmp(digest1, digest2, 32) == 0) PASS();
    else FAIL("determinism failure");
}

/* ---- Test 7: Different nonces produce different hashes ----------------- */
static void test_nonce_sensitivity(void)
{
    TEST("Different nonces → different hashes");
    uint8_t header[80];
    memset(header, 0x55, sizeof(header));

    uint8_t hash_a[32], hash_b[32];

    header[76] = 0x00;
    sha256d_compute(header, 80, hash_a);

    header[76] = 0x01;
    sha256d_compute(header, 80, hash_b);

    if (memcmp(hash_a, hash_b, 32) != 0) PASS();
    else FAIL("different nonces produced same hash");
}

/* ---- Main -------------------------------------------------------------- */
int main(void)
{
    printf("\n=== idk-nagger-4 SHA-256 Unit Tests ===\n\n");

    test_sha256_abc();
    test_sha256_empty();
    test_sha256d();
    test_sha256_longer();
    test_self_test();
    test_sha256d_block_header();
    test_nonce_sensitivity();

    printf("\n  Results: %d passed, %d failed\n\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}

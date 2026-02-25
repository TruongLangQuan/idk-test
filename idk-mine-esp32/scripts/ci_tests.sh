#!/usr/bin/env bash
# ===========================================================================
# idk-nagger-4 : CI Test Runner
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
#
# Runs unit tests inside a Docker container or on the host.
#
# Usage:
#   ./scripts/ci_tests.sh          # run on host
#   docker run --rm -v $(pwd):/project -w /project \
#       esp32-build-env /project/scripts/ci_tests.sh
# ===========================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "============================================"
echo "  idk-nagger-4 CI Tests"
echo "============================================"

cd "$PROJECT_DIR"

# --- Step 1: Build host unit tests ---
echo "[1/3] Building host unit tests..."
cd tests

if [ ! -d build ]; then
    mkdir build
fi

gcc -o build/test_sha256 \
    test_sha256.c \
    ../main/crypto/sha256_opt.c \
    -I../main -I../config \
    -DFEATURE_SCRYPT_ENABLED=1 \
    -lm -Wall -Wextra -O2

gcc -o build/test_lottery \
    test_lottery.c \
    ../main/crypto/sha256_opt.c \
    ../main/miner/lottery.c \
    ../main/miner/target_select.c \
    ../main/crypto/scrypt_wrapper.c \
    ../main/system/config.c \
    -I../main -I../config \
    -DFEATURE_SCRYPT_ENABLED=1 \
    -lm -Wall -Wextra -O2

echo "   Build OK"

# --- Step 2: Run tests ---
echo ""
echo "[2/3] Running tests..."

echo "  --- SHA-256 tests ---"
./build/test_sha256
SHA_RESULT=$?

echo ""
echo "  --- Lottery/miner tests ---"
./build/test_lottery
LOTTERY_RESULT=$?

cd "$PROJECT_DIR"

# --- Step 3: Results ---
echo ""
echo "[3/3] Results:"
FAILED=0

if [ $SHA_RESULT -eq 0 ]; then
    echo "  ✓ SHA-256 tests:  PASSED"
else
    echo "  ✗ SHA-256 tests:  FAILED"
    FAILED=1
fi

if [ $LOTTERY_RESULT -eq 0 ]; then
    echo "  ✓ Lottery tests:  PASSED"
else
    echo "  ✗ Lottery tests:  FAILED"
    FAILED=1
fi

echo ""
if [ $FAILED -eq 0 ]; then
    echo "All tests PASSED ✓"
    exit 0
else
    echo "Some tests FAILED ✗"
    exit 1
fi

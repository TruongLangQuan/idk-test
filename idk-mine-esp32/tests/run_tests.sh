#!/usr/bin/env bash
# ===========================================================================
# idk-nagger-4 : Test Runner
# ---------------------------------------------------------------------------
# Builds and runs all host-side unit tests.
# ===========================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "Building and running tests..."
echo ""

mkdir -p build

# Build test_sha256
gcc -o build/test_sha256 \
    test_sha256.c \
    ../main/crypto/sha256_opt.c \
    -I../main -I../config \
    -DFEATURE_SCRYPT_ENABLED=1 \
    -lm -Wall -Wextra -O2

# Build test_lottery
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

echo ""
./build/test_sha256
echo ""
./build/test_lottery

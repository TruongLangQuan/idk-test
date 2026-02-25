#!/usr/bin/env bash
# ===========================================================================
# idk-nagger-4 : Performance Microbenchmarks
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
#
# Run on host to measure SHA-256 and scrypt throughput.
# For ESP32 results, flash the firmware and observe serial output.
#
# Usage:
#   cd tools/bench && bash run_bench.sh
# ===========================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"

echo "============================================"
echo "  idk-nagger-4 Benchmarks (Host)"
echo "============================================"
echo ""

mkdir -p "$SCRIPT_DIR/build"

# --- Build benchmark binary ---
echo "Building benchmark..."
gcc -o "$SCRIPT_DIR/build/bench_crypto" \
    "$SCRIPT_DIR/bench_crypto.c" \
    "$PROJECT_DIR/main/crypto/sha256_opt.c" \
    "$PROJECT_DIR/main/crypto/scrypt_wrapper.c" \
    "$PROJECT_DIR/main/system/config.c" \
    -I"$PROJECT_DIR/main" \
    -I"$PROJECT_DIR/config" \
    -DFEATURE_SCRYPT_ENABLED=1 \
    -lm -Wall -O3

echo "Running benchmark..."
echo ""
"$SCRIPT_DIR/build/bench_crypto"

echo ""
echo "============================================"
echo "  NOTE: ESP32 results will differ significantly."
echo "  SHA-256: ~25-40 kH/s (with HW accel)"
echo "  Scrypt:  ~1-5 H/s  (memory-bound)"
echo "============================================"

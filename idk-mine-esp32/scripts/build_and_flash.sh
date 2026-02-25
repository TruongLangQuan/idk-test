#!/usr/bin/env bash
# ===========================================================================
# idk-nagger-4 : Build and Flash Script
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
#
# Usage:
#   ./scripts/build_and_flash.sh [PORT] [BAUD]
#
# Examples:
#   ./scripts/build_and_flash.sh                    # defaults: /dev/ttyUSB0 115200
#   ./scripts/build_and_flash.sh /dev/ttyUSB1 921600
#
# Inside Docker:
#   docker run --rm -v $(pwd):/project --device=/dev/ttyUSB0 \
#       esp32-build-env /project/scripts/build_and_flash.sh /dev/ttyUSB0
# ===========================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PORT="${1:-/dev/ttyUSB0}"
BAUD="${2:-115200}"

echo "============================================"
echo "  idk-nagger-4 Build & Flash"
echo "============================================"
echo "  Project : $PROJECT_DIR"
echo "  Port    : $PORT"
echo "  Baud    : $BAUD"
echo "============================================"

cd "$PROJECT_DIR"

# --- Step 1: Clean build (optional, uncomment for full clean) ---
# echo "[1/4] Cleaning previous build..."
# idf.py fullclean

# --- Step 2: Build ---
echo "[1/3] Building firmware..."
idf.py build

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "Build artifacts:"
ls -la build/idk-nagger-4.bin 2>/dev/null || echo "  (firmware binary)"
ls -la build/partition_table/partition-table.bin 2>/dev/null || echo "  (partition table)"
ls -la build/bootloader/bootloader.bin 2>/dev/null || echo "  (bootloader)"

# --- Step 3: Flash ---
echo ""
echo "[2/3] Flashing to $PORT at $BAUD baud..."

idf.py -p "$PORT" -b "$BAUD" flash

if [ $? -ne 0 ]; then
    echo ""
    echo "idf.py flash failed. Trying esptool.py directly..."
    echo ""

    # Fallback: use esptool.py for more control
    esptool.py --chip esp32 \
        --port "$PORT" \
        --baud "$BAUD" \
        --before default_reset \
        --after hard_reset \
        write_flash \
        0x1000  build/bootloader/bootloader.bin \
        0x8000  build/partition_table/partition-table.bin \
        0x10000 build/idk-nagger-4.bin

    if [ $? -ne 0 ]; then
        echo "ERROR: Flash failed!"
        exit 1
    fi
fi

# --- Step 4: Monitor (optional) ---
echo ""
echo "[3/3] Flash complete!"
echo ""
echo "To monitor serial output:"
echo "  idf.py -p $PORT -b $BAUD monitor"
echo ""
echo "Or with screen:"
echo "  screen $PORT 115200"
echo ""
echo "To flash + monitor in one step:"
echo "  idf.py -p $PORT -b $BAUD flash monitor"
echo ""
echo "============================================"
echo "  idk-nagger-4 is ready to mine! (lottery)"
echo "============================================"

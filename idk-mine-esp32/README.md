# idk-nagger-4 — ESP32 Lottery Miner

> **⚠️ IMPORTANT: This firmware is for educational purposes, hobby experimentation, and hardware benchmarking ONLY.**
>
> An ESP32 **cannot profitably mine Bitcoin or Litecoin**. At ~30-40 kH/s (SHA256d) or ~1-5 H/s (scrypt), the expected time to solo-mine a single Bitcoin block exceeds 10^60 years. Use testnet, simulated mode, or treat this as a learning tool.

A lightweight, headless lottery-style miner for **ESP32-WROOM-32U**, inspired by [NerdMiner v2](https://github.com/BitMaker-hub/NerdMiner_v2) and [Bitsy Miner](https://github.com/nicehash/Bitsy-miner). Supports BTC (double-SHA256) and LTC (scrypt) as selectable mining modes.

**Miner Identity:** `idk-nagger-4`
**Tested with:** esp-idf v5.2.2
**Target:** ESP32-WROOM-32U (Xtensa dual-core LX6, 240 MHz)

---

## Quick Start

### Prerequisites

- [esp-idf v5.2.2](https://docs.espressif.com/projects/esp-idf/en/v5.2.2/esp32/get-started/)
- Docker (optional, for reproducible builds)
- Python 3.8+ (for host simulator and tests)
- USB-to-serial cable for ESP32

### Build & Flash (Native)

```bash
# Source esp-idf environment
source $IDF_PATH/export.sh

# Build
idf.py build

# Flash (adjust port as needed)
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Or all-in-one:
idf.py -p /dev/ttyUSB0 flash monitor
```

### Build & Flash (Docker)

```bash
# Build the Docker image
docker build -t esp32-build-env .

# Build firmware
docker run --rm -v $(pwd):/project -w /project esp32-build-env idf.py build

# Flash (pass through USB device)
docker run --rm -v $(pwd):/project --device=/dev/ttyUSB0 -w /project \
    esp32-build-env idf.py -p /dev/ttyUSB0 flash

# Or use the convenience script:
docker run --rm -v $(pwd):/project --device=/dev/ttyUSB0 -it \
    esp32-build-env /project/scripts/build_and_flash.sh /dev/ttyUSB0 115200
```

---

## Configuration

### WiFi & Wallets

Default credentials are in `config/default_config.h`:

| Setting          | Default Value |
|------------------|---------------|
| WiFi SSID        | `Quan Le`     |
| WiFi Password    | `15032011`    |
| BTC Wallet       | `bc1qul2pvt0l5deykfznzkzhfjf3yj3cla2cssd54e` |
| LTC Wallet       | `ltc1qcl9h4kffh8w8nr77u8la5ykn4cg59ms2lteply` |
| Mining Mode      | `Simulated` (mode 2) |

**To override:** Copy `config/user_config.h.template` to `config/user_config.h` and uncomment/edit values. This file is gitignored.

```bash
cp config/user_config.h.template config/user_config.h
# Edit config/user_config.h with your values
```

### Mining Modes

| Mode | Value | Algorithm    | Description |
|------|-------|-------------|-------------|
| BTC  | 0     | SHA256d     | Bitcoin double-SHA256 |
| LTC  | 1     | scrypt      | Litecoin scrypt (N=1024, r=1, p=1) |
| SIM  | 2     | SHA256d     | Simulated pool, no network needed |

Set `DEFAULT_MINE_MODE` in your config. Default is `2` (simulated) for safe testing.

### Difficulty

`SIM_DIFFICULTY_BITS` controls how many leading zero bits a hash needs to be a valid share. Default: 16 bits (finds a share every ~65k hashes). Lower values = more frequent shares.

---

## Testing

### Run Host Unit Tests (no ESP32 needed)

```bash
# Quick way:
make test

# Or manually:
cd tests && bash run_tests.sh
```

### Run CI Tests in Docker

```bash
bash scripts/ci_tests.sh
```

### Test with Pool Simulator

1. Start the pool simulator on your PC:
   ```bash
   python3 scripts/host_simulator.py --port 3333 --difficulty 16
   ```

2. Configure the firmware to connect to your PC's IP:
   ```c
   // In config/user_config.h:
   #define POOL_HOST          "192.168.1.100"  // Your PC's IP
   #define POOL_PORT          3333
   #define POOL_USE_SIMULATED 0
   #define DEFAULT_MINE_MODE  0                 // BTC mode
   ```

3. Build, flash, and monitor serial output.

### Run Benchmarks

```bash
make bench
# Or:
cd tools/bench && bash run_bench.sh
```

---

## Expected Serial Output

At INFO log level, you'll see:

```
======================================
  idk-nagger-4 Lottery Miner
  Git commit : abc1234
  Build date : 2026-02-25T22:00:00
======================================
[INFO] ===== idk-nagger-4 v0.1.0 Configuration =====
[INFO]   WiFi SSID      : Quan Le
[INFO]   Mine mode      : Simulated
[INFO]   Pool           : 192.168.1.100:3333 (sim=1)
[INFO]   Wallet (active): bc1qul2pvt0l5deykfznzkzhfjf3yj3cla2cssd54e
[INFO]   Workers        : 2
[INFO] Free heap: 280000 bytes
[INFO] SHA-256 self-test PASSED
[INFO] Scrypt self-test: PASSED
[INFO] Connecting to WiFi 'Quan Le'...
[INFO] Connected! IP: 192.168.1.42
[INFO] Algorithm: double-SHA256 (Simulated mode)
[INFO] Using SIMULATED pool mode
[INFO] ====================================
[INFO]   idk-nagger-4 is GO!
[INFO]   Mode: SHA256d (sim)
[INFO]   Workers: 2
[INFO] ====================================
[INFO] Worker 0 started (nonce_start=0x00000000, stride=20000)
[INFO] Worker 1 started (nonce_start=0x80000000, stride=20000)
[INFO] Simulated job generated: sim_00000001
[INFO] Worker 0 found share! nonce=0x0000a3f2
[INFO] Share ACCEPTED (sim) — nonce=0x0000a3f2, job=sim_00000001
{"miner":"idk-nagger-4","algo":"SHA256d (sim)","hashrate":33250.5,"hashes":332505,"found":3,"accepted":3,"rejected":0,"uptime":10}
```

---

## Project Structure

```
idk-mine-esp32/
├── CMakeLists.txt              # Root esp-idf CMake
├── Dockerfile                  # Reproducible build env (esp-idf v5.2.2)
├── Makefile                    # Convenience targets
├── LICENSE                     # MIT
├── CHANGELOG.md
├── README.md
├── flash_report.txt            # Flash offsets and commands
├── sdkconfig.defaults          # ESP-IDF tuned defaults
├── partition_table.csv         # Custom partition layout with OTA stubs
│
├── config/
│   ├── default_config.h        # All defaults (wallets, WiFi, modes)
│   └── user_config.h.template  # User override template (gitignored when copied)
│
├── main/                       # esp-idf main component
│   ├── CMakeLists.txt          # Component registration
│   ├── app_main.c              # Entry point, boot sequence
│   ├── crypto/
│   │   ├── sha256_opt.c/.h     # HW-accelerated SHA-256
│   │   └── scrypt_wrapper.c/.h # Scrypt with memory-adaptive fallback
│   ├── miner/
│   │   ├── miner_core.c/.h     # Worker management, telemetry
│   │   ├── lottery.c/.h        # Nonce iteration, difficulty check
│   │   ├── protocol_adapter.c/.h # Pool abstraction layer
│   │   └── target_select.c/.h  # BTC/LTC algorithm selector
│   ├── net/
│   │   ├── wifi_manager.c/.h   # WiFi with exponential backoff
│   │   └── pool_client.c/.h    # Stratum v1 / simulated pool client
│   └── system/
│       ├── config.c/.h         # Runtime configuration
│       ├── build_info.c        # Git commit, build date
│       ├── flash_writer.c/.h   # Partition verification utility
│
├── scripts/
│   ├── build_and_flash.sh      # One-step build + flash
│   ├── host_simulator.py       # Test pool simulator
│   └── ci_tests.sh             # CI test runner
│
├── tests/
│   ├── test_sha256.c           # SHA-256 unit tests
│   ├── test_lottery.c          # Miner logic unit tests
│   └── run_tests.sh            # Test runner
│
├── tools/bench/
│   ├── bench_crypto.c          # Crypto throughput benchmark
│   └── run_bench.sh            # Benchmark runner
│
└── examples/
    └── webserver_status.c      # Optional HTTP status page (disabled)
```

---

## Realistic Expectations

| Metric | ESP32-WROOM-32U Value |
|--------|----------------------|
| SHA256d hash rate | ~25,000 - 40,000 H/s |
| Scrypt hash rate | ~1 - 5 H/s |
| Power consumption | ~0.5 - 1.0 W |
| BTC network hashrate | ~500 EH/s (5×10²⁰ H/s) |
| Odds of solo-mining a BTC block | ~1 in 10⁶⁴ per hash |
| Expected time to find a block | > 10⁶⁰ years |

**This is a lottery.** The code is correct and follows the real mining protocol, but the hardware is ~15 orders of magnitude too slow for practical mining. Use this for:

- ✅ Learning how Bitcoin/Litecoin mining works
- ✅ Benchmarking ESP32 crypto performance
- ✅ Testing mining pool protocols
- ✅ Hobby/novelty "lottery ticket" mining
- ❌ NOT for profit or energy-efficient mining

---

## Security Notes

> [!WARNING]
> **Plaintext Credentials:** WiFi passwords and wallet addresses are stored as plaintext `#define` values in header files. For production:
> - Move credentials to NVS encrypted storage
> - Use runtime provisioning (BLE/serial)
> - Never commit `config/user_config.h` (it's gitignored)
>
> **Wallet Safety:** Wallet addresses in this repo are receive-only. They cannot be used to steal funds. However, anyone who sees them could send mining rewards to a different address.

---

## Third-Party Code & Licenses

| Component | License | Usage |
|-----------|---------|-------|
| esp-idf | Apache 2.0 | Build system, WiFi, FreeRTOS |
| mbedTLS (bundled in esp-idf) | Apache 2.0 | SHA-256 HW acceleration |
| Scrypt (this project) | MIT | Portable C implementation |

All code in this repository is MIT licensed (SPDX: MIT). See [LICENSE](LICENSE).

---

## Advanced: Connecting to a Real Pool

To try connecting to a real Bitcoin solo mining pool:

```c
// config/user_config.h
#define POOL_HOST          "solo.ckpool.org"
#define POOL_PORT          3333
#define POOL_USE_SIMULATED 0
#define DEFAULT_MINE_MODE  0  // BTC
#define BTC_WALLET_ADDRESS "your_btc_address_here"
```

**Remember:** You will almost certainly never find a block. This is just for protocol testing.

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| `WiFi not connected` | Check SSID/password in config, verify AP is in range |
| `Scrypt OOM` | Board lacks PSRAM; firmware auto-falls back to reduced N or SIM mode |
| `Flash failed` | Check USB cable, port permissions (`sudo chmod 666 /dev/ttyUSB0`) |
| `Build error` | Ensure `IDF_PATH` is set and esp-idf v5.2.2 is installed |
| Low hash rate | Normal for ESP32. Verify 240 MHz in sdkconfig. |

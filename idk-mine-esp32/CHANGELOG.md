# Changelog

All notable changes to the **idk-nagger-4** project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [0.1.0] - 2026-02-25

### Added
- Initial project scaffolding for ESP32-WROOM-32U
- BTC (double-SHA256) lottery mining mode
- LTC (scrypt) lottery mining mode with simplified fallback
- WiFi auto-connect with exponential backoff reconnection
- Simulated pool mode for testing without real pool connection
- Host-side pool simulator (`host_simulator.py`)
- Hardware-accelerated SHA-256 using ESP32 crypto peripheral
- Serial JSON telemetry output (hashrate, submits, uptime)
- Configurable logging (ERROR, WARN, INFO, DEBUG levels)
- Docker-based reproducible build environment
- Unit tests runnable on host (x86)
- Benchmark tools for SHA-256 and scrypt throughput
- OTA partition stub for future updates
- Watchdog reset handling for stability

### Security
- WiFi credentials isolated in gitignored `config/user_config.h`
- Clear documentation about plaintext credential risks

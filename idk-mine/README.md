# idk-mine firmware suite

Four PlatformIO firmware projects for CYD-2432S028 and ESP32-WROOM-32U, with shared reusable core modules and per-project config.

## Tree
- cyd/idk-cyd-lottery
- cyd/idk-cyd-mine
- esp32-wroom-32u/idk-esp32-lottery
- esp32-wroom-32u/idk-esp32-mine
- shared/idk-mine-core
- scripts

## Shared module layout
- app: startup, task orchestration, OTA integration
- config: runtime/default config parsing and merge
- network: Wi-Fi reconnect manager and stratum client
- miner: lightweight worker engine and telemetry counters
- telemetry: snapshot model
- ui: CYD dense TFT dashboard and headless serial telemetry

## Arch Linux / fish quick commands
~~~fish
cd /home/truonglangquan/idk-code/idk-test/idk-mine
./scripts/build-all.fish
~~~

Flash one project:
~~~fish
./scripts/flash.fish cyd-lottery /dev/ttyUSB0
~~~

Flash with automatic baud fallback (recommended if upload drops):
~~~fish
./scripts/flash-safe.fish cyd-lottery /dev/ttyUSB0
~~~

Upload runtime config:
~~~fish
./scripts/upload-config.fish cyd-lottery /dev/ttyUSB0
~~~

Monitor logs:
~~~fish
./scripts/monitor.fish cyd-lottery /dev/ttyUSB0
~~~

## Important practical limitation
ESP32 cannot run full desktop-grade BTC/LTC mining algorithms and full protocol stacks at competitive hashrates. This suite implements the closest practical alternative on ESP32: stratum job ingestion, non-blocking worker loops, dense telemetry, and pseudo-share generation with config-driven pools/wallets.

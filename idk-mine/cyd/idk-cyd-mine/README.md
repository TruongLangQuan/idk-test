# idk-cyd-mine

## Target
- MCU: ESP32-WROOM-32U compatible
- Variant: Mine
- Device mode: CYD
- Miner name: idk-nagger-5

## Features
- Non-blocking architecture with separate networking, mining workers, and UI/telemetry task.
- Auto Wi-Fi credentials:
  - SSID: Quan Le
  - PASS: 15032011
- Runtime config from LittleFS path /runtime_config.json.
- Pool settings are config-driven and isolated from mining logic.
- OTA enabled with ArduinoOTA.

## Default Pools
- BTC: public-pool.io:4333 (TLS=true)
- LTC: rx.unmineable.com:443 (TLS=true)

## Build (Arch Linux, fish)
~~~fish
cd /home/truonglangquan/idk-code/idk-test/idk-mine/cyd/idk-cyd-mine
pio run
~~~

## Flash
~~~fish
cd /home/truonglangquan/idk-code/idk-test/idk-mine/cyd/idk-cyd-mine
pio run -t upload --upload-port /dev/ttyUSB0
~~~

## Upload runtime config to LittleFS
~~~fish
cd /home/truonglangquan/idk-code/idk-test/idk-mine/cyd/idk-cyd-mine
pio run -t uploadfs --upload-port /dev/ttyUSB0
~~~

## Serial monitor
~~~fish
cd /home/truonglangquan/idk-code/idk-test/idk-mine/cyd/idk-cyd-mine
pio device monitor -b 115200 --port /dev/ttyUSB0
~~~

## Troubleshooting
- If CYD display is blank/garbled, adjust TFT pin mapping in platformio.ini build flags for your board revision.
- If pool rejects shares, verify endpoint protocol and port in config/runtime_config.json.
- If OTA is not visible, ensure board and host are on the same subnet and allow mDNS plus UDP 3232.

## Limitations
- ESP32 cannot run full desktop-grade BTC/LTC miners.
- This firmware uses a lightweight ESP32-safe mining core with stratum job ingestion plus pseudo-share generation.

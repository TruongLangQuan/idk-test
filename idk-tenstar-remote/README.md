# idk-tenstar-remote

Tenstar TS-ESP32-S3 helper firmware for `idk-maze` and `idk-dice`.

- Connects to AP `dhe`
- Uses UDP to send commands to the M5StickC Plus2
- Short press on BOOT: next command
- Long press on BOOT: send current command
- Very long press: toggle `MAZE` / `DICE`

Notes:
- This board appears to have no IMU, so it acts as a button remote rather than true tilt/shake input.
- Ports:
  - Maze: `4211`
  - Dice: `4212`

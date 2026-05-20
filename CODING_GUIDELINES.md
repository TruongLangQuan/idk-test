# CODING_GUIDELINES

## Embedded-Safe Rules
- **No Dynamic Allocation in Loops:** Avoid `new`, `malloc`, or `std::string` concatenation inside tight loops (like `update()` or `loop()`). Pre-allocate buffers in `init()` or `setup()`. Memory fragmentation will cause crashes.
- **Use PSRAM Wisely:** For large display buffers (like `LGFX_Sprite` or arrays for Bad Apple/ROM emulation), allocate them in PSRAM using `sprite.setPsram(true)` or `heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`. The internal 320KB SRAM is needed for core logic and RTOS tasks.
- **Fixed-Point Math:** For complex calculations, prefer fixed-point math or pre-computed lookup tables (LUTs) if the FPU becomes a bottleneck.

## Hardware Abstraction (CRITICAL)
- **M5Unified First:** Always use `M5.Display`, `M5.BtnA`, `M5.Imu` instead of underlying specific libraries or direct GPIO reads on M5Stick devices.
- **Non-Blocking Logic:** NEVER use `delay()` for timing inside the main loop. Use `millis()`, `micros()`, or a `FrameTimer` class to manage state machines and frame rates.
- **Yield to System:** In heavy calculations (fractal generation, physics sims, crypto mining), include `delay(1)` or `vTaskDelay(1)` occasionally to prevent the ESP32 Watchdog Timer (WDT) from resetting the device.
- **5-Way Switch Conventions:** If implementing the 5-way tactile switch, use the standard GPIO mapping: UP=`32`, DOWN=`33`, LEFT=`25`, RIGHT=`26`, CENTER=`0`. Implement a debounce timer (100-200ms for directions, 300-400ms for center).

## Code Structure
- **UI/Logic Separation:** Keep rendering logic separate from game/app state.
- **Shared Libraries:** If a feature is used across multiple projects (like the Vietnamese font or common UI menus), place it in `shared/` and reference it via `lib_extra_dirs` in `platformio.ini`.
- **Landscape Orientation:** Most M5StickC Plus2 projects default to landscape orientation (`M5.Display.setRotation(3)`).

## External SPI Dependencies (CYD/Tenstar)
- **SPI Bus Contention:** Display, Touch, and SD card often share the same SPI bus (MOSI/MISO/SCK). Ensure proper Chip Select (CS) management. Do not attempt concurrent access without mutexes or proper transaction management.

# CODING_GUIDELINES

## Embedded-Safe Rules
- **No Dynamic Allocation in Loops:** Avoid `new`, `malloc`, or `std::string` concatenation inside tight loops (like `update()` or `loop()`). Pre-allocate buffers in `init()` or `setup()`.
- **Use PSRAM Wisely:** For large display buffers (like `LGFX_Sprite`), allocate them in PSRAM using `sprite.setPsram(true)` to save the 320KB SRAM for core logic.
- **Fixed-Point Math:** For complex 3D or fractal calculations, prefer fixed-point math or pre-computed lookup tables (LUTs) for sin/cos if FPU is a bottleneck.

## Hardware Abstraction
- **M5Unified First:** Always use `M5.Display`, `M5.BtnA`, `M5.Imu` instead of underlying specific libraries (like `TFT_eSPI` or direct GPIO reads).
- **Non-Blocking Logic:** NEVER use `delay()` for timing inside the main loop. Use `millis()`, `micros()`, or a `FrameTimer` class to manage state machines and frame rates.
- **Yield to System:** In heavy calculations (like Mandelbrot generation), include `delay(1)` or `vTaskDelay(1)` occasionally to prevent the ESP32 Watchdog Timer (WDT) from resetting the device.

## Memory Constraints Awareness
- **Global vs Local:** Keep stack frames small. Large arrays (like a 240x135 z-buffer) should be allocated on the heap (PSRAM) or as globals, not local variables inside a function, to avoid stack overflow crashes.

# CODING_GUIDELINES

## Embedded-safe rules
- Prefer fixed-size buffers over unbounded `String` growth in hot paths.
- Avoid heap allocation inside frame loops.
- Keep display refresh loops deterministic.
- Do not block WiFi/UDP/game loops with long delays.
- If delay is required, keep it tied to frame/update cadence.

## Memory / performance
- MJPG/GIF/text renderers must avoid repeated large allocations.
- Large generated assets belong in generated headers or FS, not ad-hoc globals.
- For subtitle/video timing, track media time separately from decode time.

## Hardware abstraction
- Use `M5Unified` APIs on M5 targets instead of duplicating low-level pin logic.
- Treat `Launcher/boards/pinouts/*` and board `interface.cpp` as the source of truth for board wiring.
- Treat Tenstar TFT pin macros/user setup as authoritative.

## Driver and I/O rules
- SD changes require re-testing mount, browse, and file open.
- WiFi changes must preserve AP/STA mode expectations per project.
- Never rename subtitle pairing convention without updating docs and UI.
- RTC changes must preserve offline fallback behavior.

## Repo conventions
- Most mini-firmwares are single-file apps with `setup()` and `loop()`.
- Use concise status strings and button hints on screen.
- Keep rotation explicit in `setup()`.
- Mark unknown hardware details as `ASSUMPTION`.

## Risk labels
- SAFE TO EDIT: project-local scene/content logic and README/docs.
- EDIT WITH CAUTION: WiFi, SD, RTC, subtitle timing, IMU math.
- DO NOT EDIT LIGHTLY: Launcher board pinouts, board interfaces, core boot flow.

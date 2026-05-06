#pragma once

#include <M5Unified.h>

namespace idk {

class FrameTimer {
public:
    FrameTimer(uint32_t targetFps = 30) {
        setTargetFps(targetFps);
        _lastMicros = micros();
    }

    void setTargetFps(uint32_t fps) {
        if (fps == 0) fps = 1;
        _framePeriodUs = 1000000 / fps;
    }

    // Returns true if enough time has passed for a new frame.
    // Automatically yields to system tasks while waiting.
    bool ready() {
        uint32_t now = micros();
        uint32_t elapsed = now - _lastMicros;

        if (elapsed < _framePeriodUs) {
            // If we have more than 2ms to wait, yield to system
            if (_framePeriodUs - elapsed > 2000) {
                delay(1);
            }
            return false;
        }

        _dtMs = elapsed / 1000.0f;
        _lastMicros = now;
        return true;
    }

    float getDeltaTimeMs() const { return _dtMs; }
    uint32_t getFramePeriodUs() const { return _framePeriodUs; }

private:
    uint32_t _lastMicros = 0;
    uint32_t _framePeriodUs = 33333; // Default 30 FPS
    float _dtMs = 0;
};

} // namespace idk

#pragma once
// Input.h - Button input handling for M5StickC Plus2
// Buttons: BtnA, BtnB, BtnPWR
//
// Press types:
//   Short  : released within BTN_SHORT_MAX_MS
//   Medium : released between BTN_MEDIUM_MIN_MS and BTN_MEDIUM_MAX_MS  (= select)
//   Long   : held >= BTN_LONG_MIN_MS
//   Double : BtnPWR pressed twice within BTN_DOUBLE_WINDOW_MS
//
// Mapping:
//   BtnA  short  = next / right / down in menu
//   BtnPWR short = prev / left / up in menu
//   BtnA  medium = confirm / select
//   BtnB  long   = back / exit
//   BtnPWR long  = (reserved / cancel)
//   BtnPWR double= open main menu from idle

#include <M5Unified.h>
#include "config.h"

enum class BtnEvent {
    None,
    ShortPress,
    MediumPress,  // select / confirm
    LongPress,
};

struct BtnState {
    bool     held        = false;
    uint32_t pressedAt   = 0;
    bool     longFired   = false; // long event already emitted while held
    BtnEvent pending     = BtnEvent::None; // set on release
};

class InputHandler {
public:
    InputHandler() {}

    void update() {
        M5.update();
        uint32_t now = millis();
        _updateBtn(_btnA,   M5.BtnA,   now);
        _updateBtn(_btnB,   M5.BtnB,   now);
        _updateBtn(_btnPWR, M5.BtnPWR, now);

        // Double-press detection for BtnPWR
        _doublePWR = false;
        if (_btnPWR.pending == BtnEvent::ShortPress) {
            uint32_t gap = now - _lastPWRReleaseMs;
            if (_waitingPWRDouble && gap <= BTN_DOUBLE_WINDOW_MS) {
                _doublePWR = true;
                _waitingPWRDouble = false;
                _btnPWR.pending = BtnEvent::None; // consume
            } else {
                _waitingPWRDouble = true;
                _lastPWRReleaseMs = now;
            }
        } else if (_waitingPWRDouble) {
            // If double window expired, promote to single short press
            if ((now - _lastPWRReleaseMs) > BTN_DOUBLE_WINDOW_MS) {
                _waitingPWRDouble = false;
                // Leave _btnPWR.pending as ShortPress (already consumed above if double)
            }
        }
    }

    // ── BtnA ──────────────────────────────────────────────────────────────
    bool btnA_shortPress()  { return _consume(_btnA, BtnEvent::ShortPress);  }
    bool btnA_mediumPress() { return _consume(_btnA, BtnEvent::MediumPress); }
    bool btnA_longPress()   { return _consume(_btnA, BtnEvent::LongPress);   }
    bool btnA_held()        { return _btnA.held; }

    // ── BtnB ──────────────────────────────────────────────────────────────
    bool btnB_shortPress()  { return _consume(_btnB, BtnEvent::ShortPress);  }
    bool btnB_mediumPress() { return _consume(_btnB, BtnEvent::MediumPress); }
    bool btnB_longPress()   { return _consume(_btnB, BtnEvent::LongPress);   }
    bool btnB_held()        { return _btnB.held; }

    // ── BtnPWR ────────────────────────────────────────────────────────────
    bool btnPWR_shortPress()  {
        // Only report short press if double-press window has expired
        if (_waitingPWRDouble) return false;
        return _consume(_btnPWR, BtnEvent::ShortPress);
    }
    bool btnPWR_mediumPress() { return _consume(_btnPWR, BtnEvent::MediumPress); }
    bool btnPWR_longPress()   { return _consume(_btnPWR, BtnEvent::LongPress);   }
    bool btnPWR_doublePress() {
        if (_doublePWR) { _doublePWR = false; return true; }
        return false;
    }
    bool btnPWR_held()        { return _btnPWR.held; }

    // ── Convenience aliases (used by scenes) ──────────────────────────────
    bool next()        { return btnB_shortPress();      } // next/down  (BtnB short)
    bool prev()        { return btnPWR_shortPress();    } // prev/up    (BtnPWR short)
    bool select()      { return btnA_shortPress();      } // confirm    (BtnA short)
    bool back()        { return btnB_longPress();       } // back/exit  (BtnB long)
    bool quickAction() { return btnA_longPress();       } // quick pet action (BtnA long)
    bool mainMenu()    { return btnPWR_doublePress();   } // open main menu (BtnPWR double)
    bool nextHeld()    { return btnB_held();            } // BtnB held (continuous)
    bool prevHeld()    { return btnPWR_held();          } // BtnPWR held (continuous)

private:
    BtnState _btnA, _btnB, _btnPWR;

    // Double-press tracking for BtnPWR
    bool     _waitingPWRDouble  = false;
    uint32_t _lastPWRReleaseMs  = 0;
    bool     _doublePWR         = false;

    template<typename T>
    void _updateBtn(BtnState& s, T& btn, uint32_t now) {
        bool pressed = btn.isPressed();

        if (pressed && !s.held) {
            // just pressed
            s.held      = true;
            s.pressedAt = now;
            s.longFired = false;
            s.pending   = BtnEvent::None;
        } else if (pressed && s.held) {
            // still held — check for long press
            uint32_t held = now - s.pressedAt;
            if (!s.longFired && held >= BTN_LONG_MIN_MS) {
                s.longFired = true;
                s.pending   = BtnEvent::LongPress;
            }
        } else if (!pressed && s.held) {
            // just released
            s.held = false;
            if (!s.longFired) {
                uint32_t held = now - s.pressedAt;
                if (held >= BTN_MEDIUM_MIN_MS && held < BTN_MEDIUM_MAX_MS) {
                    s.pending = BtnEvent::MediumPress;
                } else if (held < BTN_SHORT_MAX_MS) {
                    s.pending = BtnEvent::ShortPress;
                }
            }
        }
    }

    bool _consume(BtnState& s, BtnEvent evt) {
        if (s.pending == evt) {
            s.pending = BtnEvent::None;
            return true;
        }
        return false;
    }
};

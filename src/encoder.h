#pragma once

#include <Arduino.h>
#include "config.h"

enum EncoderEvent : uint8_t {
    ENC_NONE       = 0,
    ENC_CW         = 1,
    ENC_CCW        = 2,
    ENC_CLICK      = 3,
    ENC_LONG_CLICK = 4
};

// Interrupt-driven encoder — no delay(), never blocks loop()
class Encoder {
public:
    Encoder();
    void begin();

    // Call from loop() — returns the events accumulated so far
    EncoderEvent poll();
    bool isFast() const { return _fast; }

private:
    // ── Rotation (interrupt-driven) ───────────────────────
    int32_t  _accum;         // accumulated quadrature transitions
    uint32_t _lastDetentMs;
    bool     _fast;

    // ── Button (polled, time-based debounce) ─────────────
    int      _lastRaw;       // last raw pin reading
    int      _stableState;   // confirmed (debounced) level
    uint32_t _lastEdgeMs;    // timestamp of the last raw edge
    bool     _btnPending;
    uint32_t _btnPressMs;

    EncoderEvent _pollRotation();
    EncoderEvent _pollButton();

    // ISR part: must be static
    static void IRAM_ATTR _isr();
    static volatile int32_t _s_delta;   // transitions accumulated by the ISR
    static volatile uint8_t _s_state;   // 4 bits: previous + current state
    static portMUX_TYPE     _s_mux;
};

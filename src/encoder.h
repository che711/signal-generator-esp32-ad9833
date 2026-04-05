#pragma once

#include <Arduino.h>

// ── KY-040 pins ───────────────────────────────────────────
#define ENC_CLK_PIN  34   // input-only GPIO, no internal pull-up
#define ENC_DT_PIN   35   // input-only GPIO, no internal pull-up
#define ENC_SW_PIN   32   // button, internal pull-up OK

// ── Timing ────────────────────────────────────────────────
#define DEBOUNCE_MS        5
#define LONG_PRESS_MS    800
#define ACCEL_THRESHOLD   80   // ms between ticks → fast spin

// ── Events ────────────────────────────────────────────────
enum EncoderEvent : uint8_t {
    ENC_NONE        = 0,
    ENC_CW          = 1,   // clockwise
    ENC_CCW         = 2,   // counter-clockwise
    ENC_CLICK       = 3,   // short press
    ENC_LONG_CLICK  = 4    // long press
};

class Encoder {
public:
    Encoder();
    void begin();

    // Call in loop() — returns event or ENC_NONE
    EncoderEvent poll();

    // True if last rotation was "fast" (for acceleration)
    bool isFast() const { return _fast; }

private:
    // Rotation state
    int      _lastClk;
    uint32_t _lastTickMs;
    bool     _fast;

    // Button state
    int      _lastBtnState;
    bool     _btnPending;
    uint32_t _btnPressMs;

    EncoderEvent _pollRotation();
    EncoderEvent _pollButton();
};

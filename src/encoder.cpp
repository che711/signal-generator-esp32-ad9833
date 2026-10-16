#include "encoder.h"

volatile int32_t Encoder::_s_delta = 0;
volatile uint8_t Encoder::_s_state = 0;
portMUX_TYPE     Encoder::_s_mux   = portMUX_INITIALIZER_UNLOCKED;

// ─────────────────────────────────────────────────────────
// Table-driven quadrature decoder.
//
// Both pins (CLK+DT) are interrupt-driven, so edges are never missed
// while the OLED redraw blocks I2C for tens of milliseconds.
//
// The table index is 4 bits: (previous state << 2) | current state.
// Valid transitions yield ±1, invalid ones (bounce, missed edge) yield 0.
// That gives direction and software debounce at once — no delay() needed.
// ─────────────────────────────────────────────────────────
static const int8_t QUAD_TABLE[16] = {
     0, -1, +1,  0,
    +1,  0,  0, -1,
    -1,  0,  0, +1,
     0, +1, -1,  0
};

// KY-040: 4 quadrature transitions per detent (one click)
#define TICKS_PER_DETENT 4

void IRAM_ATTR Encoder::_isr() {
    uint8_t s = (uint8_t)((digitalRead(ENC_CLK_PIN) << 1)
                         | digitalRead(ENC_DT_PIN));
    portENTER_CRITICAL_ISR(&_s_mux);
    _s_state = (uint8_t)(((_s_state << 2) | s) & 0x0F);
    _s_delta += QUAD_TABLE[_s_state];
    portEXIT_CRITICAL_ISR(&_s_mux);
}

Encoder::Encoder()
    : _accum(0), _lastDetentMs(0), _fast(false),
      _lastRaw(HIGH), _stableState(HIGH), _lastEdgeMs(0),
      _btnPending(false), _btnPressMs(0)
{}

void Encoder::begin() {
    // GPIO 34/35 are input-only — NO internal pull-ups available!
    // The KY-040 module has its own pull-ups on CLK/DT; a bare encoder
    // needs external 10k resistors to 3.3 V.
    pinMode(ENC_CLK_PIN, INPUT);
    pinMode(ENC_DT_PIN,  INPUT);
    pinMode(ENC_SW_PIN,  INPUT_PULLUP);

    _s_state = (uint8_t)((digitalRead(ENC_CLK_PIN) << 1)
                        | digitalRead(ENC_DT_PIN));
    attachInterrupt(digitalPinToInterrupt(ENC_CLK_PIN), _isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_DT_PIN),  _isr, CHANGE);
}

// The button is polled first: _pollRotation() looks at its debounced state
// to suppress spurious ticks generated while clicking
EncoderEvent Encoder::poll() {
    EncoderEvent ev = _pollButton();
    if (ev != ENC_NONE) return ev;
    return _pollRotation();
}

EncoderEvent Encoder::_pollRotation() {
    // Atomically take what the ISR has accumulated
    portENTER_CRITICAL(&_s_mux);
    int32_t d = _s_delta;
    _s_delta = 0;
    portEXIT_CRITICAL(&_s_mux);

    // While the button is held the shaft twitches mechanically — drop those
    // spurious ticks so a click does not shift the frequency
    if (_stableState == LOW || _btnPending) { _accum = 0; return ENC_NONE; }

    _accum += d;

    int dir = 0;
    if      (_accum >=  TICKS_PER_DETENT) { _accum -= TICKS_PER_DETENT; dir = +1; }
    else if (_accum <= -TICKS_PER_DETENT) { _accum += TICKS_PER_DETENT; dir = -1; }
    else return ENC_NONE;

    uint32_t now = millis();
    _fast = ((now - _lastDetentMs) < ACCEL_THRESHOLD);
    _lastDetentMs = now;

    // If the direction comes out inverted, swap the CLK/DT wires
    // or flip the signs here.
    return (dir > 0) ? ENC_CW : ENC_CCW;
}

// Time-based debounce: any raw edge restarts the timer, and a level is
// accepted only after BTN_DEBOUNCE_MS of stability. A single re-check would
// land inside the KY-040 bounce window (5-30 ms) and lose the press.
// No blocking delay() involved.
EncoderEvent Encoder::_pollButton() {
    int raw = digitalRead(ENC_SW_PIN);
    uint32_t now = millis();

    if (raw != _lastRaw) {          // raw edge — restart the filter
        _lastRaw = raw;
        _lastEdgeMs = now;
        return ENC_NONE;
    }

    if (now - _lastEdgeMs < BTN_DEBOUNCE_MS) return ENC_NONE;

    if (raw != _stableState) {      // level is stable and has changed
        _stableState = raw;
        if (raw == LOW) {           // confirmed press
            _btnPressMs = now;
            _btnPending = true;
        } else if (_btnPending) {   // confirmed release
            _btnPending = false;
            uint32_t held = now - _btnPressMs;
            return (held >= LONG_PRESS_MS) ? ENC_LONG_CLICK : ENC_CLICK;
        }
    }
    return ENC_NONE;
}

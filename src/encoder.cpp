#include "encoder.h"

Encoder::Encoder()
    : _lastClk(HIGH),
      _lastTickMs(0),
      _fast(false),
      _lastBtnState(HIGH),
      _btnPending(false),
      _btnPressMs(0)
{}

void Encoder::begin() {
    // KY-040 has pull-ups on board — use INPUT (not INPUT_PULLUP)
    // GPIO 34/35 are input-only on ESP32, no internal pull-up available
    pinMode(ENC_CLK_PIN, INPUT);
    pinMode(ENC_DT_PIN,  INPUT);
    pinMode(ENC_SW_PIN,  INPUT_PULLUP);  // GPIO32 has internal pull-up

    _lastClk = digitalRead(ENC_CLK_PIN);
}

EncoderEvent Encoder::poll() {
    EncoderEvent ev = _pollRotation();
    if (ev != ENC_NONE) return ev;
    return _pollButton();
}

EncoderEvent Encoder::_pollRotation() {
    int clk = digitalRead(ENC_CLK_PIN);

    if (clk == _lastClk) return ENC_NONE;

    // Debounce
    delay(DEBOUNCE_MS);
    clk = digitalRead(ENC_CLK_PIN);
    if (clk == _lastClk) return ENC_NONE;

    _lastClk = clk;

    // Only trigger on falling edge
    if (clk != LOW) return ENC_NONE;

    uint32_t now = millis();
    uint32_t interval = now - _lastTickMs;
    _fast = (interval < ACCEL_THRESHOLD);
    _lastTickMs = now;

    int dt = digitalRead(ENC_DT_PIN);
    return (dt != clk) ? ENC_CW : ENC_CCW;
}

EncoderEvent Encoder::_pollButton() {
    int btn = digitalRead(ENC_SW_PIN);  // LOW when pressed
    uint32_t now = millis();

    // Pressed
    if (btn == LOW && _lastBtnState == HIGH) {
        delay(DEBOUNCE_MS);
        if (digitalRead(ENC_SW_PIN) == LOW) {
            _btnPressMs = now;
            _btnPending = true;
        }
    }

    // Released
    if (btn == HIGH && _lastBtnState == LOW && _btnPending) {
        _btnPending = false;
        _lastBtnState = btn;
        uint32_t held = now - _btnPressMs;
        if (held >= LONG_PRESS_MS) {
            return ENC_LONG_CLICK;
        } else {
            return ENC_CLICK;
        }
    }

    _lastBtnState = btn;
    return ENC_NONE;
}

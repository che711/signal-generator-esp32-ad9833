#include "encoder.h"

Encoder::Encoder()
    : _lastClk(HIGH), _lastTickMs(0), _fast(false),
      _lastBtnState(HIGH), _btnPending(false), _btnPressMs(0)
{}

void Encoder::begin() {
    // GPIO 34/35 — input-only, нет внутреннего pull-up
    // KY-040 имеет свои pull-up резисторы на плате
    pinMode(ENC_CLK_PIN, INPUT);
    pinMode(ENC_DT_PIN,  INPUT);
    pinMode(ENC_SW_PIN,  INPUT_PULLUP);
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

    delay(DEBOUNCE_MS);
    clk = digitalRead(ENC_CLK_PIN);
    if (clk == _lastClk) return ENC_NONE;

    _lastClk = clk;
    if (clk != LOW) return ENC_NONE;

    uint32_t now = millis();
    _fast = ((now - _lastTickMs) < ACCEL_THRESHOLD);
    _lastTickMs = now;

    int dt = digitalRead(ENC_DT_PIN);
    return (dt != clk) ? ENC_CW : ENC_CCW;
}

EncoderEvent Encoder::_pollButton() {
    int btn = digitalRead(ENC_SW_PIN);
    uint32_t now = millis();

    if (btn == LOW && _lastBtnState == HIGH) {
        delay(DEBOUNCE_MS);
        if (digitalRead(ENC_SW_PIN) == LOW) {
            _btnPressMs = now;
            _btnPending = true;
        }
    }

    if (btn == HIGH && _lastBtnState == LOW && _btnPending) {
        _btnPending = false;
        _lastBtnState = btn;
        uint32_t held = now - _btnPressMs;
        return (held >= LONG_PRESS_MS) ? ENC_LONG_CLICK : ENC_CLICK;
    }

    _lastBtnState = btn;
    return ENC_NONE;
}

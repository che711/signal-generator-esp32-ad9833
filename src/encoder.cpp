#include "encoder.h"

// Статические поля (ISR не может обращаться к this)
volatile int8_t   Encoder::_s_delta   = 0;
volatile uint32_t Encoder::_s_lastMs  = 0;
volatile int      Encoder::_s_lastClk = HIGH;

Encoder::Encoder()
    : _fast(false),
      _lastBtnState(HIGH), _btnPending(false), _btnPressMs(0)
{}

void Encoder::begin() {
    // GPIO 34/35 — input-only, внешние pull-up на плате KY-040
    pinMode(ENC_CLK_PIN, INPUT);
    pinMode(ENC_DT_PIN,  INPUT);
    pinMode(ENC_SW_PIN,  INPUT_PULLUP);

    _s_lastClk = digitalRead(ENC_CLK_PIN);

    // Прерывание по любому изменению CLK
    attachInterrupt(digitalPinToInterrupt(ENC_CLK_PIN), _isrClk, CHANGE);
}

// ── ISR — вызывается при любом изменении CLK ─────────────
// Максимально короткая: читаем пины, пишем delta
void IRAM_ATTR Encoder::_isrClk() {
    int clk = digitalRead(ENC_CLK_PIN);
    if (clk == _s_lastClk) return;
    _s_lastClk = clk;

    if (clk != LOW) return;   // реагируем только на falling edge

    int dt = digitalRead(ENC_DT_PIN);
    _s_delta += (dt != clk) ? 1 : -1;
    _s_lastMs = millis();
}

// ── poll() — вызывать в loop() ────────────────────────────
EncoderEvent Encoder::poll() {
    // Читаем и сбрасываем delta атомарно
    int8_t delta = 0;
    noInterrupts();
    delta    = _s_delta;
    _s_delta = 0;
    interrupts();

    if (delta != 0) {
        uint32_t now = millis();
        _fast = ((now - _s_lastMs) < ACCEL_THRESHOLD_MS);

        // Возвращаем только одно событие за вызов poll().
        // Если накопилось несколько шагов — isFast()/ACCEL_MULTIPLIER
        // уже учтёт это в main.cpp.
        return (delta > 0) ? ENC_CW : ENC_CCW;
    }

    return _pollButton();
}

// ── Кнопка — программный дебаунс без delay() ─────────────
EncoderEvent Encoder::_pollButton() {
    int btn = digitalRead(ENC_SW_PIN);
    uint32_t now = millis();

    if (btn == LOW && _lastBtnState == HIGH) {
        if (now - _btnPressMs > DEBOUNCE_MS) {   // дебаунс нажатия
            _btnPressMs = now;
            _btnPending = true;
        }
    }

    if (btn == HIGH && _lastBtnState == LOW && _btnPending) {
        _btnPending   = false;
        _lastBtnState = btn;
        uint32_t held = now - _btnPressMs;
        return (held >= LONG_PRESS_MS) ? ENC_LONG_CLICK : ENC_CLICK;
    }

    _lastBtnState = btn;
    return ENC_NONE;
}

#include "encoder.h"

volatile int32_t Encoder::_s_delta = 0;
volatile uint8_t Encoder::_s_state = 0;
portMUX_TYPE     Encoder::_s_mux   = portMUX_INITIALIZER_UNLOCKED;

// ─────────────────────────────────────────────────────────
// Квадратурный декодер на таблице переходов.
//
// Раньше энкодер опрашивался в loop(), а каждый щелчок запускал
// перерисовку OLED (~десятки мс блокировки I2C) — фронты терялись,
// вращение получалось рывками, направление определялось неверно.
//
// Теперь оба пина (CLK+DT) висят на прерываниях. Индекс таблицы —
// 4 бита: (пред. состояние << 2) | текущее. Валидные переходы дают
// ±1, невалидные (дребезг, пропуск) — 0. Это одновременно и
// направление, и программный debounce: delay() больше не нужен.
// ─────────────────────────────────────────────────────────
static const int8_t QUAD_TABLE[16] = {
     0, -1, +1,  0,
    +1,  0,  0, -1,
    -1,  0,  0, +1,
     0, +1, -1,  0
};

// KY-040: 4 квадратурных перехода на один щелчок (детент)
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
    // GPIO 34/35 — input-only, БЕЗ внутренних pull-up!
    // Модуль KY-040 имеет свои подтяжки на CLK/DT; если используется
    // "голый" энкодер — нужны внешние резисторы 10k на 3.3V.
    pinMode(ENC_CLK_PIN, INPUT);
    pinMode(ENC_DT_PIN,  INPUT);
    pinMode(ENC_SW_PIN,  INPUT_PULLUP);

    _s_state = (uint8_t)((digitalRead(ENC_CLK_PIN) << 1)
                        | digitalRead(ENC_DT_PIN));
    attachInterrupt(digitalPinToInterrupt(ENC_CLK_PIN), _isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_DT_PIN),  _isr, CHANGE);
}

EncoderEvent Encoder::poll() {
    EncoderEvent ev = _pollRotation();
    if (ev != ENC_NONE) return ev;
    return _pollButton();
}

EncoderEvent Encoder::_pollRotation() {
    // Атомарно забрать накопленное из ISR
    portENTER_CRITICAL(&_s_mux);
    int32_t d = _s_delta;
    _s_delta = 0;
    portEXIT_CRITICAL(&_s_mux);

    // Пока кнопка нажата, вал механически дёргается — отбрасываем
    // паразитные тики, чтобы клик не сдвигал частоту
    if (_stableState == LOW || _btnPending) { _accum = 0; return ENC_NONE; }

    _accum += d;

    int dir = 0;
    if      (_accum >=  TICKS_PER_DETENT) { _accum -= TICKS_PER_DETENT; dir = +1; }
    else if (_accum <= -TICKS_PER_DETENT) { _accum += TICKS_PER_DETENT; dir = -1; }
    else return ENC_NONE;

    uint32_t now = millis();
    _fast = ((now - _lastDetentMs) < ACCEL_THRESHOLD);
    _lastDetentMs = now;

    // Если направление инвертировано относительно ожидаемого —
    // поменяй местами провода CLK/DT или знаки здесь.
    return (dir > 0) ? ENC_CW : ENC_CCW;
}

// БАГ был здесь: одиночная перепроверка через 5 мс попадала на дребезг
// KY-040 (5-30 мс) — нажатие терялось целиком, клики не регистрировались,
// и режим сигнала через энкодер не переключался вовсе.
// Теперь: любой сырой фронт перезапускает таймер, состояние принимается
// только после BTN_DEBOUNCE_MS стабильности. Без блокирующих delay().
EncoderEvent Encoder::_pollButton() {
    int raw = digitalRead(ENC_SW_PIN);
    uint32_t now = millis();

    if (raw != _lastRaw) {          // сырой фронт — рестарт фильтра
        _lastRaw = raw;
        _lastEdgeMs = now;
        return ENC_NONE;
    }

    if (now - _lastEdgeMs < BTN_DEBOUNCE_MS) return ENC_NONE;

    if (raw != _stableState) {      // состояние стабильно и изменилось
        _stableState = raw;
        if (raw == LOW) {           // подтверждённое нажатие
            _btnPressMs = now;
            _btnPending = true;
        } else if (_btnPending) {   // подтверждённое отпускание
            _btnPending = false;
            uint32_t held = now - _btnPressMs;
            return (held >= LONG_PRESS_MS) ? ENC_LONG_CLICK : ENC_CLICK;
        }
    }
    return ENC_NONE;
}

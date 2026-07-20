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

// Энкодер на прерываниях — нет delay(), не блокирует loop()
class Encoder {
public:
    Encoder();
    void begin();

    // Вызывать в loop() — читает накопленные события из очереди
    EncoderEvent poll();
    bool isFast() const { return _fast; }

private:
    // ── Rotation (interrupt-driven) ───────────────────────
    int32_t  _accum;         // накопленные квадратурные переходы
    uint32_t _lastDetentMs;
    bool     _fast;

    // ── Button (polled) ───────────────────────────────────
    int      _lastBtnState;
    bool     _btnPending;
    uint32_t _btnPressMs;

    EncoderEvent _pollButton();

    // ISR-часть: должна быть static
    static void IRAM_ATTR _isr();
    static volatile int32_t _s_delta;   // переходы, накопленные ISR
    static volatile uint8_t _s_state;   // 4 бита: пред. + текущее состояние
    static portMUX_TYPE     _s_mux;
};

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
    // Прерывание на CLK
    static void IRAM_ATTR _isrClk();
    void _processRotation();

    // Состояние вращения (volatile — доступ из ISR)
    static volatile int8_t  _s_delta;     // накопленные шаги (+/-)
    static volatile uint32_t _s_lastMs;   // время последнего тика
    static volatile int      _s_lastClk;

    bool     _fast;

    // Кнопка (опрос в poll, без прерывания — SW на GPIO32 поддерживает ISR,
    // но дребезг удобнее обрабатывать программно в poll)
    int      _lastBtnState;
    bool     _btnPending;
    uint32_t _btnPressMs;

    EncoderEvent _pollButton();
};

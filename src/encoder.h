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

class Encoder {
public:
    Encoder();
    void begin();
    EncoderEvent poll();
    bool isFast() const { return _fast; }

private:
    int      _lastClk;
    uint32_t _lastTickMs;
    bool     _fast;

    int      _lastBtnState;
    bool     _btnPending;
    uint32_t _btnPressMs;

    EncoderEvent _pollRotation();
    EncoderEvent _pollButton();
};

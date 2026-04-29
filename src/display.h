#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"

class Display {
public:
    Display();
    void begin();

    // Главный экран
    void drawMain(
        const String& freqStr,
        const char*   waveLabel,
        const char*   stepLabel,
        bool          wifiOn = false
    );

    // Показать IP после подключения к WiFi
    void drawIP(const String& ip);

    // Заставка при старте
    void drawSplash();

    // Статус WiFi без перерисовки частоты (не используется в loop)
    void drawConnecting(const String& ssid);

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2;
};

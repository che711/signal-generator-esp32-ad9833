#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// ── I2C pins ──────────────────────────────────────────────
#define OLED_SDA_PIN  21
#define OLED_SCL_PIN  22
#define OLED_ADDR     0x3C   // common SSD1306 address (try 0x3D if blank)

class Display {
public:
    Display();
    void begin();

    // Main screen: frequency + waveform + step
    void drawMain(
        const String& freqStr,
        const char*   waveLabel,
        const char*   stepLabel
    );

    // Startup splash
    void drawSplash();

    // IP address display
    void drawIP(const String& ipAddr);

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2;
    void _drawWaveIcon(uint8_t x, uint8_t y, const char* wave);
};

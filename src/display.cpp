#include "display.h"

Display::Display()
    : _u8g2(U8G2_R0, /*reset=*/U8X8_PIN_NONE,
             OLED_SCL_PIN, OLED_SDA_PIN)
{}

void Display::begin() {
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    _u8g2.begin();
    _u8g2.setContrast(255);
    drawSplash();
    delay(1500);
}

void Display::drawSplash() {
    _u8g2.clearBuffer();

    _u8g2.setFont(u8g2_font_logisoso16_tr);
    _u8g2.drawStr(10, 28, "DDS GEN");

    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.drawStr(14, 44, "AD9833 + ESP32");
    _u8g2.drawStr(22, 58, "github.com/...");

    _u8g2.sendBuffer();
}

void Display::drawMain(
    const String& freqStr,
    const char*   waveLabel,
    const char*   stepLabel)
{
    _u8g2.clearBuffer();

    // ── Top bar: waveform type ─────────────────────────────
    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.setDrawColor(1);
    _u8g2.drawBox(0, 0, 128, 13);
    _u8g2.setDrawColor(0);
    _u8g2.drawStr(4, 10, "WAVE:");
    _u8g2.drawStr(36, 10, waveLabel);
    _u8g2.setDrawColor(1);

    // ── Centre: big frequency ─────────────────────────────
    _u8g2.setFont(u8g2_font_logisoso20_tf);
    // Right-align frequency string
    int w = _u8g2.getStrWidth(freqStr.c_str());
    int x = (128 - w) / 2;
    if (x < 0) x = 0;
    _u8g2.drawStr(x, 44, freqStr.c_str());

    // ── Bottom bar: step ──────────────────────────────────
    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.drawStr(4, 60, "STEP:");
    _u8g2.drawStr(36, 60, stepLabel);

    // ── Separator lines ───────────────────────────────────
    _u8g2.drawHLine(0, 14, 128);
    _u8g2.drawHLine(0, 50, 128);

    _u8g2.sendBuffer();
}

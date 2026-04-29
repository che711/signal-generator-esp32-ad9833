#include "display.h"

Display::Display()
    : _u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL_PIN, OLED_SDA_PIN)
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
    _u8g2.drawStr(8, 44, "AD9833 + ESP32");
    _u8g2.drawStr(8, 58, "v3 - WiFi + NVS");
    _u8g2.sendBuffer();
}

void Display::drawConnecting(const String& ssid) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.drawStr(4, 14, "Connecting WiFi...");
    _u8g2.drawStr(4, 30, ssid.c_str());
    _u8g2.sendBuffer();
}

void Display::drawIP(const String& ip) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.drawStr(8, 14,  "WiFi connected!");
    _u8g2.drawStr(8, 30,  "Open browser:");
    _u8g2.drawStr(4, 48,  ip.c_str());
    _u8g2.drawStr(4, 62,  "dds-gen.local");
    _u8g2.sendBuffer();
}

void Display::drawMain(
    const String& freqStr,
    const char*   waveLabel,
    const char*   stepLabel,
    bool          wifiOn)
{
    _u8g2.clearBuffer();

    // ── Верхняя полоса: форма сигнала + WiFi иконка ───────
    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.setDrawColor(1);
    _u8g2.drawBox(0, 0, 128, 13);
    _u8g2.setDrawColor(0);
    _u8g2.drawStr(4, 10, "WAVE:");
    _u8g2.drawStr(36, 10, waveLabel);
    if (wifiOn) _u8g2.drawStr(108, 10, "[W]");
    _u8g2.setDrawColor(1);

    // ── Центр: большая частота ────────────────────────────
    _u8g2.setFont(u8g2_font_logisoso20_tf);
    int w = _u8g2.getStrWidth(freqStr.c_str());
    int x = max(0, (128 - w) / 2);
    _u8g2.drawStr(x, 44, freqStr.c_str());

    // ── Нижняя полоса: шаг ───────────────────────────────
    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.drawStr(4, 60, "STEP:");
    _u8g2.drawStr(36, 60, stepLabel);

    // ── Разделители ───────────────────────────────────────
    _u8g2.drawHLine(0, 14, 128);
    _u8g2.drawHLine(0, 50, 128);

    _u8g2.sendBuffer();
}

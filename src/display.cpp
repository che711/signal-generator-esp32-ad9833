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
    _u8g2.drawStr(8, 58, "v5 - tasks + ISR");
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
    _u8g2.drawStr(8, 14, "WiFi connected!");
    _u8g2.drawStr(8, 30, "Open browser:");
    _u8g2.drawStr(4, 48, ip.c_str());
    _u8g2.drawStr(4, 62, "dds-gen.local");
    _u8g2.sendBuffer();
}

void Display::drawMain(
    const String& freqStr,
    const char*   waveLabel,
    const char*   stepLabel,
    bool          wifiOn,
    const String& ssid,
    const String& ip)
{
    _u8g2.clearBuffer();

    // Верхняя полоса: WAVE + WiFi
    _u8g2.setFont(u8g2_font_6x10_tf);
    _u8g2.setDrawColor(1);
    _u8g2.drawBox(0, 0, 128, 13);
    _u8g2.setDrawColor(0);
    _u8g2.drawStr(4, 10, "WAVE:");
    _u8g2.drawStr(36, 10, waveLabel);
    if (wifiOn) _u8g2.drawStr(107, 10, "[W]");
    _u8g2.setDrawColor(1);

    _u8g2.drawHLine(0, 13, 128);

    // Частота крупным шрифтом, по центру
    _u8g2.setFont(u8g2_font_logisoso16_tf);
    int fw = _u8g2.getStrWidth(freqStr.c_str());
    int fx = max(0, (128 - fw) / 2);
    _u8g2.drawStr(fx, 33, freqStr.c_str());

    _u8g2.drawHLine(0, 35, 128);

    // Три нижних строки
    _u8g2.setFont(u8g2_font_5x7_tf);
    _u8g2.drawStr(2, 44, "STP:");
    _u8g2.drawStr(24, 44, stepLabel);

    if (wifiOn && ssid.length() > 0) {
        _u8g2.drawStr(2, 53, "NET:");
        String s = ssid.length() > 18 ? ssid.substring(0, 18) : ssid;
        _u8g2.drawStr(24, 53, s.c_str());
    } else {
        _u8g2.drawStr(2, 53, "NET: --");
    }

    if (wifiOn && ip.length() > 0) {
        _u8g2.drawStr(2, 62, "IP: ");
        _u8g2.drawStr(24, 62, ip.c_str());
    } else {
        _u8g2.drawStr(2, 62, "IP:  --");
    }

    _u8g2.sendBuffer();
}

#include <Arduino.h>
#include "generator.h"
#include "encoder.h"
#include "display.h"
#include "webui.h"

SignalGenerator gen;
Encoder         enc;
Display         disp;
WebUI           web(gen);

uint32_t lastDrawMs  = 0;
uint32_t lastSaveMs  = 0;
bool     needRedraw  = true;

// Автосохранение через 5 сек после последнего изменения
#define AUTOSAVE_MS 5000

void setup() {
    Serial.begin(115200);
    Serial.println("[DDS] Booting v3...");

    disp.begin();
    gen.begin();    // загружает настройки из NVS
    enc.begin();

    disp.drawConnecting(WIFI_SSID);
    web.begin();

    if (web.isConnected()) {
        disp.drawIP(web.ipAddress());
        delay(3000);
    }

    needRedraw = true;
    Serial.println("[DDS] Ready");
}

void loop() {
    // Web + WiFi watchdog + CPU sampling
    web.handle();
    web.checkWiFi();
    web.updateCpuLoad();

    // Encoder
    bool changed = false;
    EncoderEvent ev = enc.poll();

    switch (ev) {
        case ENC_CW: {
            int m = enc.isFast() ? ACCEL_MULTIPLIER : 1;
            for (int i = 0; i < m; i++) gen.stepUp();
            changed = true;
            Serial.printf("[ENC] CW → %.2f Hz\n", gen.getFrequency());
            break;
        }
        case ENC_CCW: {
            int m = enc.isFast() ? ACCEL_MULTIPLIER : 1;
            for (int i = 0; i < m; i++) gen.stepDown();
            changed = true;
            Serial.printf("[ENC] CCW → %.2f Hz\n", gen.getFrequency());
            break;
        }
        case ENC_CLICK:
            gen.nextWave();
            changed = true;
            Serial.printf("[BTN] wave → %s\n", gen.waveLabel());
            break;
        case ENC_LONG_CLICK:
            gen.nextStep();
            changed = true;
            Serial.printf("[BTN] step → %s\n", gen.stepLabel());
            break;
        default:
            break;
    }

    // Запустить таймер автосохранения при любом изменении
    if (changed) {
        needRedraw = true;
        lastSaveMs = millis();
    }

    // Автосохранение: сохранить через 5 сек после последнего изменения
    if (lastSaveMs > 0 && millis() - lastSaveMs > AUTOSAVE_MS) {
        gen.saveSettings();
        lastSaveMs = 0;
    }

    // Перерисовка дисплея ~30 fps
    uint32_t now = millis();
    if (needRedraw && now - lastDrawMs > 33) {
        disp.drawMain(
            gen.freqLabel(),
            gen.waveLabel(),
            gen.stepLabel(),
            web.isConnected(),
            String(WIFI_SSID),
            web.ipAddress()
        );
        lastDrawMs = now;
        needRedraw = false;
    }
}

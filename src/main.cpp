#include <Arduino.h>
#include "generator.h"
#include "encoder.h"
#include "display.h"
#include "webui.h"

// ── Глобальные объекты ────────────────────────────────────
// Мьютекс защищает SignalGenerator от одновременного доступа
// из UI-задачи (ядро 1) и Web-задачи (ядро 0)
SemaphoreHandle_t genMutex;

SignalGenerator gen;
Encoder         enc;
Display         disp;
WebUI*          web;   // указатель — создаём после мьютекса

// ── Web-задача на ядре 0 ──────────────────────────────────
void webTask(void* param) {
    web->begin();

    for (;;) {
        web->handle();
        web->checkWiFi();
        web->updateCpuLoad();
        vTaskDelay(pdMS_TO_TICKS(1));   // уступить планировщику
    }
}

// ── setup ─────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("[DDS] Booting v5...");

    // Мьютекс создаём первым — до любых обращений к gen
    genMutex = xSemaphoreCreateMutex();

    disp.begin();

    // gen.begin() обращается к SPI и NVS — мьютекс уже готов,
    // но задача ещё не запущена, поэтому берём напрямую
    gen.begin();
    enc.begin();

    disp.drawConnecting(WIFI_SSID);

    // Создаём WebUI с мьютексом и запускаем задачу на ядре 0
    web = new WebUI(gen, genMutex);
    xTaskCreatePinnedToCore(
        webTask,
        "webTask",
        TASK_WEB_STACK,
        nullptr,
        TASK_WEB_PRIORITY,
        nullptr,
        TASK_WEB_CORE
    );

    // Небольшая пауза, чтобы webTask успел подключиться к WiFi
    // прежде чем мы перейдём к основному loop
    uint32_t waitStart = millis();
    while (!web->isConnected() && millis() - waitStart < WIFI_CONNECT_TIMEOUT_MS + 1000)
        delay(100);

    if (web->isConnected()) {
        disp.drawIP(web->ipAddress());
        delay(3000);
    }

    Serial.println("[DDS] Ready");
}

// ── loop — UI на ядре 1 ───────────────────────────────────
void loop() {
    // Web + WiFi watchdog + CPU sampling
    web.handle();
    web.checkWiFi();
    web.updateCpuLoad();

    // БАГ был здесь: изменения из веб-интерфейса не обновляли OLED
    // (needRedraw ставился только энкодером) и не автосохранялись
    if (web.consumeChanged()) {
        needRedraw = true;
        lastSaveMs = millis();
    }

    bool changed = false;
    EncoderEvent ev = enc.poll();

    switch (ev) {
        case ENC_CW: {
            int m = enc.isFast() ? ACCEL_MULTIPLIER : 1;
            if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                for (int i = 0; i < m; i++) gen.stepUp();
                Serial.printf("[ENC] CW → %.2f Hz\n", gen.getFrequency());
                xSemaphoreGive(genMutex);
            }
            changed = true;
            break;
        }
        case ENC_CCW: {
            int m = enc.isFast() ? ACCEL_MULTIPLIER : 1;
            if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                for (int i = 0; i < m; i++) gen.stepDown();
                Serial.printf("[ENC] CCW → %.2f Hz\n", gen.getFrequency());
                xSemaphoreGive(genMutex);
            }
            changed = true;
            break;
        }
        case ENC_CLICK:
            if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                gen.nextWave();
                Serial.printf("[BTN] wave → %s\n", gen.waveLabel());
                xSemaphoreGive(genMutex);
            }
            changed = true;
            break;
        case ENC_LONG_CLICK:
            if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                gen.nextStep();
                Serial.printf("[BTN] step → %s\n", gen.stepLabel());
                xSemaphoreGive(genMutex);
            }
            changed = true;
            break;
        default:
            break;
    }

    if (changed) {
        needRedraw = true;
        lastSaveMs = millis();
    }

    // Автосохранение через AUTOSAVE_MS после последнего изменения
    if (lastSaveMs > 0 && millis() - lastSaveMs > AUTOSAVE_MS) {
        if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            gen.saveSettings();
            xSemaphoreGive(genMutex);
        }
        lastSaveMs = 0;
    }

    // Перерисовка дисплея ~30 fps, только при изменениях
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

    // Небольшая задержка чтобы не монополизировать ядро 1
    vTaskDelay(pdMS_TO_TICKS(5));
}

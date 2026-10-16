#include <Arduino.h>
#include "generator.h"
#include "encoder.h"
#include "display.h"
#include "webui.h"

// ── Globals ───────────────────────────────────────────────
// The mutex guards SignalGenerator against concurrent access
// from the UI task (core 1) and the web task (core 0)
SemaphoreHandle_t genMutex;

SignalGenerator gen;
Encoder         enc;
Display         disp;
WebUI*          web;   // pointer — constructed after the mutex exists

// ── Web task on core 0 ────────────────────────────────────
void webTask(void* param) {
    web->begin();

    for (;;) {
        web->handle();
        web->checkWiFi();
        web->updateCpuLoad();
        vTaskDelay(pdMS_TO_TICKS(1));   // yield to the scheduler
    }
}

// ── setup ─────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("[DDS] Booting...");

    // Create the mutex first — before anything touches gen
    genMutex = xSemaphoreCreateMutex();

    disp.begin();

    // gen.begin() talks to SPI and NVS; the web task is not running yet,
    // so it is safe to call without taking the mutex
    gen.begin();
    enc.begin();

    disp.drawConnecting(WIFI_SSID);

    // Create the WebUI with the mutex and start its task on core 0
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

    // Give webTask a moment to bring up WiFi before entering the main loop
    uint32_t waitStart = millis();
    while (!web->isConnected() && millis() - waitStart < WIFI_CONNECT_TIMEOUT_MS + 1000)
        delay(100);

    if (web->isConnected()) {
        disp.drawIP(web->ipAddress());
        delay(3000);
    }

    Serial.println("[DDS] Ready");
}

// ── loop — UI on core 1 ───────────────────────────────────
// Web/WiFi/CPU are serviced by webTask on core 0
static bool     needRedraw = true;
static uint32_t lastSaveMs = 0;
static uint32_t lastDrawMs = 0;

void loop() {
    // Changes made from the web UI must also refresh the OLED and
    // restart the autosave timer
    if (web->consumeChanged()) {
        needRedraw = true;
        lastSaveMs = millis();
    }

    // Sweep: each tick moves the frequency. sweepActive() is read without
    // the mutex — it is a lone bool, worst case is one extra/late take
    if (gen.sweepActive()) {
        if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            if (gen.sweepTick(millis())) needRedraw = true;
            xSemaphoreGive(genMutex);
        }
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

    // Autosave AUTOSAVE_MS after the last change
    if (lastSaveMs > 0 && millis() - lastSaveMs > AUTOSAVE_MS) {
        if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            gen.saveSettings();
            xSemaphoreGive(genMutex);
        }
        lastSaveMs = 0;
    }

    // Redraw at ~30 fps, and only when something changed
    uint32_t now = millis();
    if (needRedraw && now - lastDrawMs > 33) {
        // Snapshot the state under the mutex, then draw without it:
        // sendBuffer() over I2C takes ~22 ms, far too long to hold the
        // mutex while the web task on core 0 may want it.
        String freqStr, waveStr, stepStr;
        if (xSemaphoreTake(genMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            freqStr = gen.freqLabel();
            // Output disabled — show OFF instead of the waveform
            waveStr = gen.getOutput() ? gen.waveLabel() : "OFF";
            stepStr = gen.stepLabel();
            xSemaphoreGive(genMutex);

            disp.drawMain(
                freqStr,
                waveStr.c_str(),
                stepStr.c_str(),
                web->isConnected(),
                String(WIFI_SSID),
                web->ipAddress()
            );
            lastDrawMs = now;
            needRedraw = false;
        }
        // mutex not acquired → needRedraw stays true, redraw next iteration
    }

    // Short delay so we do not hog core 1
    vTaskDelay(pdMS_TO_TICKS(5));
}

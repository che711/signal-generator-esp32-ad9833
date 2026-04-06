#include <Arduino.h>
#include "generator.h"
#include "encoder.h"
#include "display.h"
#include "webui.h"

// ── Objects ───────────────────────────────────────────────
SignalGenerator gen;
Encoder         enc;
Display         disp;
WebUI           web(gen);

// ── Acceleration multiplier for fast spin ─────────────────
static const int ACCEL_MULTIPLIER = 10;

// ── Display refresh rate limiter ──────────────────────────
uint32_t lastDrawMs = 0;
bool     needRedraw = true;

// ─────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    Serial.println("[DDS Generator] booting...");

    disp.begin();
    gen.begin();
    enc.begin();

    web.begin();

    if (web.isConnected()) {
        disp.drawIP(web.ipAddress());
        delay(3000);
    }

    Serial.println("[DDS Generator] ready");
    needRedraw = true;
}

void loop() {
    web.handle();

    // ── Heartbeat tick — must run every loop ──────────────
    gen.tickHeartbeat();

    // ── Encoder ───────────────────────────────────────────
    EncoderEvent ev = enc.poll();

    if (gen.isHeartbeat()) {
        // In heartbeat mode: encoder changes BPM, long-press still cycles step
        switch (ev) {
            case ENC_CW: {
                int delta = enc.isFast() ? 5 : 1;
                gen.setBPM(gen.getBPM() + delta);
                needRedraw = true;
                Serial.printf("[ENC] BPM → %d\n", gen.getBPM());
                break;
            }
            case ENC_CCW: {
                int delta = enc.isFast() ? 5 : 1;
                gen.setBPM(gen.getBPM() - delta);
                needRedraw = true;
                Serial.printf("[ENC] BPM → %d\n", gen.getBPM());
                break;
            }
            case ENC_CLICK:
                gen.nextWave();
                needRedraw = true;
                Serial.printf("[BTN] wave → %s\n", gen.waveLabel());
                break;

            case ENC_LONG_CLICK:
                // long press in heartbeat mode has no step effect; reserved
                break;

            default:
                break;
        }
    } else {
        // Normal mode
        switch (ev) {
            case ENC_CW: {
                int mult = enc.isFast() ? ACCEL_MULTIPLIER : 1;
                for (int i = 0; i < mult; i++) gen.stepUp();
                needRedraw = true;
                Serial.printf("[ENC] CW  → %.2f Hz\n", gen.getFrequency());
                break;
            }
            case ENC_CCW: {
                int mult = enc.isFast() ? ACCEL_MULTIPLIER : 1;
                for (int i = 0; i < mult; i++) gen.stepDown();
                needRedraw = true;
                Serial.printf("[ENC] CCW → %.2f Hz\n", gen.getFrequency());
                break;
            }
            case ENC_CLICK:
                gen.nextWave();
                needRedraw = true;
                Serial.printf("[BTN] wave → %s\n", gen.waveLabel());
                break;

            case ENC_LONG_CLICK:
                gen.nextStep();
                needRedraw = true;
                Serial.printf("[BTN] step → %s\n", gen.stepLabel());
                break;

            default:
                break;
        }
    }

    // ── Display (throttled to ~30 fps) ────────────────────
    uint32_t now = millis();
    if (needRedraw && (now - lastDrawMs > 33)) {
        disp.drawMain(
            gen.freqLabel(),
            gen.waveLabel(),
            gen.stepLabel()
        );
        lastDrawMs = now;
        needRedraw = false;
    }
}

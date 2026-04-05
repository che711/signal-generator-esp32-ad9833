#include <Arduino.h>
#include "generator.h"
#include "encoder.h"
#include "display.h"

// ── Objects ───────────────────────────────────────────────
SignalGenerator gen;
Encoder         enc;
Display         disp;

// ── Acceleration multiplier for fast spin ────────────────
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

    Serial.println("[DDS Generator] ready");
    needRedraw = true;
}

void loop() {
    EncoderEvent ev = enc.poll();

    switch (ev) {

        case ENC_CW: {
            // Clockwise → frequency up
            // Fast spin = bigger step
            int mult = enc.isFast() ? ACCEL_MULTIPLIER : 1;
            for (int i = 0; i < mult; i++) gen.stepUp();
            needRedraw = true;
            Serial.printf("[ENC] CW  → %.2f Hz\n", gen.getFrequency());
            break;
        }

        case ENC_CCW: {
            // Counter-clockwise → frequency down
            int mult = enc.isFast() ? ACCEL_MULTIPLIER : 1;
            for (int i = 0; i < mult; i++) gen.stepDown();
            needRedraw = true;
            Serial.printf("[ENC] CCW → %.2f Hz\n", gen.getFrequency());
            break;
        }

        case ENC_CLICK:
            // Short press → cycle waveform
            gen.nextWave();
            needRedraw = true;
            Serial.printf("[BTN] wave → %s\n", gen.waveLabel());
            break;

        case ENC_LONG_CLICK:
            // Long press → cycle frequency step
            gen.nextStep();
            needRedraw = true;
            Serial.printf("[BTN] step → %s\n", gen.stepLabel());
            break;

        default:
            break;
    }

    // Redraw display (throttled to max ~30 fps)
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

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

    // WiFi + Web server
    web.begin();

    // Show IP on OLED for 3 seconds
    if (web.isConnected()) {
        disp.drawIP(web.ipAddress());
        delay(3000);
    }

    Serial.println("[DDS Generator] ready");
    needRedraw = true;
}

void loop() {
    // Handle web requests first
    web.handle();

    // Handle encoder
    EncoderEvent ev = enc.poll();

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

    // Redraw display (throttled to ~30 fps)
    uint32_t now = millis();
    if (needRedraw && (now - lastDrawMs > 33)) {
        disp.drawMain(
            gen.freqLabel(),
            gen.waveLabel(),
            gen.stepLabel(),
            web.isConnected()
        );
        lastDrawMs = now;
        needRedraw = false;
    }
}

#include "generator.h"

static const float STEP_HZ_TABLE[] = {
    0.1f, 1.0f, 10.0f, 100.0f,
    1000.0f, 10000.0f, 100000.0f, 1000000.0f
};
static const char* STEP_LABELS[] = {
    "0.1Hz","1Hz","10Hz","100Hz",
    "1kHz","10kHz","100kHz","1MHz"
};
static const char* WAVE_LABELS[] = {
    "SINE","TRI","SQR","SQR/2"
};

SignalGenerator::SignalGenerator()
    : _dds(GEN_CS_PIN),
      _freq(1000.0f),
      _wave(WAVE_SINE),
      _step(STEP_1KHZ)
{}

void SignalGenerator::begin() {
    // MISO is unused (the AD9833 is write-only), and the default MISO pin
    // GPIO12 is a strapping pin (MTDI): a pull-up on it at boot switches the
    // flash voltage and can brick the boot. Hence MISO = -1.
    SPI.begin(GEN_SCK_PIN, -1, GEN_MOSI_PIN, GEN_CS_PIN);
    _dds.begin();
    _loadSettings();
    _dds.setFrequency(_freq);
    if (_outOn) _applyWave();
    else        _dds.setWave(AD9833_OFF);
    Serial.printf("[GEN] freq=%.2fHz wave=%d step=%d\n",
                  _freq, (int)_wave, (int)_step);
}

// ── NVS ───────────────────────────────────────────────────

void SignalGenerator::_loadSettings() {
    _prefs.begin(NVS_NAMESPACE, true);
    float    f = _prefs.getFloat("freq", 1000.0f);
    uint8_t  w = _prefs.getUChar("wave", 0);
    uint8_t  s = _prefs.getUChar("step", 4);
    _prefs.end();

    _freq = constrain(f, FREQ_MIN, FREQ_MAX);
    _wave = (w < WAVE_COUNT) ? (WaveType)w : WAVE_SINE;
    _step = (s < STEP_COUNT) ? (FreqStep)s : STEP_1KHZ;
    Serial.println("[GEN] Settings loaded from NVS");
}

void SignalGenerator::saveSettings() {
    _prefs.begin(NVS_NAMESPACE, false);
    bool changed = false;

    // Write only what actually changed — saves flash erase/write cycles
    if (_prefs.getFloat("freq", -1.0f) != _freq)
        { _prefs.putFloat("freq", _freq); changed = true; }
    if (_prefs.getUChar("wave", 255) != (uint8_t)_wave)
        { _prefs.putUChar("wave", (uint8_t)_wave); changed = true; }
    if (_prefs.getUChar("step", 255) != (uint8_t)_step)
        { _prefs.putUChar("step", (uint8_t)_step); changed = true; }

    _prefs.end();
    if (changed) Serial.println("[GEN] Settings saved to NVS");
    else         Serial.println("[GEN] NVS: nothing changed, skip write");
}

// ── Frequency ─────────────────────────────────────────────

float SignalGenerator::setFrequency(float hz) {
    _swActive = false;          // manual change cancels a running sweep
    _applyFreq(hz);
    return _freq;
}

void SignalGenerator::_applyFreq(float hz) {
    hz = constrain(hz, FREQ_MIN, FREQ_MAX);
    if (hz == _freq) return;
    _freq = hz;
    _dds.setFrequency(_freq);
}

void SignalGenerator::stepUp()   { setFrequency(_freq + getStepHz()); }
void SignalGenerator::stepDown() { setFrequency(_freq - getStepHz()); }

// ── Sweep ─────────────────────────────────────────────────
//
// Every SWEEP_TICK_MS the frequency is recomputed from progress p = t/T:
//   lin: f = f0 + (f1 - f0) * p          — equal steps in hertz
//   log: f = f0 * (f1/f0)^p              — equal steps in octaves; the right
//        mode for frequency response, since decades 10→100→1k→10k Hz each
//        take the same time, as on a Bode plot
// The AD9833 changes frequency without a phase discontinuity, so the output
// stays clean — no clicks.

bool SignalGenerator::sweepStart(float f0, float f1, uint32_t durMs, bool logMode) {
    if (f0 < FREQ_MIN || f0 > FREQ_MAX) return false;
    if (f1 < FREQ_MIN || f1 > FREQ_MAX) return false;
    if (f0 == f1)                       return false;
    if (durMs < SWEEP_MIN_MS || durMs > SWEEP_MAX_MS) return false;

    _swRetFreq = _freq;         // remember where to return when the sweep ends
    _swF0      = f0;
    _swF1      = f1;
    _swDurMs   = durMs;
    _swLog     = logMode;
    _swStartMs = millis();
    _swTickMs  = 0;
    _swActive  = true;
    _applyFreq(f0);
    Serial.printf("[GEN] sweep %s %.1f -> %.1f Hz, %lu ms\n",
                  logMode ? "log" : "lin", f0, f1, (unsigned long)durMs);
    return true;
}

void SignalGenerator::sweepStop() {
    if (!_swActive) return;
    _swActive = false;
    Serial.printf("[GEN] sweep stopped at %.2f Hz\n", _freq);
}

bool SignalGenerator::sweepTick(uint32_t nowMs) {
    if (!_swActive) return false;
    if (nowMs - _swTickMs < SWEEP_TICK_MS) return false;
    _swTickMs = nowMs;

    float p = (float)(nowMs - _swStartMs) / (float)_swDurMs;
    if (p >= 1.0f) {                    // finished: restore the pre-sweep freq.
        _swActive = false;              // A manual /sweep/stop instead keeps the
        _applyFreq(_swRetFreq);         // current one, to catch a point by hand
        Serial.printf("[GEN] sweep done, back to %.2f Hz\n", _freq);
        return true;
    }

    float f = _swLog
        ? _swF0 * powf(_swF1 / _swF0, p)
        : _swF0 + (_swF1 - _swF0) * p;
    _applyFreq(f);
    return true;
}

int SignalGenerator::sweepProgress() const {
    if (!_swActive) return 0;
    uint32_t el = millis() - _swStartMs;
    if (el >= _swDurMs) return 100;
    return (int)(el * 100UL / _swDurMs);
}

// ── Output ────────────────────────────────────────────────

void SignalGenerator::setOutput(bool on) {
    if (on == _outOn) return;
    _outOn = on;
    if (on) {
        _applyWave();               // restore the selected waveform
    } else {
        _swActive = false;          // OFF also kills a running sweep
        _dds.setWave(AD9833_OFF);   // sleep: DAC + MCLK, output ~0 V
    }
    Serial.printf("[GEN] output %s\n", on ? "ON" : "OFF");
}

// ── Wave ──────────────────────────────────────────────────

// Set directly by index. Negative/out-of-range idx is ignored: in C++
// (-1 % 4) == -1, which would index past the start of the label tables.
void SignalGenerator::setWaveByIndex(int idx) {
    if (idx < 0 || idx >= WAVE_COUNT) return;
    _wave = (WaveType)idx;
    _applyWave();
}

void SignalGenerator::nextWave() {
    setWaveByIndex((_wave + 1) % WAVE_COUNT);
}

void SignalGenerator::_applyWave() {
    if (!_outOn) return;            // waveform is remembered, applied on ON
    switch (_wave) {
        case WAVE_SINE:     _dds.setWave(AD9833_SINE);     break;
        case WAVE_TRIANGLE: _dds.setWave(AD9833_TRIANGLE); break;
        case WAVE_SQUARE:   _dds.setWave(AD9833_SQUARE1);  break;
        case WAVE_SQUARE2:  _dds.setWave(AD9833_SQUARE2);  break;
        default:            _dds.setWave(AD9833_SINE);     break;
    }
}

// ── Step ──────────────────────────────────────────────────

void SignalGenerator::setStepByIndex(int idx) {
    if (idx < 0 || idx >= STEP_COUNT) return;
    _step = (FreqStep)idx;
}

void SignalGenerator::nextStep() {
    setStepByIndex((_step + 1) % STEP_COUNT);
}

// ── Labels ────────────────────────────────────────────────

float SignalGenerator::getStepHz() const {
    return STEP_HZ_TABLE[_step];
}

const char* SignalGenerator::waveLabel() const {
    return WAVE_LABELS[_wave];
}

const char* SignalGenerator::stepLabel() const {
    return STEP_LABELS[_step];
}

String SignalGenerator::freqLabel() const {
    float f = _freq;
    if      (f >= 1000000.0f) return String(f / 1000000.0f, 4) + " MHz";
    else if (f >= 1000.0f)    return String(f / 1000.0f,    3) + " kHz";
    // Below 1 kHz the fractional part matters (step can be 0.1 Hz),
    // so 123.5 Hz must not be shown as "123 Hz"
    float frac = f - (long)f;
    if (frac > 0.001f) return String(f, 1) + " Hz";
    return String((long)f) + " Hz";
}

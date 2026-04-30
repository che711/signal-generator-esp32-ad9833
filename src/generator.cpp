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

// AD9833 v0.4.x wave constants
#define AD9833_SINE     0
#define AD9833_SQUARE1  2
#define AD9833_SQUARE2  3
#define AD9833_TRIANGLE 4

SignalGenerator::SignalGenerator()
    : _dds(GEN_CS_PIN),
      _freq(1000.0f),
      _wave(WAVE_SINE),
      _step(STEP_1KHZ),
      _lastSaveMs(0)
{}

void SignalGenerator::begin() {
    SPI.begin(GEN_SCK_PIN, 12, GEN_MOSI_PIN, GEN_CS_PIN);
    _dds.begin();
    _loadSettings();
    _dds.setFrequency(_freq);
    _applyWave();
    Serial.printf("[GEN] freq=%.2fHz wave=%d step=%d\n",
                  _freq, (int)_wave, (int)_step);
}

// ── NVS ───────────────────────────────────────────────────

void SignalGenerator::_loadSettings() {
    _prefs.begin(NVS_NAMESPACE, true);  // read-only
    _freq = _prefs.getFloat("freq", 1000.0f);
    _wave = (WaveType)_prefs.getUChar("wave", 0);
    _step = (FreqStep)_prefs.getUChar("step", 4);
    _prefs.end();

    // Clamp loaded values
    _freq = constrain(_freq, FREQ_MIN, FREQ_MAX);
    if ((int)_wave >= WAVE_COUNT) _wave = WAVE_SINE;
    if ((int)_step >= STEP_COUNT) _step = STEP_1KHZ;
    Serial.println("[GEN] Settings loaded from NVS");
}

void SignalGenerator::saveSettings() {
    _prefs.begin(NVS_NAMESPACE, false);  // read-write
    _prefs.putFloat("freq", _freq);
    _prefs.putUChar("wave", (uint8_t)_wave);
    _prefs.putUChar("step", (uint8_t)_step);
    _prefs.end();
    Serial.println("[GEN] Settings saved to NVS");
}

// ── Frequency ─────────────────────────────────────────────

void SignalGenerator::setFrequency(float hz) {
    hz = constrain(hz, FREQ_MIN, FREQ_MAX);
    _freq = hz;
    _dds.setFrequency(_freq);
}

void SignalGenerator::stepUp()   { setFrequency(_freq + getStepHz()); }
void SignalGenerator::stepDown() { setFrequency(_freq - getStepHz()); }

// ── Wave ──────────────────────────────────────────────────

// ИСПРАВЛЕНО: прямая установка по индексу, без цикла
void SignalGenerator::setWaveByIndex(int idx) {
    _wave = (WaveType)(idx % WAVE_COUNT);
    _applyWave();
}

void SignalGenerator::nextWave() {
    _wave = (WaveType)((_wave + 1) % WAVE_COUNT);
    _applyWave();
}

void SignalGenerator::_applyWave() {
    switch (_wave) {
        case WAVE_SINE:     _dds.setWave(AD9833_SINE);     break;
        case WAVE_TRIANGLE: _dds.setWave(AD9833_TRIANGLE); break;
        case WAVE_SQUARE:   _dds.setWave(AD9833_SQUARE1);  break;
        case WAVE_SQUARE2:  _dds.setWave(AD9833_SQUARE2);  break;
        default:            _dds.setWave(AD9833_SINE);     break;
    }
}

// ── Step ──────────────────────────────────────────────────

// ИСПРАВЛЕНО: прямая установка по индексу, без цикла
void SignalGenerator::setStepByIndex(int idx) {
    _step = (FreqStep)(idx % STEP_COUNT);
}

void SignalGenerator::nextStep() {
    _step = (FreqStep)((_step + 1) % STEP_COUNT);
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
    else if (f < 1.0f)        return String(f, 1) + " Hz";
    else                      return String((long)f) + " Hz";
}

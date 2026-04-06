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
    "SINE", "TRI", "SQR", "SQR/2", "BEAT"
};

// ── Heartbeat pulse timings (ms) ──────────────────────────
//   LUB: first heart sound, ~50 ms, lower frequency
//   PAUSE1: brief gap between the two sounds, ~80 ms
//   DUB: second heart sound, ~35 ms, higher frequency
//   PAUSE2: diastolic rest — whatever is left to fill one beat
static const uint32_t HB_LUB_MS    = 50;
static const uint32_t HB_PAUSE1_MS = 80;
static const uint32_t HB_DUB_MS    = 35;

static const float HB_LUB_FREQ = 40.0f;   // Hz — "lub" tone
static const float HB_DUB_FREQ = 55.0f;   // Hz — "dub" tone

// ─────────────────────────────────────────────────────────

SignalGenerator::SignalGenerator()
    : _dds(GEN_CS_PIN),
      _freq(1000.0f),
      _wave(WAVE_SINE),
      _step(STEP_1KHZ),
      _bpm(BPM_DEFAULT),
      _hbState(HB_LUB),
      _hbStateMs(0)
{}

void SignalGenerator::begin() {
    SPI.begin(GEN_SCK_PIN, 12, GEN_MOSI_PIN, GEN_CS_PIN);
    _dds.begin();
    _dds.setFrequency(_freq);
    _applyWave();
}

// ── Frequency ─────────────────────────────────────────────

void SignalGenerator::setFrequency(float hz) {
    hz = constrain(hz, FREQ_MIN, FREQ_MAX);
    _freq = hz;
    _dds.setFrequency(_freq);
}

void SignalGenerator::stepUp()   { setFrequency(_freq + getStepHz()); }
void SignalGenerator::stepDown() { setFrequency(_freq - getStepHz()); }

// ── Waveform / step cycling ───────────────────────────────

void SignalGenerator::nextWave() {
    _wave = static_cast<WaveType>((_wave + 1) % WAVE_COUNT);

    if (_wave == WAVE_HEARTBEAT) {
        // entering heartbeat — silence output until first LUB
        _hbState   = HB_LUB;
        _hbStateMs = millis();
        _ddsOutput(false);
    } else {
        _applyWave();
    }
}

void SignalGenerator::nextStep() {
    _step = static_cast<FreqStep>((_step + 1) % STEP_COUNT);
}

// ── BPM ───────────────────────────────────────────────────

void SignalGenerator::setBPM(int bpm) {
    _bpm = constrain(bpm, BPM_MIN, BPM_MAX);
}

// ── Heartbeat tick (call every loop()) ───────────────────

void SignalGenerator::tickHeartbeat() {
    if (_wave != WAVE_HEARTBEAT) return;

    uint32_t now     = millis();
    uint32_t elapsed = now - _hbStateMs;

    // Total beat period in ms, minus the three fixed segments
    uint32_t beatMs   = 60000UL / (uint32_t)_bpm;
    uint32_t pause2Ms = beatMs - HB_LUB_MS - HB_PAUSE1_MS - HB_DUB_MS;
    if (pause2Ms < 10) pause2Ms = 10;  // safety floor

    switch (_hbState) {

        case HB_LUB:
            _dds.setFrequency(HB_LUB_FREQ, 0);
            _ddsOutput(true);
            if (elapsed >= HB_LUB_MS) {
                _ddsOutput(false);
                _hbState   = HB_PAUSE1;
                _hbStateMs = now;
            }
            break;

        case HB_PAUSE1:
            _ddsOutput(false);
            if (elapsed >= HB_PAUSE1_MS) {
                _hbState   = HB_DUB;
                _hbStateMs = now;
            }
            break;

        case HB_DUB:
            _dds.setFrequency(HB_DUB_FREQ);
            _ddsOutput(true);
            if (elapsed >= HB_DUB_MS) {
                _ddsOutput(false);
                _hbState   = HB_PAUSE2;
                _hbStateMs = now;
            }
            break;

        case HB_PAUSE2:
            _ddsOutput(false);
            if (elapsed >= pause2Ms) {
                _hbState   = HB_LUB;
                _hbStateMs = now;
            }
            break;
    }
}

// ── Labels ────────────────────────────────────────────────

float SignalGenerator::getStepHz() const { return STEP_HZ_TABLE[_step]; }
const char* SignalGenerator::waveLabel() const { return WAVE_LABELS[_wave]; }
const char* SignalGenerator::stepLabel() const { return STEP_LABELS[_step]; }

String SignalGenerator::freqLabel() const {
    if (_wave == WAVE_HEARTBEAT) {
        return String(_bpm) + " BPM";
    }
    float f = _freq;
    if      (f >= 1000000.0f) return String(f / 1000000.0f, 4) + " MHz";
    else if (f >= 1000.0f)    return String(f / 1000.0f,    3) + " kHz";
    else if (f < 1.0f)        return String(f, 1) + " Hz";
    else                      return String((long)f) + " Hz";
}

// ── Private helpers ───────────────────────────────────────

void SignalGenerator::_applyWave() {
    switch (_wave) {
        case WAVE_SINE:      _dds.setWave(0); break;  // AD9833_SINE
        case WAVE_TRIANGLE:  _dds.setWave(4); break;  // AD9833_TRIANGLE
        case WAVE_SQUARE:    _dds.setWave(2); break;  // AD9833_SQUARE1
        case WAVE_SQUARE2:   _dds.setWave(3); break;  // AD9833_SQUARE2
        case WAVE_HEARTBEAT: /* managed by tickHeartbeat */ break;
        default:             _dds.setWave(0); break;
    }
}

void SignalGenerator::_ddsOutput(bool on) {
    if (on) {
        _dds.setWave(1);   // AD9833_SINE — включить, частота уже установлена
    } else {
        _dds.setWave(0);   // AD9833_OFF — DAC на ноль, без глитча
    }
}

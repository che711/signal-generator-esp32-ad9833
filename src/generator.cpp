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

// ── ECG segment timings (ms) ──────────────────────────────
//  Fixed segments sum: 80+100+50+110+160 = 500 ms
//  At 70 BPM: period = 857 ms → TP_REST = 357 ms
//  At 30 BPM: period = 2000ms → TP_REST = 1500ms
//  At 200 BPM: period = 300ms → TP_REST must not go negative:
//    floor at 10 ms

static const uint32_t HB_P_MS     =  80;   // P-wave duration
static const uint32_t HB_PQ_MS    = 100;   // PQ silence
static const uint32_t HB_QRS_MS   =  50;   // QRS spike duration
static const uint32_t HB_ST_MS    = 110;   // ST silence
static const uint32_t HB_T_MS     = 160;   // T-wave duration
static const uint32_t HB_FIXED_MS = HB_P_MS + HB_PQ_MS + HB_QRS_MS + HB_ST_MS + HB_T_MS; // 500

// ECG segment frequencies
static const float HB_P_FREQ   =  6.0f;   // P-wave:  slow, small
static const float HB_QRS_FREQ = 28.0f;   // QRS:     fast, sharp
static const float HB_T_FREQ   =  5.0f;   // T-wave:  slow, broad

// ─────────────────────────────────────────────────────────

SignalGenerator::SignalGenerator()
    : _dds(GEN_CS_PIN),
      _freq(1000.0f),
      _wave(WAVE_SINE),
      _step(STEP_1KHZ),
      _bpm(BPM_DEFAULT),
      _hbState(HB_TP_REST),
      _hbStateMs(0)
{}

void SignalGenerator::begin() {
    SPI.begin(GEN_SCK_PIN, 12, GEN_MOSI_PIN, GEN_CS_PIN);
    _dds.begin();
    _dds.setFrequency(_freq, 0);
    _applyWave();
}

// ── Frequency ─────────────────────────────────────────────

void SignalGenerator::setFrequency(float hz) {
    hz = constrain(hz, FREQ_MIN, FREQ_MAX);
    _freq = hz;
    _dds.setFrequency(_freq, 0);
}

void SignalGenerator::stepUp()   { setFrequency(_freq + getStepHz()); }
void SignalGenerator::stepDown() { setFrequency(_freq - getStepHz()); }

// ── Waveform / step ───────────────────────────────────────

void SignalGenerator::nextWave() {
    _wave = static_cast<WaveType>((_wave + 1) % WAVE_COUNT);

    if (_wave == WAVE_HEARTBEAT) {
        _hbState   = HB_P_WAVE;
        _hbStateMs = millis();
        _hbSilence();
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

// ── ECG tick (call every loop()) ──────────────────────────

void SignalGenerator::tickHeartbeat() {
    if (_wave != WAVE_HEARTBEAT) return;

    uint32_t now     = millis();
    uint32_t elapsed = now - _hbStateMs;

    // TP_REST duration = what's left after fixed segments
    uint32_t beatMs   = 60000UL / (uint32_t)_bpm;
    uint32_t tpRestMs = (beatMs > HB_FIXED_MS) ? (beatMs - HB_FIXED_MS) : 10;

    switch (_hbState) {

        // P-wave: small slow sine bump
        case HB_P_WAVE:
            if (elapsed == 0) _hbSetSegment(HB_P_FREQ);
            if (elapsed >= HB_P_MS) {
                _hbSilence();
                _hbState   = HB_PQ_SEG;
                _hbStateMs = now;
            }
            break;

        // PQ segment: flat baseline
        case HB_PQ_SEG:
            if (elapsed >= HB_PQ_MS) {
                _hbState   = HB_QRS;
                _hbStateMs = now;
            }
            break;

        // QRS complex: fast sine → looks like sharp spike on scope
        case HB_QRS:
            if (elapsed == 0) _hbSetSegment(HB_QRS_FREQ);
            if (elapsed >= HB_QRS_MS) {
                _hbSilence();
                _hbState   = HB_ST_SEG;
                _hbStateMs = now;
            }
            break;

        // ST segment: flat baseline
        case HB_ST_SEG:
            if (elapsed >= HB_ST_MS) {
                _hbState   = HB_T_WAVE;
                _hbStateMs = now;
            }
            break;

        // T-wave: broad slow sine bump
        case HB_T_WAVE:
            if (elapsed == 0) _hbSetSegment(HB_T_FREQ);
            if (elapsed >= HB_T_MS) {
                _hbSilence();
                _hbState   = HB_TP_REST;
                _hbStateMs = now;
            }
            break;

        // TP rest: diastolic silence until next beat
        case HB_TP_REST:
            if (elapsed >= tpRestMs) {
                _hbState   = HB_P_WAVE;
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
    if (_wave == WAVE_HEARTBEAT) return String(_bpm) + " BPM";
    float f = _freq;
    if      (f >= 1000000.0f) return String(f / 1000000.0f, 4) + " MHz";
    else if (f >= 1000.0f)    return String(f / 1000.0f,    3) + " kHz";
    else if (f < 1.0f)        return String(f, 1) + " Hz";
    else                      return String((long)f) + " Hz";
}

// ── Private helpers ───────────────────────────────────────

void SignalGenerator::_applyWave() {
    switch (_wave) {
        case WAVE_SINE:      _dds.setWave(1); break;   // AD9833_SINE
        case WAVE_TRIANGLE:  _dds.setWave(4); break;   // AD9833_TRIANGLE
        case WAVE_SQUARE:    _dds.setWave(2); break;   // AD9833_SQUARE1
        case WAVE_SQUARE2:   _dds.setWave(3); break;   // AD9833_SQUARE2
        case WAVE_HEARTBEAT: _hbSilence(); break;
        default:             _dds.setWave(1); break;
    }
}

void SignalGenerator::_hbSetSegment(float freqHz) {
    // Set frequency first, then enable output — avoids glitch
    _dds.setFrequency(freqHz, 0);
    _dds.setWave(1);               // AD9833_SINE = 1, also clears OFF
}

void SignalGenerator::_hbSilence() {
    _dds.setWave(0);               // AD9833_OFF = 0 — DAC to mid-scale, no glitch
}

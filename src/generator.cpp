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

// ── ECG segment parameters ────────────────────────────────
//
//  P-wave:  sine   5 Hz  60 ms  → ~0.3 cycle = gentle small bump
//  PQ:      OFF          120 ms → flat baseline
//  QRS:     SQUARE 40 Hz 35 ms  → square looks taller/sharper than sine
//  ST:      OFF          100 ms → flat baseline
//  T-wave:  sine   4 Hz  180 ms → ~0.7 cycle = broad smooth bump
//  TP:      OFF    remainder    → diastolic rest
//
//  Fixed total: 60+120+35+100+180 = 495 ms
//  At 60 BPM (1000ms): TP = 505ms  — plenty of flat line
//  At 70 BPM ( 857ms): TP = 362ms
//  At 100BPM ( 600ms): TP = 105ms

static const uint32_t HB_P_MS     =  60;
static const uint32_t HB_PQ_MS    = 120;
static const uint32_t HB_QRS_MS   =  35;
static const uint32_t HB_ST_MS    = 100;
static const uint32_t HB_T_MS     = 180;
static const uint32_t HB_FIXED_MS = HB_P_MS + HB_PQ_MS + HB_QRS_MS
                                   + HB_ST_MS + HB_T_MS;  // 495 ms

static const float HB_P_FREQ   =  5.0f;   // sine  — soft bump
static const float HB_QRS_FREQ = 40.0f;   // square — sharp spike
static const float HB_T_FREQ   =  4.0f;   // sine  — broad bump

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

void SignalGenerator::setFrequency(float hz) {
    hz = constrain(hz, FREQ_MIN, FREQ_MAX);
    _freq = hz;
    _dds.setFrequency(_freq, 0);
}

void SignalGenerator::stepUp()   { setFrequency(_freq + getStepHz()); }
void SignalGenerator::stepDown() { setFrequency(_freq - getStepHz()); }

void SignalGenerator::nextWave() {
    _wave = static_cast<WaveType>((_wave + 1) % WAVE_COUNT);
    if (_wave == WAVE_HEARTBEAT) {
        _hbEnterState(HB_TP_REST);   // start in silence, P-wave fires next tick
    } else {
        _applyWave();
    }
}

void SignalGenerator::nextStep() {
    _step = static_cast<FreqStep>((_step + 1) % STEP_COUNT);
}

void SignalGenerator::setBPM(int bpm) {
    _bpm = constrain(bpm, BPM_MIN, BPM_MAX);
}

// ── ECG tick ──────────────────────────────────────────────
// Output is configured ONCE in _hbEnterState().
// tickHeartbeat() only checks elapsed time and fires transitions.

void SignalGenerator::tickHeartbeat() {
    if (_wave != WAVE_HEARTBEAT) return;

    uint32_t now     = millis();
    uint32_t elapsed = now - _hbStateMs;

    uint32_t beatMs   = 60000UL / (uint32_t)_bpm;
    uint32_t tpRestMs = (beatMs > HB_FIXED_MS)
                        ? (beatMs - HB_FIXED_MS) : 20;

    switch (_hbState) {
        case HB_P_WAVE:   if (elapsed >= HB_P_MS)   _hbEnterState(HB_PQ_SEG);  break;
        case HB_PQ_SEG:   if (elapsed >= HB_PQ_MS)  _hbEnterState(HB_QRS);     break;
        case HB_QRS:      if (elapsed >= HB_QRS_MS) _hbEnterState(HB_ST_SEG);  break;
        case HB_ST_SEG:   if (elapsed >= HB_ST_MS)  _hbEnterState(HB_T_WAVE);  break;
        case HB_T_WAVE:   if (elapsed >= HB_T_MS)   _hbEnterState(HB_TP_REST); break;
        case HB_TP_REST:  if (elapsed >= tpRestMs)  _hbEnterState(HB_P_WAVE);  break;
    }
}

// ── Labels ────────────────────────────────────────────────

float       SignalGenerator::getStepHz() const { return STEP_HZ_TABLE[_step]; }
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

// ── Private ───────────────────────────────────────────────

void SignalGenerator::_applyWave() {
    switch (_wave) {
        case WAVE_SINE:      _dds.setWave(1); break;   // AD9833_SINE
        case WAVE_TRIANGLE:  _dds.setWave(4); break;   // AD9833_TRIANGLE
        case WAVE_SQUARE:    _dds.setWave(2); break;   // AD9833_SQUARE1
        case WAVE_SQUARE2:   _dds.setWave(3); break;   // AD9833_SQUARE2
        case WAVE_HEARTBEAT: _hbSilence();    break;
        default:             _dds.setWave(1); break;
    }
}

void SignalGenerator::_hbEnterState(HBState s) {
    _hbState   = s;
    _hbStateMs = millis();

    switch (s) {
        case HB_P_WAVE:
            // Sine at 5 Hz for 60 ms ≈ 0.3 cycle → smooth gentle bump
            _dds.setFrequency(HB_P_FREQ, 0);
            _dds.setWave(1);        // SINE
            break;

        case HB_PQ_SEG:
            _hbSilence();
            break;

        case HB_QRS:
            // Square at 40 Hz for 35 ms ≈ 1.4 cycles → sharp, tall-looking spike
            _dds.setFrequency(HB_QRS_FREQ, 0);
            _dds.setWave(2);        // SQUARE1 — visually sharper & "taller"
            break;

        case HB_ST_SEG:
            _hbSilence();
            break;

        case HB_T_WAVE:
            // Sine at 4 Hz for 180 ms ≈ 0.72 cycle → broad smooth dome
            _dds.setFrequency(HB_T_FREQ, 0);
            _dds.setWave(1);        // SINE
            break;

        case HB_TP_REST:
            _hbSilence();
            break;
    }
}

void SignalGenerator::_hbSilence() {
    _dds.setWave(0);   // AD9833_OFF — DAC mid-scale, no glitch
}

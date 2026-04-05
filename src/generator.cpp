#include "generator.h"

// Step sizes in Hz for each decade
static const float STEP_HZ_TABLE[] = {
    0.1f, 1.0f, 10.0f, 100.0f,
    1000.0f, 10000.0f, 100000.0f, 1000000.0f
};

static const char* STEP_LABELS[] = {
    "0.1Hz", "1Hz", "10Hz", "100Hz",
    "1kHz", "10kHz", "100kHz", "1MHz"
};

static const char* WAVE_LABELS[] = {
    "SINE", "TRI", "SQR", "SQR/2"
};

// ─────────────────────────────────────────────────────────

SignalGenerator::SignalGenerator()
    : _dds(GEN_CS_PIN),
      _freq(1000.0f),
      _wave(WAVE_SINE),
      _step(STEP_1KHZ)
{}

void SignalGenerator::begin() {
    // Use HSPI bus: SCK=18, MISO=12(unused), MOSI=23
    SPI.begin(GEN_SCK_PIN, /*MISO=*/12, GEN_MOSI_PIN, GEN_CS_PIN);
    _dds.begin();
    _dds.setFrequency(AD9833::FREQ0, _freq);
    _applyWave();
}

void SignalGenerator::setFrequency(float hz) {
    hz = constrain(hz, FREQ_MIN, FREQ_MAX);
    _freq = hz;
    _dds.setFrequency(AD9833::FREQ0, _freq);
}

void SignalGenerator::stepUp() {
    setFrequency(_freq + getStepHz());
}

void SignalGenerator::stepDown() {
    setFrequency(_freq - getStepHz());
}

void SignalGenerator::nextWave() {
    _wave = static_cast<WaveType>((_wave + 1) % WAVE_COUNT);
    _applyWave();
}

void SignalGenerator::nextStep() {
    _step = static_cast<FreqStep>((_step + 1) % STEP_COUNT);
}

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
    if (f >= 1000000.0f) {
        return String(f / 1000000.0f, 4) + " MHz";
    } else if (f >= 1000.0f) {
        return String(f / 1000.0f, 3) + " kHz";
    } else if (f < 1.0f) {
        return String(f, 1) + " Hz";
    } else {
        return String((long)f) + " Hz";
    }
}

void SignalGenerator::_applyWave() {
    switch (_wave) {
        case WAVE_SINE:
            _dds.setWave(AD9833::SINE);
            break;
        case WAVE_TRIANGLE:
            _dds.setWave(AD9833::TRIANGLE);
            break;
        case WAVE_SQUARE:
            _dds.setWave(AD9833::SQUARE1);
            break;
        case WAVE_SQUARE2:
            _dds.setWave(AD9833::SQUARE2);
            break;
        default:
            _dds.setWave(AD9833::SINE);
            break;
    }
}

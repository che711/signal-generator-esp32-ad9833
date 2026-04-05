#pragma once

#include <Arduino.h>
#include <AD9833.h>
#include <SPI.h>

// ── SPI pins ──────────────────────────────────────────────
#define GEN_SCK_PIN   18
#define GEN_MOSI_PIN  23
#define GEN_CS_PIN     5

// ── Frequency limits ──────────────────────────────────────
#define FREQ_MIN       0.1f       // Hz
#define FREQ_MAX   12000000.0f    // 12 MHz

// ── Waveform types ────────────────────────────────────────
enum WaveType : uint8_t {
    WAVE_SINE     = 0,
    WAVE_TRIANGLE = 1,
    WAVE_SQUARE   = 2,
    WAVE_SQUARE2  = 3,   // square at f/2
    WAVE_COUNT    = 4
};

// ── Frequency step decades ───────────────────────────────
enum FreqStep : uint8_t {
    STEP_01HZ  = 0,   //  0.1 Hz
    STEP_1HZ   = 1,   //  1   Hz
    STEP_10HZ  = 2,   //  10  Hz
    STEP_100HZ = 3,   //  100 Hz
    STEP_1KHZ  = 4,   //  1   kHz
    STEP_10KHZ = 5,   //  10  kHz
    STEP_100KHZ= 6,   //  100 kHz
    STEP_1MHZ  = 7,   //  1   MHz
    STEP_COUNT = 8
};

class SignalGenerator {
public:
    SignalGenerator();

    void begin();

    // Set frequency in Hz (float for sub-Hz resolution)
    void setFrequency(float hz);

    // Increment / decrement by current step
    void stepUp();
    void stepDown();

    // Cycle through waveforms
    void nextWave();

    // Cycle through frequency steps
    void nextStep();

    // Getters
    float     getFrequency()  const { return _freq; }
    WaveType  getWave()       const { return _wave; }
    FreqStep  getStep()       const { return _step; }
    float     getStepHz()     const;

    // Human-readable strings
    const char* waveLabel()  const;
    const char* stepLabel()  const;
    String      freqLabel()  const;

private:
    AD9833   _dds;
    float    _freq;
    WaveType _wave;
    FreqStep _step;

    void _applyWave();
};

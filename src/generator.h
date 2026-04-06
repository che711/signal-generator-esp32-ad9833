#pragma once

#include <Arduino.h>
#include <AD9833.h>
#include <SPI.h>

// ── SPI pins ──────────────────────────────────────────────
#define GEN_SCK_PIN   18
#define GEN_MOSI_PIN  23
#define GEN_CS_PIN     5

// ── Frequency limits ──────────────────────────────────────
#define FREQ_MIN       0.1f
#define FREQ_MAX   12000000.0f

// ── Heartbeat BPM limits ──────────────────────────────────
#define BPM_MIN   30
#define BPM_MAX  200
#define BPM_DEFAULT 70

// ── Waveform types ────────────────────────────────────────
enum WaveType : uint8_t {
    WAVE_SINE      = 0,
    WAVE_TRIANGLE  = 1,
    WAVE_SQUARE    = 2,
    WAVE_SQUARE2   = 3,
    WAVE_HEARTBEAT = 4,
    WAVE_COUNT     = 5
};

// ── Heartbeat FSM states ──────────────────────────────────
enum HBState : uint8_t {
    HB_LUB    = 0,
    HB_PAUSE1 = 1,
    HB_DUB    = 2,
    HB_PAUSE2 = 3
};

// ── Frequency step decades ────────────────────────────────
enum FreqStep : uint8_t {
    STEP_01HZ   = 0,
    STEP_1HZ    = 1,
    STEP_10HZ   = 2,
    STEP_100HZ  = 3,
    STEP_1KHZ   = 4,
    STEP_10KHZ  = 5,
    STEP_100KHZ = 6,
    STEP_1MHZ   = 7,
    STEP_COUNT  = 8
};

class SignalGenerator {
public:
    SignalGenerator();

    void begin();

    // Frequency control
    void  setFrequency(float hz);
    void  stepUp();
    void  stepDown();

    // Waveform / step cycling
    void  nextWave();
    void  nextStep();

    // Heartbeat mode
    void  tickHeartbeat();           // call every loop()
    bool  isHeartbeat() const        { return _wave == WAVE_HEARTBEAT; }
    void  setBPM(int bpm);
    int   getBPM()  const            { return _bpm; }

    // Getters
    float     getFrequency() const   { return _freq; }
    WaveType  getWave()      const   { return _wave; }
    FreqStep  getStep()      const   { return _step; }
    float     getStepHz()    const;

    // Human-readable labels
    const char* waveLabel() const;
    const char* stepLabel() const;
    String      freqLabel() const;   // returns "XX BPM" in heartbeat mode

private:
    AD9833   _dds;
    float    _freq;
    WaveType _wave;
    FreqStep _step;

    // Heartbeat state
    int      _bpm;
    HBState  _hbState;
    uint32_t _hbStateMs;

    void _applyWave();
    void _ddsOutput(bool on);   // enable / disable AD9833 output
};

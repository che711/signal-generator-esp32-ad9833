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
#define BPM_MIN      30
#define BPM_MAX     200
#define BPM_DEFAULT  70

// ── Waveform types ────────────────────────────────────────
enum WaveType : uint8_t {
    WAVE_SINE      = 0,
    WAVE_TRIANGLE  = 1,
    WAVE_SQUARE    = 2,
    WAVE_SQUARE2   = 3,
    WAVE_HEARTBEAT = 4,
    WAVE_COUNT     = 5
};

// ── Heartbeat FSM: ECG shape P -> PQ -> QRS -> ST -> T -> TP ──
enum HBState : uint8_t {
    HB_P_WAVE  = 0,   // small slow bump   ~6 Hz  sine  80 ms
    HB_PQ_SEG  = 1,   // silence                       100 ms
    HB_QRS     = 2,   // sharp fast spike  ~28 Hz sine  50 ms
    HB_ST_SEG  = 3,   // silence                       110 ms
    HB_T_WAVE  = 4,   // broad slow bump   ~5 Hz  sine 160 ms
    HB_TP_REST = 5    // diastolic silence (beat remainder)
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

    void  setFrequency(float hz);
    void  stepUp();
    void  stepDown();
    void  nextWave();
    void  nextStep();

    // Heartbeat
    void  tickHeartbeat();
    bool  isHeartbeat() const      { return _wave == WAVE_HEARTBEAT; }
    void  setBPM(int bpm);
    int   getBPM()  const          { return _bpm; }

    float     getFrequency() const { return _freq; }
    WaveType  getWave()      const { return _wave; }
    FreqStep  getStep()      const { return _step; }
    float     getStepHz()    const;

    const char* waveLabel() const;
    const char* stepLabel() const;
    String      freqLabel() const;

private:
    AD9833   _dds;
    float    _freq;
    WaveType _wave;
    FreqStep _step;

    int      _bpm;
    HBState  _hbState;
    uint32_t _hbStateMs;

    void _applyWave();
    void _hbSetSegment(float freqHz);  // start active segment at given freq
    void _hbSilence();                  // mute output
};

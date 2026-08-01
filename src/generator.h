#pragma once

#include <Arduino.h>
#include <AD9833.h>
#include <SPI.h>
#include <Preferences.h>
#include "config.h"

#define FREQ_MIN      0.1f
#define FREQ_MAX  12000000.0f

enum WaveType : uint8_t {
    WAVE_SINE     = 0,
    WAVE_TRIANGLE = 1,
    WAVE_SQUARE   = 2,
    WAVE_SQUARE2  = 3,
    WAVE_COUNT    = 4
};

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
    void saveSettings();

    // setFrequency возвращает реально установленное значение
    float setFrequency(float hz);
    void  setWaveByIndex(int idx);
    void  setStepByIndex(int idx);

    void stepUp();
    void stepDown();
    void nextWave();
    void nextStep();

    float      getFrequency() const { return _freq; }
    WaveType   getWave()      const { return _wave; }
    FreqStep   getStep()      const { return _step; }
    float      getStepHz()    const;

    const char* waveLabel() const;
    const char* stepLabel() const;
    String      freqLabel() const;

private:
    AD9833      _dds;
    Preferences _prefs;
    float       _freq;
    WaveType    _wave;
    FreqStep    _step;

    void _applyWave();
    void _loadSettings();
};

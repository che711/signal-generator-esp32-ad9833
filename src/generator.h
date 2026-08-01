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

    // ── Sweep ─────────────────────────────────────────────
    // Линейный или логарифмический проход f0 → f1 за durMs.
    // false — параметры вне диапазона. Ручной setFrequency()
    // (энкодер, веб) останавливает активный sweep.
    bool sweepStart(float f0, float f1, uint32_t durMs, bool logMode);
    void sweepStop();
    bool sweepActive()   const { return _swActive; }
    // Вызывать из loop() ПОД мьютексом. true — частота изменилась
    bool sweepTick(uint32_t nowMs);
    float    sweepF0()       const { return _swF0; }
    float    sweepF1()       const { return _swF1; }
    uint32_t sweepDurMs()    const { return _swDurMs; }
    bool     sweepIsLog()    const { return _swLog; }
    int      sweepProgress() const;   // 0–100 %

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

    // ── Sweep state ───────────────────────────────────────
    bool     _swActive  = false;
    bool     _swLog     = false;
    float    _swF0      = 0;
    float    _swF1      = 0;
    uint32_t _swDurMs   = 0;
    uint32_t _swStartMs = 0;
    uint32_t _swTickMs  = 0;

    // Установка частоты БЕЗ остановки sweep (для sweepTick)
    void _applyFreq(float hz);

    void _applyWave();
    void _loadSettings();
};

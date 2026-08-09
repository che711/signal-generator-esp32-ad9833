#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include <atomic>
#include "generator.h"
#include "config.h"

class WebUI {
public:
    // genMutex — guards access to SignalGenerator, shared with main
    WebUI(SignalGenerator& gen, SemaphoreHandle_t genMutex);

    void begin();
    void handle();              // call from the web task
    void checkWiFi();           // WiFi watchdog
    void updateCpuLoad();       // CPU sampling

    bool   isConnected() const { return _connected; }
    String ipAddress()   const;

    // Returns true once after settings changed via the web UI, so the main
    // loop can redraw the OLED and restart the autosave timer.
    // Atomic: written by the web task (core 0), read by loop() (core 1);
    // exchange() reads and clears in one step so no event is lost
    bool consumeChanged() {
        return _changedFlag.exchange(false);
    }

private:
    SignalGenerator&  _gen;
    SemaphoreHandle_t _genMutex;
    WebServer         _server;
    bool             _connected;
    bool             _serverStarted;   // routes are registered exactly once
    bool             _mdnsStarted;
    std::atomic<bool> _changedFlag;
    uint32_t         _lastWifiCheckMs;

    // ── CPU load monitor ──────────────────────────────────
    static volatile uint32_t _s_idle0;   // idle hook counters
    static volatile uint32_t _s_idle1;
    uint32_t _cpuSampleMs;
    uint32_t _cpuIdle0Prev;
    uint32_t _cpuIdle1Prev;
    float    _cpuIdleRateMax;  // baseline: idle ticks/ms at ~0 % load
    bool     _cpuFirstSample;  // the first interval is skewed — skip it
    int      _cpuLoad;         // 0-100 %

    static bool IRAM_ATTR _idleHook0();
    static bool IRAM_ATTR _idleHook1();
    void _initCpuMon();

    // ── WiFi / Server ─────────────────────────────────────
    void _connectWiFi();
    void _startServer();
    void _registerRoutes();

    void _handleRoot();
    void _handleStatus();
    void _handleSetFreq();
    void _handleSetWave();
    void _handleSetStep();
    void _handleSave();
    void _handleSetOut();
    void _handleReboot();
    void _handleSweepStart();
    void _handleSweepStop();

    // Helper: take the mutex, run the action, release it.
    // Returns false if the mutex was not acquired within timeout
    bool _withGen(std::function<void()> fn, TickType_t timeout = pdMS_TO_TICKS(50));

    static const char _HTML[];
};

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp_freertos_hooks.h"
#include "generator.h"
#include "config.h"

class WebUI {
public:
    WebUI(SignalGenerator& gen);

    void begin();
    void handle();
    void checkWiFi();         // WiFi watchdog — call in loop()
    void updateCpuLoad();     // CPU sampling   — call in loop()

    bool   isConnected() const { return _connected; }
    String ipAddress()   const;

private:
    SignalGenerator& _gen;
    WebServer        _server;
    bool             _connected;
    uint32_t         _lastWifiCheckMs;

    // ── CPU load monitor ──────────────────────────────────
    static volatile uint32_t _s_idle0;   // idle hook counters
    static volatile uint32_t _s_idle1;
    uint32_t _cpuSampleMs;
    uint32_t _cpuIdle0Prev;
    uint32_t _cpuIdle1Prev;
    uint32_t _cpuIdleMax;    // baseline: ticks per 2s at 0% load
    int      _cpuLoad;       // 0-100 %

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

    static const char _HTML[];
};
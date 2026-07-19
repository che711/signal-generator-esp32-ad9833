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

    // true один раз после изменения настроек через веб (для перерисовки
    // OLED и запуска таймера автосохранения в main loop)
    bool consumeChanged() {
        bool c = _changedFlag;
        _changedFlag = false;
        return c;
    }

private:
    SignalGenerator& _gen;
    WebServer        _server;
    bool             _connected;
    bool             _serverStarted;   // маршруты регистрируем ровно один раз
    bool             _mdnsStarted;
    bool             _changedFlag;
    uint32_t         _lastWifiCheckMs;

    // ── CPU load monitor ──────────────────────────────────
    static volatile uint32_t _s_idle0;   // idle hook counters
    static volatile uint32_t _s_idle1;
    uint32_t _cpuSampleMs;
    uint32_t _cpuIdle0Prev;
    uint32_t _cpuIdle1Prev;
    float    _cpuIdleRateMax;  // baseline: idle-тиков/мс при ~0% загрузки
    bool     _cpuFirstSample;  // первый интервал искажён — пропускаем
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

    static const char _HTML[];
};
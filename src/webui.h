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
    // genMutex — мьютекс на доступ к SignalGenerator, разделяется с main
    WebUI(SignalGenerator& gen, SemaphoreHandle_t genMutex);

    void begin();
    void handle();              // вызывать из web-задачи
    void checkWiFi();           // WiFi watchdog
    void updateCpuLoad();       // CPU sampling

    bool   isConnected() const { return _connected; }
    String ipAddress()   const;

    // true один раз после изменения настроек через веб (для перерисовки
    // OLED и запуска таймера автосохранения в main loop)
    // atomic: флаг пишется web-задачей (ядро 0), читается loop() (ядро 1).
    // exchange() атомарно читает и сбрасывает — событие не потеряется
    bool consumeChanged() {
        return _changedFlag.exchange(false);
    }

private:
    SignalGenerator&  _gen;
    SemaphoreHandle_t _genMutex;
    WebServer         _server;
    bool             _connected;
    bool             _serverStarted;   // маршруты регистрируем ровно один раз
    bool             _mdnsStarted;
    std::atomic<bool> _changedFlag;
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

    // Хелпер: захватить мьютекс, выполнить действие, отпустить
    // Возвращает false если мьютекс не получен за timeout
    bool _withGen(std::function<void()> fn, TickType_t timeout = pdMS_TO_TICKS(50));

    static const char _HTML[];
};

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "generator.h"
#include "config.h"

class WebUI {
public:
    WebUI(SignalGenerator& gen);

    void begin();
    void handle();

    // WiFi watchdog — вызывать в loop()
    void checkWiFi();

    bool   isConnected() const { return _connected; }
    String ipAddress()   const;

private:
    SignalGenerator& _gen;
    WebServer        _server;
    bool             _connected;
    uint32_t         _lastWifiCheckMs;

    void _connectWiFi();
    void _startServer();
    void _registerRoutes();

    void _handleRoot();
    void _handleSetFreq();
    void _handleSetWave();
    void _handleSetStep();
    void _handleStatus();
    void _handleSave();

    static const char _HTML[];
};

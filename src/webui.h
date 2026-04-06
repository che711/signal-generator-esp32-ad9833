#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "generator.h"

class WebUI {
public:
    WebUI(SignalGenerator& gen);

    // Call once in setup() — connects WiFi and starts server
    void begin();

    // Call every loop() — handles incoming HTTP requests
    void handle();

    bool   isConnected() const { return _connected; }
    String ipAddress()   const;

private:
    SignalGenerator& _gen;
    WebServer        _server;
    bool             _connected;

    void _connectWiFi();
    void _registerRoutes();

    void _handleRoot();
    void _handleStatus();
    void _handleSetFreq();
    void _handleSetWave();
    void _handleSetStep();
    void _handleSetBPM();     // ← new

    static const char _HTML[];
};

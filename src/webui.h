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

    // True after successful WiFi connect
    bool isConnected() const { return _connected; }

    // IP as string for display
    String ipAddress() const;

private:
    SignalGenerator& _gen;
    WebServer        _server;
    bool             _connected;

    void _connectWiFi();
    void _registerRoutes();

    // Route handlers
    void _handleRoot();
    void _handleSetFreq();
    void _handleSetWave();
    void _handleSetStep();
    void _handleStatus();

    // HTML page (embedded)
    static const char _HTML[];
};

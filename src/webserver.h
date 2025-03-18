#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>
#include "clockcontrol.h"
#include "homeassistant.h"

class WebInterface {
public:
    WebInterface(ClockControl& clockControl);
    void begin();
    void handle();
    void setHomeAssistant(HomeAssistant* ha) { homeAssistant = ha; }

private:
    ESP8266WebServer server;
    ESP8266HTTPUpdateServer httpUpdater;
    ClockControl& clockControl;
    HomeAssistant* homeAssistant;

    // API-Handler
    void handleGetStatus();
    void handleSetTime();
    void handleAdjustTime();
    void handleResetWifi();
    void handleRestart();
    void handleNotFound();
    void handleResetAllEEPROM();
    
    // MQTT-Handler
    void handleGetMqttConfig();
    void handleSetMqttConfig();
    void handleGetMqttStatus();
    void handleResetMqtt();

    // Hilfsfunktionen
    String getContentType(const String& path);
    bool handleFileRead(const String& path);
    void sendCORSHeaders();
};
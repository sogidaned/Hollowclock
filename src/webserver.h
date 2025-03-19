#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include "clockcontrol.h"
#include "homeassistant.h"
#include <ArduinoJson.h>

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
    WiFiClient client;  // WiFiClient für HTTP-Requests

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

    // Update Handler
    void handleCheckUpdate();
    void handleStartUpdate();
    void handleUpdateResult(int result);

    // Hilfsfunktionen
    String getContentType(const String& path);
    bool handleFileRead(const String& path);
    void sendCORSHeaders();

    // Neue Methoden für GitHub API Zugriff
    void fetchGitHubRelease(JsonDocument& response);
    void directFetchGitHubRelease(JsonDocument& response);
};
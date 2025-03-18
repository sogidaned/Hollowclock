#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "clockcontrol.h"

class HomeAssistant {
private:
    static HomeAssistant* instance;
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    ClockControl& clock;
    String mqttServer;
    int mqttPort;
    String mqttClientId;
    String mqttTopic;
    String mqttUser;
    String mqttPassword;
    bool connected;
    unsigned long lastUpdate;
    String lastError;

    void handleCommand(char* topic, byte* payload, unsigned int length);
    String getBaseTopic();
    String getStateTopic();
    String getCommandTopic();
    String getAvailabilityTopic();
    String getDiscoveryTopic();
    static void handleCallback(char* topic, byte* payload, unsigned int length);
    void addDeviceInfo(JsonObject& device);

public:
    HomeAssistant(ClockControl& clockControl);
    void begin(const char* server, int port, const char* clientId, const char* topic, const char* user, const char* password);
    void reconnect(const char* server, int port, const char* clientId, const char* topic, const char* user, const char* password);
    void update();
    bool isConnected();
    const String& getLastError() { return lastError; }
    void publishState();
    void publishDiscovery();
    void connect();
}; 
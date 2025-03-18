#include "homeassistant.h"
#include "config.h"
#include <ESP8266WiFi.h>

HomeAssistant* HomeAssistant::instance = nullptr;

// Globale Variable für die Discovery-Planung
static bool g_discoveryPending = false;
static unsigned long g_lastDiscoveryAttempt = 0;

HomeAssistant::HomeAssistant(ClockControl& clockControl) : mqttClient(wifiClient), clock(clockControl) {
    connected = false;
    lastUpdate = 0;
    lastError = "";
    instance = this;
}

void HomeAssistant::begin(const char* server, int port, const char* clientId, const char* topic, const char* user, const char* password) {
    // Debug-Ausgaben für die Verbindungsparameter
    Serial.println("\n=== MQTT Parameter setzen ===");
    Serial.printf("Server: %s\n", server);
    Serial.printf("Port: %d\n", port);
    Serial.printf("Client-ID: %s\n", clientId);
    Serial.printf("Topic: %s\n", topic);
    Serial.printf("User: %s\n", user);
    Serial.println("Passwort: " + String(password && password[0] != '\0' ? "gesetzt" : "nicht gesetzt"));
    
    mqttServer = String(server);
    mqttPort = port;
    mqttClientId = String(clientId);
    mqttTopic = String(topic);
    mqttUser = String(user);
    mqttPassword = String(password);
    
    // PubSubClient-Konfiguration
    mqttClient.setServer(server, port);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->handleCallback(topic, payload, length);
    });
    
    // Maximale Paketgröße erhöhen für robustere Verbindung
    mqttClient.setBufferSize(512);
    
    Serial.println("=== MQTT Parameter gesetzt ===");
}

void HomeAssistant::reconnect(const char* server, int port, const char* clientId, const char* topic, const char* user, const char* password) {
    // Trennen, falls verbunden
    if (mqttClient.connected()) {
        mqttClient.disconnect();
    }
    
    // Neue Verbindungsdaten setzen
    begin(server, port, clientId, topic, user, password);
    
    // Sofort neu verbinden
    connect();
}

void HomeAssistant::update() {
    static unsigned long lastReconnectAttempt = 0;
    const unsigned long RECONNECT_INTERVAL = 10000; // 10 Sekunden
    static int reconnectAttempts = 0;
    const int MAX_RECONNECT_ATTEMPTS = 3;
    static unsigned long backoffTime = 0;
    const unsigned long BACKOFF_INTERVAL = 300000; // 5 Minuten Pause nach maximalen Versuchen
    static unsigned long lastStatusUpdate = 0;

    // Verwaltung der Verbindung
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        
        // Wenn wir in der Backoff-Zeit sind, warte
        if (backoffTime > 0) {
            if (now - backoffTime < BACKOFF_INTERVAL) {
                // Noch in der Backoff-Zeit, warte weiter
                return;
            } else {
                // Backoff-Zeit abgelaufen, versuche erneut mit zurückgesetztem Zähler
                Serial.println("MQTT: Backoff-Zeit abgelaufen, versuche erneut zu verbinden");
                reconnectAttempts = 0;
                backoffTime = 0;
            }
        }
        
        if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
            reconnectAttempts++;
            Serial.printf("MQTT-Verbindung verloren. Versuch %d von %d\n", 
                         reconnectAttempts, MAX_RECONNECT_ATTEMPTS);
            
            if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
                Serial.println("Maximale MQTT-Reconnect-Versuche erreicht. Warte 5 Minuten...");
                backoffTime = now; // Starte Backoff-Zeit
                return;
            }
            
            lastReconnectAttempt = now;
            connect();
        }
    } else {
        // Verbindung erfolgreich, setze Zähler zurück
        reconnectAttempts = 0;
        backoffTime = 0;
        
        // MQTT-Loop ausführen
        mqttClient.loop();
        
        unsigned long now = millis();
        
        // Discovery verzögert senden, wenn angefordert
        if (g_discoveryPending && now - g_lastDiscoveryAttempt > 60000) {
            // Genug Speicher für Discovery?
            if (ESP.getFreeHeap() > 10000) {
                Serial.println("Sende verzögerte Discovery-Nachrichten...");
                publishDiscovery();
                g_discoveryPending = false;
                g_lastDiscoveryAttempt = now;
            } else {
                Serial.println("Zu wenig Speicher für Discovery, verschiebe...");
            }
        }
        
        // Status periodisch aktualisieren (alle 60 Sekunden)
        if (now - lastStatusUpdate > 60000) {
            publishState();
            lastStatusUpdate = now;
        }
    }
}

bool HomeAssistant::isConnected() {
    return connected;
}

void HomeAssistant::connect() {
    if (mqttClient.connected()) {
        Serial.println("MQTT bereits verbunden");
        connected = true;
        return;
    }
    
    // Prüfe verfügbaren Speicher
    if (ESP.getFreeHeap() < 8192) {  // Erhöhe die Anforderungen, damit weniger Speicherprobleme auftreten
        Serial.println("Warnung: Zu wenig Speicher verfügbar, MQTT-Verbindung übersprungen");
        return;
    }
    
    Serial.println("\n=== MQTT Verbindungsversuch ===");
    
    // Prüfe WiFi-Verbindung
    if (WiFi.status() != WL_CONNECTED) {
        lastError = "Keine WiFi-Verbindung";
        Serial.println(lastError);
        return;
    }
    
    // Prüfe Server-Adresse
    if (mqttServer.length() == 0) {
        lastError = "Keine Server-Adresse";
        Serial.println(lastError);
        return;
    }
    
    bool success = false;
    
    // Einfache Verbindung ohne Last Will probieren
    if (mqttUser.length() > 0) {
        // Mit Authentifizierung
        success = mqttClient.connect(mqttClientId.c_str(), mqttUser.c_str(), mqttPassword.c_str());
    } else {
        // Ohne Authentifizierung
        success = mqttClient.connect(mqttClientId.c_str());
    }
    
    if (success) {
        Serial.println("MQTT verbunden!");
        connected = true;
        
        // Abonniere Command-Topic für Befehle
        mqttClient.subscribe(getCommandTopic().c_str());
        Serial.printf("Abonniert: %s\n", getCommandTopic().c_str());
        
        // Warte kurz für Stabilität
        delay(100);
        yield();
        
        // Status sofort senden
        publishState();
        
        // Discovery für später planen
        g_discoveryPending = true;
        Serial.println("Discovery wurde für später geplant");
    } else {
        lastError = "Verbindungsfehler: " + String(mqttClient.state());
        Serial.printf("Verbindung fehlgeschlagen, Code: %d\n", mqttClient.state());
        connected = false;
    }
    
    Serial.println("=== MQTT Verbindungsversuch abgeschlossen ===\n");
}

void HomeAssistant::publishDiscovery() {
    if (!mqttClient.connected()) {
        Serial.println("MQTT nicht verbunden, Discovery wird übersprungen");
        return;
    }
    
    Serial.println("\n=== Sende MQTT Auto-Discovery (minimale Version) ===");
    
    // Wir bauen ein einzelnes kombiniertes Gerät mit weniger Entitäten
    // Begrenzen wir uns auf maximal 2 wichtige Sensoren, um Speicherprobleme zu vermeiden
    
    // 1. Zeit-Entity als Sensor
    {
        StaticJsonDocument<256> timeDoc;
        timeDoc["name"] = "Hollow Clock Time";
        timeDoc["unique_id"] = "hollowclock_time_" + String(ESP.getChipId());
        timeDoc["state_topic"] = getStateTopic();
        timeDoc["value_template"] = "{{ value_json.time }}";
        timeDoc["icon"] = "mdi:clock";
        
        JsonObject device = timeDoc.createNestedObject("device");
        addDeviceInfo(device);
        
        String timePayload;
        serializeJson(timeDoc, timePayload);
        
        String timeTopic = "homeassistant/sensor/" + mqttClientId + "/time/config";
        Serial.printf("Sende Zeit-Entity an Topic: %s\n", timeTopic.c_str());
        
        bool success = mqttClient.publish(timeTopic.c_str(), timePayload.c_str(), true);
        if (!success) {
            Serial.println("Fehler beim Senden der Zeit-Entity");
        }
        
        // JSONDocument freigeben
        timeDoc.clear();
    }
    
    // Pause zwischen den Discovery-Nachrichten für Stabilität
    delay(250);
    yield();
    
    // 2. Position als Sensor (numerisch)
    {
        StaticJsonDocument<256> positionDoc;
        positionDoc["name"] = "Hollow Clock Position";
        positionDoc["unique_id"] = "hollowclock_position_" + String(ESP.getChipId());
        positionDoc["state_topic"] = getStateTopic();
        positionDoc["value_template"] = "{{ value_json.position }}";
        positionDoc["icon"] = "mdi:ray-vertex";
        
        JsonObject device = positionDoc.createNestedObject("device");
        addDeviceInfo(device);
        
        String positionPayload;
        serializeJson(positionDoc, positionPayload);
        
        String positionTopic = "homeassistant/sensor/" + mqttClientId + "/position/config";
        Serial.printf("Sende Position-Entity an Topic: %s\n", positionTopic.c_str());
        
        bool success = mqttClient.publish(positionTopic.c_str(), positionPayload.c_str(), true);
        if (!success) {
            Serial.println("Fehler beim Senden der Position-Entity");
        }
        
        // JSONDocument freigeben
        positionDoc.clear();
    }
    
    Serial.println("=== MQTT Auto-Discovery abgeschlossen ===\n");
}

void HomeAssistant::publishState() {
    if (!mqttClient.connected()) {
        Serial.println("MQTT nicht verbunden, Status wird nicht gesendet");
        return;
    }
    
    // Baue JSON-Dokument - kleiner halten, um Speicherprobleme zu vermeiden
    StaticJsonDocument<256> doc;
    
    // Status-Informationen - nur das Wesentliche
    doc["position"] = clock.getCurrentPosition();
    
    // Status (vereinfacht)
    static int lastReportedPosition = -1;
    bool isMoving = lastReportedPosition != clock.getCurrentPosition();
    lastReportedPosition = clock.getCurrentPosition();
    doc["is_running"] = isMoving;
    
    // Aktuelle Uhrzeit
    int currentHour = hour() % 12;
    if (currentHour == 0) currentHour = 12;
    int currentMinute = minute();
    
    // Zeit als formatierter String mit führenden Nullen
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", currentHour, currentMinute);
    doc["time"] = timeStr;
    
    // Basisdaten für Fehlersuche
    doc["uptime"] = millis() / 1000;
    doc["heap"] = ESP.getFreeHeap();
    
    // Serialisiere JSON
    String payload;
    serializeJson(doc, payload);
    
    // Sende an MQTT
    String stateTopic = getStateTopic();
    Serial.printf("Sende Status an: %s\n", stateTopic.c_str());
    Serial.println(payload);
    
    if (!mqttClient.publish(stateTopic.c_str(), payload.c_str(), true)) {
        Serial.println("Fehler beim Senden des Status!");
    }
}

void HomeAssistant::handleCommand(char* topic, byte* payload, unsigned int length) {
    // Payload in String umwandeln
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("JSON-Parsing fehlgeschlagen:");
        Serial.println(error.c_str());
        return;
    }
    
    String response = "{\"status\":\"error\",\"message\":\"Unbekannter Befehl\"}";
    
    // Zeit setzen (wie beim Time Picker)
    if (doc.containsKey("hour") && doc.containsKey("minute")) {
        int hour = doc["hour"];
        int minute = doc["minute"];
        
        if (hour >= 1 && hour <= 12 && minute >= 0 && minute <= 59) {
            // Position aus der Zeit berechnen und direkt setzen (ohne Bewegung)
            int position = clock.calculateStepsFromTime(hour, minute);
            clock.setPosition(position);
            
            response = "{\"status\":\"success\",\"message\":\"Zeit gesetzt\",\"hour\":" + String(hour) + 
                      ",\"minute\":" + String(minute) + ",\"position\":" + String(position) + "}";
            
            Serial.printf("Zeit über MQTT gesetzt: %02d:%02d (Position: %d)\n", hour, minute, position);
        } else {
            lastError = "Ungültige Zeitwerte";
            response = "{\"status\":\"error\",\"message\":\"Ungültige Zeitwerte\"}";
        }
    }
    // Position speichern
    else if (doc.containsKey("save_position")) {
        // Aktuelle Position speichern
        clock.savePosition();
        // Zur aktuellen NTP-Zeit synchronisieren
        clock.syncToNTPTime();
        
        response = "{\"status\":\"success\",\"message\":\"Position gespeichert\"}";
        Serial.println("Position über MQTT gespeichert");
    }
    // Neue Funktion: Zeit setzen und kalibrieren (wie auf der Homepage)
    else if (doc.containsKey("set_time")) {
        JsonObject timeObj = doc["set_time"];
        
        if (timeObj.containsKey("hour") && timeObj.containsKey("minute")) {
            int hour = timeObj["hour"];
            int minute = timeObj["minute"];
            
            if (hour >= 1 && hour <= 12 && minute >= 0 && minute <= 59) {
                // Position aus der Zeit berechnen und direkt setzen (ohne Bewegung)
                int position = clock.calculateStepsFromTime(hour, minute);
                clock.setPosition(position);
                
                // Zur aktuellen NTP-Zeit synchronisieren (wie bei der Weboberfläche)
                clock.syncToNTPTime();
                
                response = "{\"status\":\"success\",\"message\":\"Zeit gesetzt und kalibriert\",\"hour\":" + String(hour) + 
                          ",\"minute\":" + String(minute) + ",\"position\":" + String(position) + "}";
                
                Serial.printf("Zeit über MQTT gesetzt und kalibriert: %02d:%02d (Position: %d)\n", hour, minute, position);
            } else {
                lastError = "Ungültige Zeitwerte";
                response = "{\"status\":\"error\",\"message\":\"Ungültige Zeitwerte\"}";
            }
        } else {
            lastError = "Zeit fehlt";
            response = "{\"status\":\"error\",\"message\":\"Zeit fehlt\"}";
        }
    }
    // Neuer Befehl: Discovery erneut senden
    else if (doc.containsKey("rediscover")) {
        // Discovery-Nachrichten erneut senden
        publishDiscovery();
        
        response = "{\"status\":\"success\",\"message\":\"Discovery-Nachrichten erneut gesendet\"}";
        Serial.println("Discovery-Nachrichten über MQTT erneut gesendet");
    }
    
    // Sende Antwort
    String responseTopic = getBaseTopic() + "/response";
    mqttClient.publish(responseTopic.c_str(), response.c_str(), true);
    
    // Status nach Änderung aktualisieren
    publishState();
}

String HomeAssistant::getBaseTopic() {
    return mqttTopic;
}

String HomeAssistant::getStateTopic() {
    return getBaseTopic() + "/state";
}

String HomeAssistant::getCommandTopic() {
    return getBaseTopic() + "/set";
}

String HomeAssistant::getAvailabilityTopic() {
    return getBaseTopic() + "/available";
}

String HomeAssistant::getDiscoveryTopic() {
    // Da wir diese Funktion nicht mehr direkt nutzen, geben wir einfach das Basis-Topic für Sensoren zurück
    return "homeassistant/sensor/" + mqttClientId;
}

void HomeAssistant::handleCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        instance->handleCommand(topic, payload, length);
    }
}

// Hilfsfunktion für einheitliches Device-Objekt
void HomeAssistant::addDeviceInfo(JsonObject& device) {
    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add("hollowclock_" + String(ESP.getChipId()));
    device["name"] = "Hollow Clock";
    device["model"] = "Hollow Clock v2";
    device["manufacturer"] = "DIY";
    device["sw_version"] = FW_VERSION;
    device["configuration_url"] = "http://hollowclock.local";
} 
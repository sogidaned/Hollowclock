#include "webserver.h"
#include "config.h"
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

WebInterface::WebInterface(ClockControl& clockControl)
    : server(80)
    , clockControl(clockControl)
    , homeAssistant(nullptr) {
    httpUpdater.setup(&server);
}

void WebInterface::begin() {
    Serial.println("\n=== Starte Webserver ===");
    
    // Lade MQTT-Konfiguration
    MQTTConfig config;
    EEPROM.get(MQTT_CONFIG_ADDR, config);
    
    if (config.valid) {
        Serial.println("MQTT-Konfiguration gefunden:");
        Serial.printf("Server: %s\n", config.server);
        Serial.printf("Port: %d\n", config.port);
        Serial.printf("User: %s\n", config.user);
        Serial.println("Passwort: " + String(config.password[0] != '\0' ? "gesetzt" : "nicht gesetzt"));
        
        // Initialisiere MQTT
        if (homeAssistant) {
            homeAssistant->begin(
                config.server,
                config.port,
                config.clientId,
                config.topic,
                config.user,
                config.password
            );
        }
    } else {
        Serial.println("Keine gültige MQTT-Konfiguration gefunden");
    }
    
    // API-Routen zuerst registrieren
    server.on("/api/status", HTTP_OPTIONS, [this]() {
        Serial.println("OPTIONS-Anfrage für /api/status");
        sendCORSHeaders();
        server.send(200, "text/plain", "");
    });
    
    server.on("/api/status", HTTP_GET, [this]() {
        Serial.println("GET-Anfrage für /api/status");
        handleGetStatus();
    });
    
    server.on("/api/setTime", HTTP_POST, [this]() {
        Serial.println("POST-Anfrage für /api/setTime");
        handleSetTime();
    });
    
    server.on("/api/adjustTime", HTTP_POST, [this]() {
        Serial.println("POST-Anfrage für /api/adjustTime");
        handleAdjustTime();
    });
    
    server.on("/api/reset_wifi", HTTP_POST, [this]() {
        Serial.println("POST-Anfrage für /api/reset_wifi");
        handleResetWifi();
    });
    
    server.on("/api/restart", HTTP_POST, [this]() {
        Serial.println("POST-Anfrage für /api/restart");
        handleRestart();
    });
    
    // Neuer Endpunkt zum kompletten Zurücksetzen des EEPROM
    server.on("/api/reset_all", HTTP_POST, [this]() {
        Serial.println("POST-Anfrage für /api/reset_all - Komplettes EEPROM-Reset");
        handleResetAllEEPROM();
    });
    
    server.on("/api/mqtt/config", HTTP_POST, [this]() {
        Serial.println("POST-Anfrage für MQTT-Konfiguration");
        handleSetMqttConfig();
    });
    
    server.on("/api/mqtt/config", HTTP_GET, [this]() {
        Serial.println("GET-Anfrage für MQTT-Konfiguration");
        handleGetMqttConfig();
    });
    
    server.on("/api/mqtt/status", HTTP_GET, [this]() {
        Serial.println("GET-Anfrage für MQTT-Status");
        handleGetMqttStatus();
    });
    
    // Neuer Endpunkt zum Zurücksetzen der MQTT-Einstellungen
    server.on("/api/mqtt/reset", HTTP_POST, [this]() {
        Serial.println("POST-Anfrage für MQTT-Reset");
        handleResetMqtt();
    });
    
    // Update-Endpunkte
    httpUpdater.setup(&server);
    
    // Firmware-Update-Seite
    server.on("/firmware", HTTP_GET, [this]() {
        Serial.println("GET-Anfrage für /firmware");
        String html = F("<!DOCTYPE html><html><head>"
            "<title>Firmware Update</title>"
            "<meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1'>"
            "<style>"
            "body{font-family:sans-serif;line-height:1.5;padding:20px;max-width:800px;margin:0 auto;background:#111827;color:#f3f4f6}"
            ".container{background:#1f2937;padding:20px;border-radius:8px;box-shadow:0 4px 6px rgba(0,0,0,0.1)}"
            "h1{margin-top:0;color:#f3f4f6}"
            "form{margin-top:20px}"
            "input[type=file]{display:block;margin:10px 0;padding:10px;width:100%;box-sizing:border-box;background:#374151;color:#f3f4f6;border:1px solid #4f46e5;border-radius:4px}"
            "input[type=submit]{background:#4f46e5;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer}"
            "input[type=submit]:hover{background:#4338ca}"
            ".warning{color:#fbbf24;margin:20px 0;padding:10px;background:#374151;border-radius:4px}"
            "</style>"
            "</head><body>"
            "<div class='container'>"
            "<h1>Firmware Update</h1>"
            "<div class='warning'>⚠️ Achtung: Dies aktualisiert nur die Firmware. Das Dateisystem bleibt erhalten.</div>"
            "<form method='POST' action='/update' enctype='multipart/form-data'>"
            "<input type='file' name='update' accept='.bin'>"
            "<input type='submit' value='Firmware aktualisieren'>"
            "</form>"
            "</div></body></html>");
        server.send(200, "text/html", html);
    });
    
    // Statische Dateien NACH den API-Routen
    server.serveStatic("/", LittleFS, "/");
    
    // 404-Handler als letztes
    server.onNotFound([this]() {
        Serial.print("Nicht gefunden: ");
        Serial.println(server.uri());
        
        if (server.uri().startsWith("/api/")) {
            Serial.println("API-Endpunkt nicht gefunden");
            server.send(404, "application/json", "{\"error\":\"API endpoint not found\"}");
            return;
        }
        
        if (handleFileRead(server.uri())) {
            return;
        }
        
        server.send(404, "text/plain", "File not found");
    });
    
    Serial.println("Routen registriert:");
    Serial.println("- GET  /");
    Serial.println("- GET  /style.css");
    Serial.println("- GET  /api.js");
    Serial.println("- GET  /status.js");
    Serial.println("- GET  /ui.js");
    Serial.println("- GET  /app.js");
    Serial.println("- GET  /api/status");
    Serial.println("- POST /api/setTime");
    Serial.println("- POST /api/adjustTime");
    Serial.println("- POST /api/reset_wifi");
    Serial.println("- POST /api/restart");
    
    server.begin();
    Serial.println("Webserver gestartet auf Port 80");
    Serial.println("============================\n");
}

void WebInterface::handle() {
    server.handleClient();
}

void WebInterface::handleGetStatus() {
    Serial.println("Verarbeite Status-Anfrage");
    sendCORSHeaders();
    
    StaticJsonDocument<512> doc;
    
    // System-Informationen
    doc["version"] = FW_VERSION;
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    
    // Uhr-Informationen
    int currentPos = clockControl.getCurrentPosition();
    String currentTimeStr = clockControl.getCurrentTimeStr();
    String ntpTimeStr = clockControl.getNTPTimeStr();
    
    doc["currentPosition"] = currentPos;
    doc["currentTime"] = currentTimeStr;
    doc["ntpTime"] = ntpTimeStr;
    doc["stepPosition"] = currentPos;
    doc["stepPercent"] = (currentPos * 100) / TOTAL_STEPS;
    
    String response;
    serializeJson(doc, response);
    
    Serial.print("Sende Status-Antwort: ");
    Serial.println(response);
    
    server.send(200, "application/json", response);
}

void WebInterface::handleSetTime() {
    sendCORSHeaders();
    
    if (server.hasArg("hour") && server.hasArg("minute")) {
        int hour = server.arg("hour").toInt();
        int minute = server.arg("minute").toInt();
        
        // Position aus der Zeit berechnen und direkt setzen (ohne Bewegung)
        int position = clockControl.calculateStepsFromTime(hour, minute);
        clockControl.setPosition(position);
        
        // Zur aktuellen NTP-Zeit synchronisieren
        clockControl.syncToNTPTime();
        
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Zeit fehlt\"}");
    }
}

void WebInterface::handleAdjustTime() {
    sendCORSHeaders();
    
    if (server.hasArg("adjustment")) {
        int adjustment = server.arg("adjustment").toInt();
        clockControl.adjustPosition(adjustment);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Anpassung fehlt\"}");
    }
}

void WebInterface::handleResetWifi() {
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"WLAN-Einstellungen werden zurückgesetzt\"}");
    delay(500); // Warte kurz, damit die Antwort gesendet werden kann
    
    // WLAN-Einstellungen zurücksetzen
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    
    // Neustart des ESP
    ESP.restart();
}

void WebInterface::handleRestart() {
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Neustart wird durchgeführt\"}");
    delay(500); // Warte kurz, damit die Antwort gesendet werden kann
    
    // Neustart des ESP
    ESP.restart();
}

void WebInterface::handleGetMqttConfig() {
    sendCORSHeaders();
    
    MQTTConfig config;
    bool configLoaded = false;
    
    // Versuche mehrmals zu laden, mit kurzer Verzögerung
    for (int attempt = 1; attempt <= 3; attempt++) {
        EEPROM.get(MQTT_CONFIG_ADDR, config);
        
        // Validiere config vollständig
        if (config.valid && 
            strlen(config.server) > 0 && 
            config.port > 0 && 
            strlen(config.clientId) > 0 && 
            strlen(config.topic) > 0) {
            
            configLoaded = true;
            Serial.printf("Gültige MQTT-Konfiguration für API beim Versuch %d geladen\n", attempt);
            break;
        } else {
            Serial.printf("MQTT-Konfiguration für API beim Versuch %d ungültig: valid=%d, server=%s\n", 
                attempt, config.valid, config.server);
            delay(50 * attempt);
        }
    }
    
    StaticJsonDocument<512> doc;
    
    if (configLoaded) {
        // Wenn eine gültige Konfiguration existiert, verwende diese
        doc["enabled"] = config.enabled;
        String server = config.server;
        // Entferne mqtt:// falls vorhanden
        if (server.startsWith("mqtt://")) {
            server = server.substring(7);
        }
        doc["server"] = server;
        doc["port"] = config.port;
        doc["clientId"] = config.clientId;
        doc["topic"] = config.topic;
        doc["user"] = config.user;
        
        Serial.println("Sende gültige MQTT-Konfiguration an Client");
    } else {
        // Standardwerte
        Serial.println("Keine gültige Konfiguration gefunden, sende Standardwerte an Client");
        doc["enabled"] = false;  // MQTT standardmäßig deaktiviert
        doc["server"] = DEFAULT_MQTT_SERVER;
        doc["port"] = DEFAULT_MQTT_PORT;
        doc["clientId"] = "Hollowclock";
        doc["topic"] = "Hollowclock";
        doc["user"] = DEFAULT_MQTT_USER;
        
        // Erstelle und speichere gleich eine Standard-Konfiguration im EEPROM
        MQTTConfig defaultConfig;
        defaultConfig.valid = true;
        defaultConfig.enabled = false;  // MQTT standardmäßig deaktiviert
        strlcpy(defaultConfig.server, DEFAULT_MQTT_SERVER, sizeof(defaultConfig.server));
        defaultConfig.port = DEFAULT_MQTT_PORT;
        strlcpy(defaultConfig.clientId, "Hollowclock", sizeof(defaultConfig.clientId));
        strlcpy(defaultConfig.topic, "Hollowclock", sizeof(defaultConfig.topic));
        strlcpy(defaultConfig.user, DEFAULT_MQTT_USER, sizeof(defaultConfig.user));
        strlcpy(defaultConfig.password, DEFAULT_MQTT_PASSWORD, sizeof(defaultConfig.password));
        
        // Speichere die Standard-Konfiguration
        EEPROM.put(MQTT_CONFIG_ADDR, defaultConfig);
        bool success = EEPROM.commit();
        
        if (success) {
            Serial.println("Standard-MQTT-Konfiguration automatisch erstellt und gespeichert");
        } else {
            Serial.println("Fehler beim automatischen Speichern der Standard-MQTT-Konfiguration");
        }
    }
    
    // Passwort wird aus Sicherheitsgründen nie zurückgegeben
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebInterface::handleSetMqttConfig() {
    sendCORSHeaders();
    
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Keine Daten empfangen\"}");
        return;
    }
    
    String json = server.arg("plain");
    Serial.println("Empfangene MQTT-Konfiguration:");
    Serial.println(json);
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.println("JSON-Parsing fehlgeschlagen:");
        Serial.println(error.c_str());
        server.send(400, "application/json", "{\"error\":\"Ungültiges JSON-Format\"}");
        return;
    }
    
    // Lade bestehende Konfiguration
    MQTTConfig config;
    EEPROM.get(MQTT_CONFIG_ADDR, config);
    bool hadValidConfig = config.valid;
    
    // MQTT-Aktivierung (standardmäßig deaktiviert)
    if (doc.containsKey("enabled")) {
        config.enabled = doc["enabled"].as<bool>();
    } else if (!hadValidConfig) {
        config.enabled = false;
    }
    
    // Server-Adresse (Standard: homeassistant.local)
    if (doc.containsKey("server")) {
        String server = doc["server"].as<String>();
        strlcpy(config.server, server.c_str(), sizeof(config.server));
    } else if (!hadValidConfig) {
        strlcpy(config.server, "homeassistant.local", sizeof(config.server));
    }
    
    // Port (Standard: 1883)
    if (doc.containsKey("port")) {
        config.port = doc["port"].as<int>();
    } else if (!hadValidConfig) {
        config.port = 1883;
    }
    
    // Client-ID (Standard: Hollowclock)
    if (doc.containsKey("clientId")) {
        strlcpy(config.clientId, doc["clientId"].as<String>().c_str(), sizeof(config.clientId));
    } else if (!hadValidConfig) {
        strlcpy(config.clientId, "Hollowclock", sizeof(config.clientId));
    }
    
    // Topic (Standard: Hollowclock)
    if (doc.containsKey("topic")) {
        strlcpy(config.topic, doc["topic"].as<String>().c_str(), sizeof(config.topic));
    } else if (!hadValidConfig) {
        strlcpy(config.topic, "Hollowclock", sizeof(config.topic));
    }
    
    // Optional: Benutzer 
    if (doc.containsKey("user")) {
        strlcpy(config.user, doc["user"].as<String>().c_str(), sizeof(config.user));
    } else if (!hadValidConfig) {
        config.user[0] = '\0';
    }
    
    // Optional: Passwort - nur überschreiben wenn angegeben und nicht leer
    if (doc.containsKey("password") && doc["password"].as<String>().length() > 0) {
        strlcpy(config.password, doc["password"].as<String>().c_str(), sizeof(config.password));
        Serial.println("Neues Passwort gesetzt");
    } else {
        Serial.println("Passwort unverändert gelassen");
        // Behalte das vorhandene Passwort bei
    }
    
    config.valid = true;
    
    // Zuerst Original MQTT-Konfiguration als Backup speichern
    MQTTConfig originalConfig;
    EEPROM.get(MQTT_CONFIG_ADDR, originalConfig);
    
    // Speichere in EEPROM - Mehrere Versuche mit Verzögerung
    EEPROM.put(MQTT_CONFIG_ADDR, config);
    bool success = false;
    
    // Bis zu 3 Versuche mit immer längerer Verzögerung
    for (int attempt = 1; attempt <= 3; attempt++) {
        success = EEPROM.commit();
        if (success) {
            Serial.printf("MQTT-Konfiguration erfolgreich beim Versuch %d gespeichert\n", attempt);
            
            // Überprüfe, ob die Daten korrekt gespeichert wurden, indem wir sie zurücklesen
            MQTTConfig verifyConfig;
            EEPROM.get(MQTT_CONFIG_ADDR, verifyConfig);
            
            if (verifyConfig.valid && 
                verifyConfig.enabled == config.enabled &&
                strcmp(verifyConfig.server, config.server) == 0 && 
                verifyConfig.port == config.port && 
                strcmp(verifyConfig.clientId, config.clientId) == 0 && 
                strcmp(verifyConfig.topic, config.topic) == 0) {
                
                Serial.println("MQTT-Konfiguration erfolgreich verifiziert!");
                success = true;
                break;
            } else {
                Serial.println("Fehler bei der Verifizierung der MQTT-Konfiguration!");
                success = false;
            }
        }
        
        // Wenn nicht erfolgreich und nicht der letzte Versuch, versuche es erneut
        if (!success && attempt < 3) {
            int delayMs = 100 * attempt; // 100ms, 200ms, ...
            Serial.printf("Wiederhole EEPROM-Speicherung nach %d ms (Versuch %d/3)...\n", delayMs, attempt+1);
            delay(delayMs);
        }
    }
    
    if (!success) {
        Serial.println("Speichern der MQTT-Konfiguration endgültig fehlgeschlagen!");
        
        // Versuche, zur ursprünglichen Konfiguration zurückzukehren
        EEPROM.put(MQTT_CONFIG_ADDR, originalConfig);
        EEPROM.commit();
        
        server.send(500, "application/json", "{\"error\":\"Speichern im EEPROM fehlgeschlagen\"}");
        return;
    }
    
    // Zeige Debugging-Informationen
    Serial.println("Gespeicherte MQTT-Konfiguration:");
    Serial.printf("MQTT aktiviert: %s\n", config.enabled ? "Ja" : "Nein");
    Serial.printf("Server: %s\n", config.server);
    Serial.printf("Port: %d\n", config.port);
    Serial.printf("Client-ID: %s\n", config.clientId);
    Serial.printf("Topic: %s\n", config.topic);
    Serial.printf("User: %s\n", config.user);
    Serial.println("Passwort: " + String(config.password[0] != '\0' ? "gesetzt" : "nicht gesetzt"));
    
    // Aktualisiere die MQTT-Verbindung nur wenn MQTT aktiviert ist
    if (homeAssistant) {
        if (config.enabled) {
            homeAssistant->reconnect(
                config.server,
                config.port,
                config.clientId,
                config.topic,
                config.user,
                config.password
            );
        } else {
            // Trenne MQTT-Verbindung wenn deaktiviert
            homeAssistant->reconnect("", 0, "", "", "", "");
        }
    }
    
    // Sende Erfolgsantwort
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"MQTT-Konfiguration gespeichert\"}");
}

void WebInterface::handleGetMqttStatus() {
    sendCORSHeaders();
    
    StaticJsonDocument<256> doc;
    
    if (homeAssistant) {
        doc["connected"] = homeAssistant->isConnected();
        doc["lastError"] = homeAssistant->getLastError();
    } else {
        doc["connected"] = false;
        doc["lastError"] = "Home Assistant nicht initialisiert";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

// Neue Handler-Funktion für MQTT-Reset
void WebInterface::handleResetMqtt() {
    sendCORSHeaders();
    
    // Hier extern deklarierte Funktion aufrufen
    extern void resetMQTTSettings();
    resetMQTTSettings();
    
    // Trennen von MQTT, falls verbunden
    if (homeAssistant) {
        homeAssistant->reconnect("", 0, "", "", "", "");
    }
    
    // Erfolgsmeldung senden
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"MQTT-Einstellungen zurückgesetzt\"}");
}

String WebInterface::getContentType(const String& path) {
    if (path.endsWith(".html")) return "text/html";
    else if (path.endsWith(".css")) return "text/css";
    else if (path.endsWith(".js")) return "application/javascript";
    else if (path.endsWith(".ico")) return "image/x-icon";
    else if (path.endsWith(".png")) return "image/png";
    else if (path.endsWith(".jpg")) return "image/jpeg";
    else if (path.endsWith(".gif")) return "image/gif";
    else if (path.endsWith(".svg")) return "image/svg+xml";
    else if (path.endsWith(".woff")) return "font/woff";
    else if (path.endsWith(".woff2")) return "font/woff2";
    else if (path.endsWith(".ttf")) return "font/ttf";
    else if (path.endsWith(".eot")) return "application/vnd.ms-fontobject";
    else if (path.endsWith(".otf")) return "font/otf";
    else if (path.endsWith(".json")) return "application/json";
    else if (path.endsWith(".xml")) return "text/xml";
    else if (path.endsWith(".pdf")) return "application/pdf";
    else if (path.endsWith(".zip")) return "application/zip";
    return "text/plain";
}

bool WebInterface::handleFileRead(const String& path) {
    Serial.print("Datei angefordert: ");
    Serial.println(path);
    
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    
    if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
        if (LittleFS.exists(pathWithGz)) {
            pathWithGz = pathWithGz;
        } else {
            pathWithGz = path;
        }
        
        Serial.print("Sende Datei: ");
        Serial.println(pathWithGz);
        
        File file = LittleFS.open(pathWithGz, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    Serial.println("Datei nicht gefunden");
    return false;
}

void WebInterface::sendCORSHeaders() {
    Serial.println("Sende CORS-Header");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With");
    server.sendHeader("Access-Control-Max-Age", "86400");
}

void WebInterface::handleResetAllEEPROM() {
    sendCORSHeaders();
    
    // Bestätigungsnachricht senden
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Alle Einstellungen werden zurückgesetzt\"}");
    delay(500); // Warte kurz, damit die Antwort gesendet werden kann
    
    // Hier extern deklarierte Funktion aufrufen
    extern void resetAllEEPROM();
    resetAllEEPROM();
} 
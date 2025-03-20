#include <Arduino.h>
#include "config.h"
#include <ESP8266WiFi.h>
#include <AccelStepper.h>
#include <ezTime.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include "clockcontrol.h"
#include "webserver.h"
#include "homeassistant.h"
#include <LittleFS.h>

// Funktionsdeklarationen
void updateClockPosition();
void configModeCallback(WiFiManager *myWiFiManager);
void savePosition(int position);
int loadPosition();
void setupWiFi();
void setupMQTT();
void resetWiFiSettings();
void resetMQTTSettings();
void handleEEPROMTasks();
void saveNetworkInfo();
void checkWiFiConnection();
bool connectToSavedNetwork();
void resetAllEEPROM();
void saveMQTTConfig();

// Pins für den Schrittmotor (ULN2003)
#define IN1 D8
#define IN2 D7
#define IN3 D6
#define IN4 D5

// Globale Objekte
ESP8266WebServer server(80);
WiFiManager wifiManager;

// Hauptobjekte
ClockControl clockControl;
WebInterface webInterface(clockControl);
HomeAssistant* homeAssistant = nullptr;

// Globale Variablen
int lastPosition = 0;
int manualHour = 0;
int manualMinute = 0;

// Neue Variablen für WiFi-Reconnect
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 30000; // 30 Sekunden zwischen Reconnect-Versuchen (statt 60)
const int MIN_RSSI = -85; // Geändert von -80 auf -85 für bessere Toleranz bei schwächerem Signal
int reconnectAttempts = 0;
const int MAX_RECONNECT_ATTEMPTS = 10; // Erhöht von 5 auf 10 für mehr Versuche
unsigned long lastWiFiHealthCheck = 0;
const unsigned long WIFI_HEALTH_CHECK_INTERVAL = 600000; // Alle 10 Minuten WLAN-Status prüfen

// Variablen für regelmäßiges EEPROM-Speichern
unsigned long lastEEPROMSaveMillis = 0;
unsigned long lastMQTTSaveMillis = 0;
const unsigned long EEPROM_SAVE_INTERVAL = 300000; // 5 Minuten
const unsigned long MQTT_SAVE_INTERVAL = 600000;   // 10 Minuten

// Globale Variable für LED-Blinken
unsigned long lastLedToggle = 0;
bool isInConfigMode = false;

// Struktur für WLAN-Informationen im EEPROM
struct WiFiCredentials {
  bool valid;
  char ssid[33];
  char password[65];
};

// Die WIFI_CREDENTIALS_ADDR-Definition wurde nach config.h verschoben

void setup() {
  Serial.begin(115200);
  Serial.println("\nHollow Clock startet...");
  
  // EEPROM initialisieren mit größerem Puffer
  EEPROM.begin(EEPROM_SIZE);
  Serial.printf("EEPROM mit %d Bytes initialisiert\n", EEPROM_SIZE);
  
  // Kurze Verzögerung nach EEPROM-Initialisierung
  delay(100);
  
  // LittleFS initialisieren
  if (!LittleFS.begin()) {
    Serial.println("Fehler beim Initialisieren von LittleFS!");
    delay(3000);
    ESP.restart();
  }
  Serial.println("LittleFS initialisiert");
  
  // WiFi Setup
  setupWiFi();
  
  // ezTime initialisieren
  setInterval(3600); // Aktualisiere die Zeit jede Stunde
  waitForSync();
  
  // Uhr initialisieren
  clockControl.begin();
  
  // Server initialisieren
  webInterface.begin();
  
  // Dateien im LittleFS auflisten
  Serial.println("\nDateien im LittleFS:");
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Serial.print("  ");
    Serial.print(dir.fileName());
    Serial.print("  ");
    Serial.println(dir.fileSize());
  }
  
  // MQTT/Home Assistant Setup
  setupMQTT();
  
  // Speichere sofort die aktuelle Position
  savePosition(clockControl.getCurrentPosition());
  
  Serial.println("Setup abgeschlossen!");
}

void loop() {
  // LED im Konfigurations-Modus blinken lassen
  if (isInConfigMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastLedToggle >= 250) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      lastLedToggle = currentMillis;
    }
  }
  
  // Uhr aktualisieren
  clockControl.update();
  
  // Webserver aktualisieren
  webInterface.handle();
  
  // mDNS aktualisieren
  MDNS.update();
  
  // Home Assistant aktualisieren, nur wenn aktiviert und verbunden
  if (homeAssistant) {
    // Prüfen, ob MQTT aktiviert ist
    MQTTConfig config;
    EEPROM.get(MQTT_CONFIG_ADDR, config);
    
    if (config.valid && config.enabled) {
      homeAssistant->update();
    }
  }
  
  // WiFi-Status prüfen und ggf. neu verbinden
  if (!isInConfigMode) {
    checkWiFiConnection();
  }
  
  // EEPROM-Aufgaben ausführen
  handleEEPROMTasks();
  
  // Speicher freigeben bei knappem Speicher
  if (ESP.getFreeHeap() < 4096) { // Wenn weniger als 4KB frei
    Serial.println("Warnung: Speicher knapp!");
    // ESP.gc() nicht verfügbar - Alternative Speicherfreigabe
    delay(100);
    yield(); // Mehr Zeit für interne Bereinigung geben
  }
  
  // Kurze Pause für ESP8266
  yield();
}

// Regelmäßiges Speichern der Position im EEPROM
void handleEEPROMTasks() {
  unsigned long currentMillis = millis();
  
  // Position regelmäßig im EEPROM speichern (auch wenn sich nichts ändert)
  if (currentMillis - lastEEPROMSaveMillis >= EEPROM_SAVE_INTERVAL) {
    lastEEPROMSaveMillis = currentMillis;
    int currentPos = clockControl.getCurrentPosition();
    savePosition(currentPos);
    Serial.printf("Regelmäßige EEPROM-Sicherung: Position %d gespeichert\n", currentPos);
  }
  
  // MQTT-Konfiguration regelmäßig sichern, aber nur wenn aktiviert
  if (currentMillis - lastMQTTSaveMillis >= MQTT_SAVE_INTERVAL && homeAssistant) {
    lastMQTTSaveMillis = currentMillis;
    
    // Prüfe, ob MQTT aktiviert ist
    MQTTConfig config;
    EEPROM.get(MQTT_CONFIG_ADDR, config);
    
    if (config.valid && config.enabled && homeAssistant->isConnected()) {
      saveMQTTConfig();
    } else {
      Serial.println("MQTT-Sicherung übersprungen: Deaktiviert oder nicht verbunden");
    }
  }
}

void updateClockPosition() {
  // Aktuelle Zeit von clockControl holen
  clockControl.moveToTime(hour(), minute());
}

void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("\n");
  Serial.println("Konfigurations-Modus gestartet");
  Serial.println("*********************************");
  Serial.print("AP-Name: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.print("AP-IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Öffnen Sie http://192.168.4.1 im Browser");
  Serial.println("Nach erfolgreicher Verbindung wird http://hollowclock.local verfügbar sein");
  Serial.println("*********************************");
  
  // LED-Pin konfigurieren
  pinMode(LED_PIN, OUTPUT);
  isInConfigMode = true;
}

void savePosition(int position) {
  // Mehrfache Sicherung der Position um Datenverlust zu vermeiden
  EEPROM.put(POSITION_ADDR, position);
  EEPROM.put(POSITION_ADDR + 4, position); // Backup an anderer Stelle
  bool success = EEPROM.commit();
  if (success) {
    Serial.printf("Position %d gespeichert\n", position);
  } else {
    Serial.printf("Fehler beim Speichern der Position %d\n", position);
    delay(100);
    EEPROM.commit(); // Zweiter Versuch
  }
}

int loadPosition() {
  int position, backupPosition;
  EEPROM.get(POSITION_ADDR, position);
  EEPROM.get(POSITION_ADDR + 4, backupPosition);
  
  // Prüfe zunächst die Hauptposition
  if (position >= 0 && position <= TOTAL_STEPS) {
    Serial.printf("Position %d geladen\n", position);
    return position;
  }
  
  // Falls Hauptposition ungültig ist, prüfe Backup
  if (backupPosition >= 0 && backupPosition <= TOTAL_STEPS) {
    Serial.printf("Backup-Position %d geladen (Hauptposition ungültig)\n", backupPosition);
    return backupPosition;
  }
  
  // Fallback: Position auf 0 setzen
  Serial.println("Ungültige Position gefunden, setze auf 0");
  return 0;
}

void setupWiFi() {
  // WiFi-Parameter setzen für bessere Stabilität
  WiFi.persistent(true);  // WICHTIG: Auf true setzen, damit Konfiguration im Flash gespeichert wird
  
  // LED-Pin für Status konfigurieren
  pinMode(LED_PIN, OUTPUT);
  
  // Prüfen, ob bereits WiFi-Credentials vorhanden sind
  WiFiCredentials credentials;
  memset(&credentials, 0, sizeof(WiFiCredentials)); // Struktur mit 0 initialisieren
  EEPROM.get(WIFI_CREDENTIALS_ADDR, credentials);
  
  // Beim ersten Start (nach Flashen) direkt in den AP-Modus wechseln
  if (!credentials.valid || strlen(credentials.ssid) < 1) {
    Serial.println("Keine gespeicherten WLAN-Daten gefunden - starte direkt im Konfigurationsmodus");
    
    // WiFi Modus explizit setzen
    WiFi.mode(WIFI_STA);
    
    // WiFiManager konfigurieren
    wifiManager.setDebugOutput(true);
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
    wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT);
    wifiManager.setBreakAfterConfig(true);
    wifiManager.setSaveConfigCallback([]() {
      Serial.println("WLAN-Konfiguration gespeichert");
      WiFi.setAutoReconnect(true);
      
      // Speichere Verbindungsdaten
      saveNetworkInfo();
    });
    
    // Optionale Parameter für das Portal
    WiFiManagerParameter custom_text("<p>Willkommen bei Ihrer HollowClock!</p><p>Nach erfolgreicher Verbindung ist die Uhr unter <strong>http://hollowclock.local</strong> im Browser erreichbar.</p>");
    wifiManager.addParameter(&custom_text);
    
    // Generiere eindeutigen AP-Namen
    String apName = String(AP_NAME) + "-" + String(ESP.getChipId(), HEX);
    
    // Starte sofort das Konfigurationsportal ohne Verbindungsversuch
    Serial.println("Starte Konfigurationsportal...");
    if (!wifiManager.startConfigPortal(apName.c_str(), AP_PASSWORD)) {
      Serial.println("Konfigurationsportal-Timeout, starte neu...");
      delay(3000);
      ESP.restart();
    }
    
    // Nach erfolgreicher Verbindung
    isInConfigMode = false;
    Serial.println("WiFi verbunden!");
    Serial.printf("IP-Adresse: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signalstärke: %d dBm\n", WiFi.RSSI());
    
    // Speichere sofort die Netzwerkinformationen
    saveNetworkInfo();
    
    // LED einmal lang blinken lassen zur Bestätigung
    digitalWrite(LED_PIN, LOW);
    delay(1000);
    digitalWrite(LED_PIN, HIGH);
    
    // mDNS initialisieren
    if (MDNS.begin("hollowclock")) {
      Serial.println("mDNS gestartet: http://hollowclock.local");
    } else {
      Serial.println("Fehler beim Starten von mDNS!");
    }
    
    return;
  }
  
  // Wenn WLAN-Daten vorhanden sind, versuche mit gespeicherten Daten zu verbinden
  Serial.println("Gespeicherte WLAN-Daten gefunden - versuche zu verbinden...");
  if (connectToSavedNetwork()) {
    Serial.println("Verbindung mit gespeicherten Daten erfolgreich!");
    isInConfigMode = false;
    
    // Nach erfolgreicher Verbindung
    Serial.println("WiFi verbunden!");
    Serial.printf("IP-Adresse: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signalstärke: %d dBm\n", WiFi.RSSI());
    
    // LED einmal lang blinken lassen zur Bestätigung
    digitalWrite(LED_PIN, LOW);
    delay(1000);
    digitalWrite(LED_PIN, HIGH);
    
    // mDNS initialisieren
    if (MDNS.begin("hollowclock")) {
      Serial.println("mDNS gestartet: http://hollowclock.local");
    } else {
      Serial.println("Fehler beim Starten von mDNS!");
    }
    
    return; // Verlasse die Funktion, da WLAN bereits verbunden ist
  }
  
  // Wenn Verbindung mit gespeicherten Daten fehlschlägt, nutze WiFiManager
  Serial.println("Konnte nicht mit gespeicherten Daten verbinden, starte WiFiManager...");
  
  // WiFi Modus explizit setzen
  WiFi.mode(WIFI_STA);
  
  // WiFiManager konfigurieren
  wifiManager.setDebugOutput(true);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
  wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT);
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback([]() {
    Serial.println("WLAN-Konfiguration gespeichert");
    WiFi.setAutoReconnect(true);
    
    // Speichere Verbindungsdaten
    saveNetworkInfo();
  });
  
  // Optionale Parameter für das Portal
  WiFiManagerParameter custom_text("<p>Willkommen bei Ihrer HollowClock!</p><p>Nach erfolgreicher Verbindung ist die Uhr unter <strong>http://hollowclock.local</strong> im Browser erreichbar.</p>");
  wifiManager.addParameter(&custom_text);
  
  // Generiere eindeutigen AP-Namen
  String apName = String(AP_NAME) + "-" + String(ESP.getChipId(), HEX);
  
  // Versuche eine Verbindung herzustellen oder starte das Konfigurationsportal
  if(!wifiManager.autoConnect(apName.c_str(), AP_PASSWORD)) {
    Serial.println("Verbindung fehlgeschlagen, starte neu...");
    delay(3000);
    ESP.restart();
  }
  
  // Nach erfolgreicher Verbindung
  isInConfigMode = false;
  Serial.println("WiFi verbunden!");
  Serial.printf("IP-Adresse: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Signalstärke: %d dBm\n", WiFi.RSSI());
  
  // Speichere sofort die Netzwerkinformationen
  saveNetworkInfo();
  
  // LED einmal lang blinken lassen zur Bestätigung
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  
  // mDNS initialisieren
  if (MDNS.begin("hollowclock")) {
    Serial.println("mDNS gestartet: http://hollowclock.local");
  } else {
    Serial.println("Fehler beim Starten von mDNS!");
  }
}

void setupMQTT() {
  Serial.println("\n=== MQTT Setup ===");
  
  // EEPROM-Zugriff verzögern
  delay(100);
  
  // MQTTConfig-Struktur für den Ladevorgang
  MQTTConfig config;
  bool configLoaded = false;
  
  // Versuche mehrmals zu laden, mit kurzer Verzögerung
  for (int attempt = 1; attempt <= 3; attempt++) {
    EEPROM.get(MQTT_CONFIG_ADDR, config);
    
    // Validiere config (statt nur config.valid zu prüfen)
    if (config.valid && 
        strlen(config.server) > 0 && 
        config.port > 0 && 
        strlen(config.clientId) > 0 && 
        strlen(config.topic) > 0) {
      
      configLoaded = true;
      Serial.printf("Gültige MQTT-Konfiguration beim Versuch %d geladen\n", attempt);
      break;
    } else {
      Serial.printf("MQTT-Konfiguration beim Versuch %d ungültig oder leer: valid=%d, server=%s, port=%d\n", 
                     attempt, config.valid, config.server, config.port);
      delay(50 * attempt);
    }
  }
  
  homeAssistant = new HomeAssistant(clockControl);
  if (!homeAssistant) {
    Serial.println("Fehler: HomeAssistant konnte nicht erstellt werden!");
    return;
  }
  
  // HomeAssistant-Pointer im WebInterface setzen
  webInterface.setHomeAssistant(homeAssistant);
  
  if (configLoaded) {
    // Prüfen, ob MQTT aktiviert ist
    if (!config.enabled) {
      Serial.println("MQTT ist deaktiviert - kein Verbindungsversuch");
      return;
    }
    
    Serial.println("Verwende geladene MQTT-Konfiguration:");
    Serial.printf("MQTT aktiviert: %s\n", config.enabled ? "Ja" : "Nein");
    Serial.printf("Server: %s\n", config.server);
    Serial.printf("Port: %d\n", config.port);
    Serial.printf("Client-ID: %s\n", config.clientId);
    Serial.printf("Topic: %s\n", config.topic);
    Serial.printf("User: %s\n", config.user);
    Serial.println("Passwort: " + String(config.password[0] != '\0' ? "gesetzt" : "nicht gesetzt"));
    
    // Hole Kopien der Strings, da reconnect keine Pointer akzeptiert
    String server = String(config.server);
    String clientId = String(config.clientId);
    String topic = String(config.topic);
    String user = String(config.user);
    String password = String(config.password);
    
    homeAssistant->begin(
      server.c_str(),
      config.port,
      clientId.c_str(),
      topic.c_str(),
      user.c_str(),
      password.c_str()
    );
    
    // Speichere Konfiguration nochmal, um sicherzustellen, dass sie korrekt im EEPROM ist
    EEPROM.put(MQTT_CONFIG_ADDR, config);
    if (EEPROM.commit()) {
      Serial.println("MQTT-Konfiguration zur Sicherheit im EEPROM gespeichert");
    }
    
    // Sofortige Verbindung versuchen
    homeAssistant->connect();
  } else {
    Serial.println("Keine gültige Konfiguration gefunden, erstelle Standardkonfiguration");
    
    // Erstelle eine neue Standardkonfiguration
    MQTTConfig defaultConfig;
    defaultConfig.valid = true;
    defaultConfig.enabled = false;  // MQTT standardmäßig deaktiviert
    strlcpy(defaultConfig.server, DEFAULT_MQTT_SERVER, sizeof(defaultConfig.server));
    defaultConfig.port = DEFAULT_MQTT_PORT;
    strlcpy(defaultConfig.clientId, "Hollowclock", sizeof(defaultConfig.clientId));
    strlcpy(defaultConfig.topic, "Hollowclock", sizeof(defaultConfig.topic));
    strlcpy(defaultConfig.user, DEFAULT_MQTT_USER, sizeof(defaultConfig.user));
    strlcpy(defaultConfig.password, DEFAULT_MQTT_PASSWORD, sizeof(defaultConfig.password));
    
    // Speichere Standard-Konfiguration - mehrere Versuche
    bool saved = false;
    for (int attempt = 1; attempt <= 3; attempt++) {
      EEPROM.put(MQTT_CONFIG_ADDR, defaultConfig);
      saved = EEPROM.commit();
      
      if (saved) {
        Serial.printf("Standard-MQTT-Konfiguration beim Versuch %d gespeichert\n", attempt);
        
        // Verifizieren
        MQTTConfig verifyConfig;
        EEPROM.get(MQTT_CONFIG_ADDR, verifyConfig);
        
        if (verifyConfig.valid) {
          Serial.println("Standard-Konfiguration erfolgreich verifiziert");
          break;
        } else {
          Serial.println("Verifikation fehlgeschlagen!");
          saved = false;
        }
      }
      
      if (!saved && attempt < 3) {
        delay(100 * attempt);
      }
    }
    
    Serial.println("MQTT ist standardmäßig deaktiviert - kein Verbindungsversuch");
  }
  Serial.println("=== MQTT Setup Ende ===\n");
}

// Reset-Funktion für WLAN-Einstellungen
void resetWiFiSettings() {
  Serial.println("WLAN-Einstellungen werden zurückgesetzt...");
  
  // WLAN-Einstellungen im WifiManager zurücksetzen
  wifiManager.resetSettings();
  
  // Zusätzlich eigene Anmeldedaten im EEPROM löschen
  WiFiCredentials emptyCredentials;
  emptyCredentials.valid = false;
  memset(emptyCredentials.ssid, 0, sizeof(emptyCredentials.ssid));
  memset(emptyCredentials.password, 0, sizeof(emptyCredentials.password));
  
  // Im EEPROM speichern
  EEPROM.put(WIFI_CREDENTIALS_ADDR, emptyCredentials);
  bool success = EEPROM.commit();
  
  if (success) {
    Serial.println("WLAN-Einstellungen erfolgreich zurückgesetzt");
  } else {
    Serial.println("Fehler beim Zurücksetzen der WLAN-Einstellungen");
  }
  
  // Gerät neu starten
  delay(500);
  ESP.restart();
}

// Neue Funktion: MQTT-Einstellungen zurücksetzen
void resetMQTTSettings() {
  Serial.println("MQTT-Einstellungen werden zurückgesetzt...");
  
  // Leere Konfiguration erstellen
  MQTTConfig emptyConfig;
  memset(&emptyConfig, 0, sizeof(MQTTConfig)); // Komplette Struktur mit 0 befüllen
  emptyConfig.valid = false;
  emptyConfig.enabled = false;  // Explizit deaktivieren
  
  // Im EEPROM speichern - Mehrere Versuche mit Verzögerung
  bool success = false;
  
  for (int attempt = 1; attempt <= 3; attempt++) {
    // Zuerst den Bereich im EEPROM mit 0 überschreiben
    for (unsigned int i = MQTT_CONFIG_ADDR; i < MQTT_CONFIG_ADDR + sizeof(MQTTConfig); i++) {
      EEPROM.write(i, 0);
    }
    
    // Dann die leere Konfiguration schreiben
    EEPROM.put(MQTT_CONFIG_ADDR, emptyConfig);
    
    success = EEPROM.commit();
    if (success) {
      Serial.printf("MQTT-Einstellungen beim Versuch %d zurückgesetzt\n", attempt);
      
      // Verifizieren
      MQTTConfig verifyConfig;
      EEPROM.get(MQTT_CONFIG_ADDR, verifyConfig);
      
      if (!verifyConfig.valid && !verifyConfig.enabled) {
        Serial.println("MQTT-Reset erfolgreich verifiziert!");
        break;
      } else {
        Serial.println("MQTT-Reset-Verifizierung fehlgeschlagen!");
        success = false;
      }
    }
    
    if (!success && attempt < 3) {
      int delayMs = 100 * attempt;
      Serial.printf("Wiederhole MQTT-Reset nach %d ms (Versuch %d/3)...\n", delayMs, attempt+1);
      delay(delayMs);
    }
  }
  
  if (success) {
    Serial.println("MQTT-Einstellungen erfolgreich zurückgesetzt");
    
    // Erstelle eine neue Standardkonfiguration
    MQTTConfig defaultConfig;
    defaultConfig.valid = true;
    defaultConfig.enabled = false;  // MQTT standardmäßig deaktiviert
    strlcpy(defaultConfig.server, DEFAULT_MQTT_SERVER, sizeof(defaultConfig.server));
    defaultConfig.port = DEFAULT_MQTT_PORT;
    strlcpy(defaultConfig.clientId, "Hollowclock", sizeof(defaultConfig.clientId));
    strlcpy(defaultConfig.topic, "Hollowclock", sizeof(defaultConfig.topic));
    strlcpy(defaultConfig.user, DEFAULT_MQTT_USER, sizeof(defaultConfig.user));
    strlcpy(defaultConfig.password, DEFAULT_MQTT_PASSWORD, sizeof(defaultConfig.password));
    
    // Speichere Standard-Konfiguration
    delay(200); // Kurze Pause vor dem erneuten Speichern
    EEPROM.put(MQTT_CONFIG_ADDR, defaultConfig);
    bool configSaved = EEPROM.commit();
    
    if (configSaved) {
      Serial.println("Neue Standard-MQTT-Konfiguration erfolgreich gespeichert");
    } else {
      Serial.println("Fehler beim Speichern der Standard-MQTT-Konfiguration");
    }
  } else {
    Serial.println("Fehler beim Zurücksetzen der MQTT-Einstellungen nach mehreren Versuchen");
  }
}

// Neue Funktion: Gesamtes EEPROM zurücksetzen
void resetAllEEPROM() {
  Serial.println("Vollständiges Zurücksetzen des EEPROM...");
  
  // Alle Bytes im EEPROM auf 0 setzen
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  
  bool success = EEPROM.commit();
  
  if (success) {
    Serial.println("EEPROM erfolgreich zurückgesetzt");
  } else {
    Serial.println("Fehler beim Zurücksetzen des EEPROM");
  }
  
  // Gerät neu starten
  delay(500);
  ESP.restart();
}

// Überprüfung der WiFi-Verbindung mit verbesserter Logik
void checkWiFiConnection() {
  unsigned long currentMillis = millis();
  int rssi = WiFi.RSSI();
  
  if (WiFi.status() != WL_CONNECTED || rssi < MIN_RSSI) {
    if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
      reconnectAttempts++;
      Serial.printf("WiFi-Verbindung verloren oder schwach (RSSI: %d dBm). Versuch %d von %d\n", 
                   rssi, reconnectAttempts, MAX_RECONNECT_ATTEMPTS);
      
      if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        Serial.println("Maximale Reconnect-Versuche erreicht. Versuche gespeicherte Verbindung...");
        
        // Sicherstellen, dass die Verbindungsinformationen aktuell sind
        if (WiFi.status() == WL_CONNECTED) {
          saveNetworkInfo();
        }
        
        // Verbindung trennen und neu verbinden
        WiFi.disconnect();
        delay(1000);
        
        // Versuche mit gespeicherten Daten zu verbinden
        if (!connectToSavedNetwork()) {
          Serial.println("Gespeicherte Verbindung fehlgeschlagen. Neustart...");
          delay(1000);
          ESP.restart();
        } else {
          reconnectAttempts = 0;
        }
        return;
      }
      
      // Versuche direkte Wiederverbindung
      WiFi.disconnect();
      delay(1000);
      WiFi.reconnect();
      lastReconnectAttempt = currentMillis;
    }
  } else {
    reconnectAttempts = 0; // Reset bei erfolgreicher Verbindung
    
    // Regelmäßiger WiFi-Gesundheitscheck
    if (currentMillis - lastWiFiHealthCheck >= WIFI_HEALTH_CHECK_INTERVAL) {
      lastWiFiHealthCheck = currentMillis;
      Serial.printf("WiFi-Gesundheitscheck: RSSI=%d dBm, IP=%s\n", 
                   rssi, WiFi.localIP().toString().c_str());
      
      // Speichere aktuelle Verbindungsdaten regelmäßig
      saveNetworkInfo();
    }
  }
}

// Verbesserte Funktion zum Speichern der Netzwerkinformationen
void saveNetworkInfo() {
  WiFiCredentials credentials;
  credentials.valid = true;
  strlcpy(credentials.ssid, WiFi.SSID().c_str(), sizeof(credentials.ssid));
  strlcpy(credentials.password, WiFi.psk().c_str(), sizeof(credentials.password));
  
  EEPROM.put(WIFI_CREDENTIALS_ADDR, credentials);
  bool success = EEPROM.commit();
  
  if (success) {
    Serial.printf("Netzwerkinformationen erfolgreich gespeichert (SSID: %s)\n", credentials.ssid);
  } else {
    Serial.println("Fehler beim Speichern der Netzwerkinformationen!");
    // Zweiter Versuch nach kurzer Pause
    delay(200);
    success = EEPROM.commit();
    if (success) {
      Serial.println("Netzwerkinformationen beim zweiten Versuch gespeichert");
    } else {
      Serial.println("Speichern der Netzwerkinformationen endgültig fehlgeschlagen!");
    }
  }
}

// Verbesserte Funktion zum Verbinden mit gespeicherten Daten
bool connectToSavedNetwork() {
  WiFiCredentials credentials;
  EEPROM.get(WIFI_CREDENTIALS_ADDR, credentials);
  
  if (!credentials.valid || strlen(credentials.ssid) < 1) {
    Serial.println("Keine gültigen Netzwerkdaten gefunden");
    return false;
  }
  
  Serial.printf("Versuche Verbindung zu gespeichertem Netzwerk: %s\n", credentials.ssid);
  
  // Wichtig: Setze WiFi-Parameter
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
  // Starte die Verbindung
  WiFi.begin(credentials.ssid, credentials.password);
  
  // LED blinken lassen während des Verbindungsversuchs
  int ledState = LOW;
  
  unsigned long timeout = WIFI_CONNECT_TIMEOUT * 1000UL; // Millisekunden als unsigned long
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
    // LED-Status umschalten
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Erfolgreich mit gespeichertem Netzwerk verbunden. IP: %s\n", WiFi.localIP().toString().c_str());
    // Stelle sicher, dass die LED nach erfolgreicher Verbindung aus ist
    digitalWrite(LED_PIN, HIGH);
    return true;
  } else {
    Serial.println("Verbindung zum gespeicherten Netzwerk fehlgeschlagen");
    return false;
  }
}

// Funktion zum Sichern der aktuellen MQTT-Konfiguration
void saveMQTTConfig() {
  // Original-Konfiguration als Backup laden
  MQTTConfig originalConfig;
  EEPROM.get(MQTT_CONFIG_ADDR, originalConfig);
  
  // Nur MQTT-Konfiguration sichern, wenn sie aktiviert ist
  if (!originalConfig.enabled) {
    Serial.println("MQTT ist deaktiviert - keine Sicherung erforderlich");
    return;
  }
  
  // Prüfe, ob die Konfiguration gültig ist
  if (originalConfig.valid && 
      strlen(originalConfig.server) > 0 && 
      originalConfig.port > 0 && 
      strlen(originalConfig.clientId) > 0 && 
      strlen(originalConfig.topic) > 0) {
    
    Serial.println("Sichere aktuelle MQTT-Konfiguration...");
    
    // Mehrere Speicherversuche mit zunehmender Verzögerung
    bool success = false;
    
    for (int attempt = 1; attempt <= 3; attempt++) {
      // Kurze Verzögerung vor dem Speichern
      delay(50 * attempt);
      
      // Speichere Konfiguration im EEPROM
      EEPROM.put(MQTT_CONFIG_ADDR, originalConfig);
      success = EEPROM.commit();
      
      if (success) {
        // Verifiziere die gespeicherten Daten
        MQTTConfig verifyConfig;
        EEPROM.get(MQTT_CONFIG_ADDR, verifyConfig);
        
        if (verifyConfig.valid && 
            verifyConfig.enabled &&
            strcmp(verifyConfig.server, originalConfig.server) == 0 && 
            verifyConfig.port == originalConfig.port && 
            strcmp(verifyConfig.clientId, originalConfig.clientId) == 0 && 
            strcmp(verifyConfig.topic, originalConfig.topic) == 0) {
          
          Serial.printf("MQTT-Konfiguration erfolgreich beim Versuch %d gesichert\n", attempt);
          break;
        } else {
          Serial.println("Verifikation der MQTT-Konfiguration fehlgeschlagen!");
          success = false;
        }
      }
      
      if (!success && attempt < 3) {
        Serial.printf("MQTT-Speicherversuch %d fehlgeschlagen, versuche erneut...\n", attempt);
      }
    }
    
    if (!success) {
      Serial.println("Sichern der MQTT-Konfiguration nach mehreren Versuchen fehlgeschlagen!");
    }
  } else {
    Serial.println("Keine gültige MQTT-Konfiguration zum Sichern gefunden");
    
    // Erstelle eine Standard-Konfiguration, wenn keine vorhanden ist
    MQTTConfig defaultConfig;
    defaultConfig.valid = true;
    defaultConfig.enabled = false;  // Standardmäßig deaktiviert
    strlcpy(defaultConfig.server, DEFAULT_MQTT_SERVER, sizeof(defaultConfig.server));
    defaultConfig.port = DEFAULT_MQTT_PORT;
    strlcpy(defaultConfig.clientId, "Hollowclock", sizeof(defaultConfig.clientId));
    strlcpy(defaultConfig.topic, "Hollowclock", sizeof(defaultConfig.topic));
    strlcpy(defaultConfig.user, DEFAULT_MQTT_USER, sizeof(defaultConfig.user));
    strlcpy(defaultConfig.password, DEFAULT_MQTT_PASSWORD, sizeof(defaultConfig.password));
    
    // Speichere Standard-Konfiguration
    EEPROM.put(MQTT_CONFIG_ADDR, defaultConfig);
    bool success = EEPROM.commit();
    
    if (success) {
      Serial.println("Standard-MQTT-Konfiguration erfolgreich gespeichert");
    } else {
      Serial.println("Fehler beim Speichern der Standard-MQTT-Konfiguration");
    }
  }
} 
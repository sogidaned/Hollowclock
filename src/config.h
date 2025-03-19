#pragma once

// Version
#define FW_VERSION "1.0.6"  // Semantic Versioning: Major.Minor.Patch

// GitHub Update Konfiguration
#define GITHUB_REPO_OWNER "sogidaned"
#define GITHUB_REPO_NAME "Hollowclock"
#define GITHUB_USER_AGENT "ESP8266-HollowclockUpdater/1.0"

// GitHub API-Einstellungen
// Wähle HTTPS (Standard) oder HTTP
#define USE_GITHUB_HTTPS false  // Auf false gesetzt, um HTTP zu verwenden
// Wenn HTTP verwendet wird, kann ein öffentlicher API-Proxy verwendet werden
#define GITHUB_API_PROXY "api.allorigins.win/raw?url=https://"

// Uncomment und füge dein Token ein, falls das Rate-Limit überschritten wird
// #define GITHUB_TOKEN "ghp_yourPersonalAccessToken"

// Konstanten für die Uhr
const int TOTAL_STEPS = 491520;  // 12 Stunden in Schritten (Half-Step Modus)
const int STEPS_PER_HALF_MINUTE = 341;

// EEPROM Konfiguration
const int EEPROM_SIZE = 1024;  // Vergrößert für zuverlässigere Speicherung
const int POSITION_ADDR = 0;   // Position braucht 4 Bytes
const int POSITION_BACKUP_ADDR = 4;  // Backup der Position
// Deutlich voneinander trennen, mit ausreichend Puffer
const int MQTT_CONFIG_ADDR = 16;  // MQTT-Config startet bei 16
const int WIFI_CREDENTIALS_ADDR = 512; // WiFi-Credentials weit genug entfernt

const int MQTT_SERVER_MAX_LENGTH = 64;
const int MQTT_USER_MAX_LENGTH = 32;
const int MQTT_PASSWORD_MAX_LENGTH = 32;
const int MQTT_TOPIC_PREFIX_MAX_LENGTH = 32;

// Standard MQTT-Einstellungen
#define DEFAULT_MQTT_SERVER "homeassistant.local"
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_USER ""
#define DEFAULT_MQTT_PASSWORD ""
#define DEFAULT_MQTT_TOPIC_PREFIX "hollowclock"

// Pins für den Schrittmotor (ULN2003)
#define IN1 D8
#define IN2 D7
#define IN3 D6
#define IN4 D5

// Home Assistant Konfiguration
#define MQTT_SERVER "your_mqtt_server"
#define MQTT_PORT 1883
#define MQTT_USER "your_mqtt_user"
#define MQTT_PASSWORD "your_mqtt_password"
#define MQTT_TOPIC_PREFIX "hollowclock"

// WiFi Konfiguration für AP-Modus
#define AP_NAME "HollowClock"  // Einfacherer Name
#define AP_PASSWORD "12345678"
#define AP_TIMEOUT 600  // 10 Minuten statt 5
#define CONFIG_PORTAL_TIMEOUT 600  // 10 Minuten Timeout für das Konfigurationsportal
#define WIFI_CONNECT_TIMEOUT 60    // 60 Sekunden Timeout für WiFi-Verbindung (erhöht von 30)
#define LED_PIN LED_BUILTIN       // LED für Status-Anzeige

// MQTT Konfigurationsstruktur
struct MQTTConfig {
    bool valid;
    bool enabled;     // Neues Flag, um MQTT zu aktivieren/deaktivieren
    char server[128];
    int port;
    char clientId[64];
    char topic[64];
    char user[32];
    char password[32];
}; 
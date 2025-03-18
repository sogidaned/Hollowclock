#include <AccelStepper.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

// Konstanten
const int TOTAL_STEPS = 491520;  // 12 Stunden in Schritten (Half-Step Modus)
const int STEPS_PER_HALF_MINUTE = 341;

// Pins für den Schrittmotor (ULN2003)
#define IN1 D8
#define IN2 D7
#define IN3 D6
#define IN4 D5

// WLAN-Zugangsdaten
const char* ssid = "IHRE_SSID";
const char* password = "IHR_PASSWORD";

// Webserver auf Port 80
ESP8266WebServer server(80);

// NTP-Einstellungen
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 30*60*1000); // GMT +1 für Deutschland

// Stepper-Motor-Konfiguration
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

// Globale Variablen
int lastPosition = 0;
int manualHour = 0;
int manualMinute = 0;

void setup() {
  Serial.begin(115200);
  
  // Stepper-Konfiguration
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(50);
  
  // WLAN-Verbindung
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // NTP starten
  timeClient.begin();
  
  // Webserver-Routen
  setupWebServer();
  server.begin();
  
  // Initialisiere die Position basierend auf der aktuellen Zeit
  timeClient.update();
  int currentHour = timeClient.getHours() % 12;
  int currentMinute = timeClient.getMinutes();
  int currentMinutes = currentHour * 60 + currentMinute;
  lastPosition = (currentMinutes * TOTAL_STEPS) / 720;
  stepper.setCurrentPosition(lastPosition);
}

void loop() {
  server.handleClient();
  
  // Aktualisiere Zeit alle 30 Sekunden
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 30000) {
    timeClient.update();
    updateClockPosition();
    lastUpdate = millis();
  }
  
  stepper.run();
}

void updateClockPosition() {
  int currentHour = timeClient.getHours() % 12;
  int currentMinute = timeClient.getMinutes();
  int currentMinutes = currentHour * 60 + currentMinute;
  int targetSteps = (currentMinutes * TOTAL_STEPS) / 720;
  
  // Sonderbehandlung für Mitternacht und Mittag
  if (targetSteps == 0 && lastPosition > TOTAL_STEPS/2) {
    stepper.moveTo(TOTAL_STEPS);
    lastPosition = TOTAL_STEPS;
  } else if (targetSteps == TOTAL_STEPS && lastPosition < TOTAL_STEPS/2) {
    stepper.moveTo(TOTAL_STEPS);
    lastPosition = TOTAL_STEPS;
  } else {
    stepper.moveTo(targetSteps);
    lastPosition = targetSteps;
  }
}

void setupWebServer() {
  // Webseite für manuelle Einstellung
  server.on("/", HTTP_GET, []() {
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;} input,button{margin:10px 0;padding:5px;}</style></head>";
    html += "<body><h2>Hohlwellen-Uhr Einstellung</h2>";
    html += "<form action='/setTime' method='get'>";
    html += "Stunden (0-12): <input type='number' name='hour' min='0' max='12'><br>";
    html += "Minuten (0-59): <input type='number' name='minute' min='0' max='59'><br>";
    html += "<button type='submit'>Zeit einstellen</button></form>";
    html += "<button onclick='location.href=\"/adjust/forward\"'>+30 Sekunden</button>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  // Handler für Zeiteinstellung
  server.on("/setTime", HTTP_GET, []() {
    if (server.hasArg("hour") && server.hasArg("minute")) {
      manualHour = server.arg("hour").toInt();
      manualMinute = server.arg("minute").toInt();
      int totalManualMinutes = manualHour * 60 + manualMinute;
      int targetSteps = (totalManualMinutes * TOTAL_STEPS) / 720;
      stepper.setCurrentPosition(targetSteps);
      lastPosition = targetSteps;
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "Updated");
  });
  
  // Handler für Feineinstellung
  server.on("/adjust/forward", HTTP_GET, []() {
    int newTarget = stepper.currentPosition() - STEPS_PER_HALF_MINUTE;
    stepper.setCurrentPosition(newTarget);
    lastPosition = newTarget;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "Adjusted");
  });
} 
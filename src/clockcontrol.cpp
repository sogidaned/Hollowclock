#include "clockcontrol.h"
#include "config.h"
#include <ezTime.h>
#include <ESP8266WiFi.h>

ClockControl::ClockControl() 
    : stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4)
    , lastPosition(0) {
}

void ClockControl::begin() {
    initStepper();
    lastTimeCheck = 0;  // Timer initialisieren
    
    // Zeitzone setzen
    waitForSync();
    berlinTZ.setLocation("Europe/Berlin");
    
    // Gespeicherte Position laden und setzen
    lastPosition = loadPosition();
    stepper.setCurrentPosition(lastPosition);
    Serial.printf("Starte mit gespeicherter Position: %d\n", lastPosition);
    
    // Initiale Zeit speichern
    updateStoredTime();
    
    // Zur aktuellen NTP-Zeit synchronisieren
    syncToNTPTime();
}

void ClockControl::initStepper() {
    stepper.setMaxSpeed(500);
    stepper.setAcceleration(50);
}

void ClockControl::update() {
    // Stepper aktualisieren
    stepper.run();
    
    // Vorsichtige Behandlung der Position bei Erreichen von TOTAL_STEPS
    if (!stepper.isRunning() && stepper.currentPosition() >= TOTAL_STEPS - 5) {
        // Kleine Verzögerung für Stabilität
        delay(50);
        
        // Langsam die Geschwindigkeit reduzieren
        int origSpeed = stepper.maxSpeed();
        stepper.setMaxSpeed(100);  // Langsamere Geschwindigkeit
        
        Serial.println("TOTAL_STEPS fast erreicht. Reduziere Geschwindigkeit und setze Position auf 0.");
        
        // Setze Position exakt auf TOTAL_STEPS und dann auf 0
        stepper.setCurrentPosition(TOTAL_STEPS);
        stepper.moveTo(TOTAL_STEPS);
        stepper.runToPosition();  // Warte auf Zielposition
        delay(100);  // Kleine Pause
        
        stepper.setCurrentPosition(0);
        
        // Restore original speed
        stepper.setMaxSpeed(origSpeed);
    }
    
    // Vorsichtige Behandlung der Position bei Erreichen von 0
    if (!stepper.isRunning() && stepper.currentPosition() <= 5 && stepper.currentPosition() > 0 && 
        stepper.targetPosition() == TOTAL_STEPS) {
        
        // Kleine Verzögerung für Stabilität
        delay(50);
        
        // Langsam die Geschwindigkeit reduzieren
        int origSpeed = stepper.maxSpeed();
        stepper.setMaxSpeed(100);  // Langsamere Geschwindigkeit
        
        Serial.println("Position 0 fast erreicht. Reduziere Geschwindigkeit und setze auf TOTAL_STEPS.");
        
        // Setze Position exakt auf 0 und dann auf TOTAL_STEPS
        stepper.setCurrentPosition(0);
        stepper.moveTo(0);
        stepper.runToPosition();  // Warte auf Zielposition
        delay(100);  // Kleine Pause
        
        stepper.setCurrentPosition(TOTAL_STEPS);
        
        // Restore original speed
        stepper.setMaxSpeed(origSpeed);
    }
    
    // Prüfen ob eine Minute vergangen ist (60000ms)
    unsigned long currentMillis = millis();
    static bool lastSyncSuccessful = true;
    
    if (currentMillis - lastTimeCheck >= 60000) {
        lastTimeCheck = currentMillis;
        
        // Gespeicherte Zeit aktualisieren
        updateStoredTime();
        
        // Versuche NTP-Synchronisation
        if (WiFi.status() == WL_CONNECTED) {
            if (syncToNTPTime()) {
                lastSyncSuccessful = true;
                Serial.println("NTP-Synchronisation erfolgreich");
            } else {
                if (lastSyncSuccessful) {
                    Serial.println("NTP-Synchronisation fehlgeschlagen - verwende interne Zeit");
                }
                lastSyncSuccessful = false;
                // Verwende interne Zeit als Fallback
                time_t now = berlinTZ.now();
                struct tm *timeinfo = localtime(&now);
                int currentHour = timeinfo->tm_hour % 12;
                int currentMinute = timeinfo->tm_min;
                if (currentHour == 0) currentHour = 12;
                
                // Bewege zur internen Zeit
                moveToTime(currentHour, currentMinute);
            }
        } else {
            if (lastSyncSuccessful) {
                Serial.println("Keine WLAN-Verbindung - verwende interne Zeit");
            }
            lastSyncSuccessful = false;
            // Verwende interne Zeit als Fallback
            time_t now = berlinTZ.now();
            struct tm *timeinfo = localtime(&now);
            int currentHour = timeinfo->tm_hour % 12;
            int currentMinute = timeinfo->tm_min;
            if (currentHour == 0) currentHour = 12;
            
            // Bewege zur internen Zeit
            moveToTime(currentHour, currentMinute);
        }
    }
    
    // Position speichern wenn keine Bewegung
    if (!stepper.isRunning() && lastPosition != stepper.currentPosition()) {
        lastPosition = stepper.currentPosition();
        savePosition();
    }
}

void ClockControl::setPosition(int position) {
    // Position validieren
    if (position < 0) position = TOTAL_STEPS + (position % TOTAL_STEPS);
    if (position >= TOTAL_STEPS) position = position % TOTAL_STEPS;
    
    // Aktuelle Position setzen
    lastPosition = position;
    stepper.setCurrentPosition(position);
    savePosition();
    
    Serial.printf("Position gesetzt auf: %d\n", position);
}

void ClockControl::moveToTime(int hour, int minute) {
    // Validierung der Eingabeparameter
    if (hour < 1 || hour > 12 || minute < 0 || minute > 59) {
        Serial.printf("Ungültige Zeit für moveToTime: %02d:%02d\n", hour, minute);
        return;
    }
    
    // Statische Variable, die anzeigt, ob wir uns im 12-Uhr-Übergang befinden
    static bool inTransition = false;
    
    // Wenn wir bereits in einem Übergang sind, nicht unterbrechen
    if (inTransition) {
        Serial.println("Übergang bereits im Gange, ignoriere neuen moveToTime Befehl");
        return;
    }
    
    int targetSteps = calculateStepsFromTime(hour, minute);
    int currentPosition = getCurrentPosition();
    
    // Debug-Ausgabe
    Serial.printf("moveToTime: Aktuelle Position: %d, Zielposition: %d, TOTAL_STEPS: %d\n", 
                 currentPosition, targetSteps, TOTAL_STEPS);
    
    // Berechne den kürzesten Weg
    int distanceForward = (targetSteps >= currentPosition) ? 
                          (targetSteps - currentPosition) : 
                          (TOTAL_STEPS - currentPosition + targetSteps);
                          
    int distanceBackward = (currentPosition >= targetSteps) ? 
                           (currentPosition - targetSteps) : 
                           (currentPosition + TOTAL_STEPS - targetSteps);
    
    // Optimierte Sonderbehandlung für Übergänge bei 12 Uhr
    // Genaue Definitionen für bessere Lesbarkeit
    bool isTarget12OClock = (targetSteps == 0 || targetSteps == TOTAL_STEPS);
    bool isCurrent12OClock = (currentPosition == 0 || currentPosition == TOTAL_STEPS);
    bool isNear12OClock = (currentPosition > TOTAL_STEPS - 1000 || currentPosition < 1000);
    bool isMovingForward = distanceForward <= distanceBackward;
    
    // Prüfen, ob der kürzeste Weg über den 12-Uhr-Punkt geht (0 oder TOTAL_STEPS)
    bool crossesMidnight = false;
    
    // FALL 1: Wir stehen bei 1 Uhr (Position ~40000) und wollen zu 11 Uhr (Position ~450000)
    // Der kürzeste Weg ist rückwärts und geht über 12 Uhr/0 Uhr
    if (targetSteps > currentPosition && !isMovingForward) {
        crossesMidnight = true;
        Serial.println("Fall 1: Übergang über 12 Uhr (rückwärts von früh zu spät)");
    }
    // FALL 2: Wir stehen bei 11 Uhr (Position ~450000) und wollen zu 1 Uhr (Position ~40000)
    // Der kürzeste Weg ist vorwärts und geht über 12 Uhr/0 Uhr
    else if (targetSteps < currentPosition && isMovingForward) {
        crossesMidnight = true;
        Serial.println("Fall 2: Übergang über 12 Uhr (vorwärts von spät zu früh)");
    }
    
    inTransition = true; // Markiere Beginn des Übergangs
    
    // Direkter Übergang bei 12 Uhr (optimiert)
    if ((isTarget12OClock || isCurrent12OClock) && isNear12OClock) {
        if (targetSteps == 0 && currentPosition > TOTAL_STEPS - 1000) {
            // Wir sind nahe an TOTAL_STEPS und wollen zu 0 - direkter Übergang
            // Wichtig: Erst auf TOTAL_STEPS fahren, dann auf 0 setzen
            Serial.println("Optimierter 12-Uhr-Übergang: Fahre genau zu TOTAL_STEPS, dann Sprung auf 0");
            
            // Wenn wir nicht exakt auf TOTAL_STEPS sind, dorthin fahren
            if (currentPosition != TOTAL_STEPS) {
                stepper.moveTo(TOTAL_STEPS);  // Fahre genau zu TOTAL_STEPS (12:00)
                // Warte auf Erreichen der Position
                while (stepper.isRunning()) {
                    stepper.run();
                    yield();
                }
            }
            
            // Jetzt auf 0 umstellen und zu 0 fahren
            stepper.setCurrentPosition(0);
            stepper.moveTo(0);
        } 
        else if (targetSteps == TOTAL_STEPS && currentPosition < 1000) {
            // Wir sind nahe an 0 und wollen zu TOTAL_STEPS - direkter Übergang
            // Wichtig: Erst auf 0 fahren, dann auf TOTAL_STEPS setzen
            Serial.println("Optimierter 12-Uhr-Übergang: Fahre genau zu 0, dann Sprung auf TOTAL_STEPS");
            
            // Wenn wir nicht exakt auf 0 sind, dorthin fahren
            if (currentPosition != 0) {
                stepper.moveTo(0);  // Fahre genau zu 0 (00:00)
                // Warte auf Erreichen der Position
                while (stepper.isRunning()) {
                    stepper.run();
                    yield();
                }
            }
            
            // Jetzt auf TOTAL_STEPS umstellen und zu TOTAL_STEPS fahren
            stepper.setCurrentPosition(TOTAL_STEPS);
            stepper.moveTo(TOTAL_STEPS);
        }
        else {
            // Normale Bewegung, wenn es sich nicht um einen optimierbaren 12-Uhr-Übergang handelt
            stepper.moveTo(targetSteps);
        }
    }
    // Spezialbehandlung für längere Übergänge über 12 Uhr
    else if (crossesMidnight) {
        // Bei Übergang über 12 Uhr wählen wir einen Zwischenschritt
        // Wir bewegen uns erst zur 12-Uhr-Position (0 oder TOTAL_STEPS)
        if (currentPosition < TOTAL_STEPS / 2) {
            // Wenn wir näher an 0 sind, fahren wir zu 0
            Serial.println("Übergang über 12 Uhr: Fahre zuerst exakt zu Position 0");
            // Stellen sicher, dass wir korrekt positioniert sind
            if (currentPosition != 0) {
                stepper.moveTo(0);
                
                // Nach Erreichen von Position 0, zur Zielposition fahren
                while (stepper.isRunning()) {
                    stepper.run();
                    yield(); // ESP8266-spezifisch: Gibt Zeit für WiFi-Stack
                }
            }
            
            // Position auf TOTAL_STEPS setzen, da 0 und TOTAL_STEPS physisch identisch sind
            Serial.println("Erreichte Position 0: Setze logische Position auf TOTAL_STEPS");
            stepper.setCurrentPosition(TOTAL_STEPS);
            
            // Jetzt zur eigentlichen Zielposition fahren
            Serial.printf("Übergang abgeschlossen: Fahre nun zu Zielposition %d\n", targetSteps);
            stepper.moveTo(targetSteps);
        } else {
            // Wenn wir näher an TOTAL_STEPS sind, fahren wir zu TOTAL_STEPS
            Serial.println("Übergang über 12 Uhr: Fahre zuerst exakt zu Position TOTAL_STEPS");
            // Stellen sicher, dass wir korrekt positioniert sind
            if (currentPosition != TOTAL_STEPS) {
                stepper.moveTo(TOTAL_STEPS);
                
                // Nach Erreichen von TOTAL_STEPS, zur Zielposition fahren
                while (stepper.isRunning()) {
                    stepper.run();
                    yield(); // ESP8266-spezifisch: Gibt Zeit für WiFi-Stack
                }
            }
            
            // Position auf 0 setzen, da 0 und TOTAL_STEPS physisch identisch sind
            Serial.println("Erreichte Position TOTAL_STEPS: Setze logische Position auf 0");
            stepper.setCurrentPosition(0);
            
            // Jetzt zur eigentlichen Zielposition fahren
            Serial.printf("Übergang abgeschlossen: Fahre nun zu Zielposition %d\n", targetSteps);
            stepper.moveTo(targetSteps);
        }
    } 
    else {
        // Normale Bewegung für alle anderen Fälle - kürzesten Weg wählen
        if (isMovingForward) {
            // Vorwärts ist kürzer oder gleich
            stepper.moveTo(targetSteps);
        } else {
            // Rückwärts ist kürzer
            if (targetSteps > currentPosition) {
                // Muss über 0 springen - negative Position, damit AccelStepper rückwärts fährt
                stepper.moveTo(targetSteps - TOTAL_STEPS);
            } else {
                stepper.moveTo(targetSteps);
            }
        }
    }
    
    // Übergang ist abgeschlossen
    inTransition = false;
    
    Serial.printf("Bewege zu Zeit %02d:%02d (Zielposition: %d)\n", hour, minute, targetSteps);
}

int ClockControl::getCurrentPosition() {
    return stepper.currentPosition();
}

void ClockControl::savePosition() {
    // Mehrfache Sicherung der Position um Datenverlust zu vermeiden
    EEPROM.put(POSITION_ADDR, lastPosition);
    EEPROM.put(POSITION_ADDR + 4, lastPosition); // Backup an anderer Stelle
    bool success = EEPROM.commit();
    if (success) {
        Serial.printf("Position %d gespeichert\n", lastPosition);
    } else {
        Serial.printf("Fehler beim Speichern der Position %d\n", lastPosition);
        delay(100);
        EEPROM.commit(); // Zweiter Versuch
    }
}

int ClockControl::loadPosition() {
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

String ClockControl::getCurrentTimeStr() {
    int totalMinutes = (stepper.currentPosition() * 720) / TOTAL_STEPS;
    int currentHour = (totalMinutes / 60) % 12;
    int currentMinute = totalMinutes % 60;
    
    // Sekunden berechnen (Position * 720 * 60) / TOTAL_STEPS
    long long totalSeconds = ((long long)stepper.currentPosition() * 720LL * 60LL) / TOTAL_STEPS;
    int currentSecond = abs(totalSeconds % 60);
    
    if (currentHour == 0) currentHour = 12;
    String timeStr = String(currentHour < 10 ? "0" : "") + String(currentHour) + ":" +
                    String(currentMinute < 10 ? "0" : "") + String(currentMinute) + ":" +
                    String(currentSecond < 10 ? "0" : "") + String(currentSecond);
    return timeStr;
}

String ClockControl::getNTPTimeStr() {
    time_t now = berlinTZ.now();
    struct tm *timeinfo = localtime(&now);
    int hour = timeinfo->tm_hour % 12;
    int minute = timeinfo->tm_min;
    int second = timeinfo->tm_sec;
    
    if (hour == 0) hour = 12;
    return String(hour < 10 ? "0" : "") + String(hour) + ":" +
           String(minute < 10 ? "0" : "") + String(minute) + ":" +
           String(second < 10 ? "0" : "") + String(second);
}

int ClockControl::calculateStepsFromTime(int hour, int minute) {
    int totalMinutes = hour * 60 + minute;
    int position = (totalMinutes * TOTAL_STEPS) / 720;
    
    // Sorge dafür, dass die Position immer im gültigen Bereich liegt
    if (position >= TOTAL_STEPS) {
        position = position % TOTAL_STEPS;
    }
    
    return position;
}

time_t ClockControl::getNTPTime() {
    return berlinTZ.now();
}

void ClockControl::updateStoredTime() {
    time_t now = berlinTZ.now();
    struct tm *timeinfo = localtime(&now);
    lastHour = timeinfo->tm_hour % 12;
    lastMinute = timeinfo->tm_min;
    if (lastHour == 0) lastHour = 12;
    Serial.printf("Gespeicherte Zeit aktualisiert: %02d:%02d\n", lastHour, lastMinute);
}

void ClockControl::adjustPosition(int adjustment) {
    int currentPosition = getCurrentPosition();
    
    // Wenn adjustment positiv ist (z.B. +30 Sekunden), müssen wir die Position reduzieren
    // Wenn adjustment negativ ist (z.B. -30 Sekunden), müssen wir die Position erhöhen
    int newPosition = currentPosition - (adjustment * STEPS_PER_HALF_MINUTE / 30);
    
    // Position setzen
    setPosition(newPosition);
    
    // Zur aktuellen NTP-Zeit synchronisieren
    syncToNTPTime();
}

bool ClockControl::syncToNTPTime() {
    // Aktuelle NTP-Zeit abrufen mit Timeout
    unsigned long startAttempt = millis();
    bool timeUpdated = false;
    
    while (millis() - startAttempt < 1000) { // 1 Sekunde Timeout
        if (berlinTZ.now() > 1600000000) { // Prüfe auf gültige Zeit (nach Sept 2020)
            timeUpdated = true;
            break;
        }
        delay(100);
    }
    
    if (!timeUpdated) {
        return false;
    }
    
    time_t now = berlinTZ.now();
    struct tm *timeinfo = localtime(&now);
    
    // Robustere Zeitkonvertierung
    int currentHour = timeinfo->tm_hour;
    if (currentHour == 0 || currentHour == 12) {
        currentHour = 12;
    } else if (currentHour > 12) {
        currentHour = currentHour - 12;
    }
    
    int currentMinute = timeinfo->tm_min;
    
    // Zur aktuellen Zeit fahren
    Serial.printf("Synchronisiere mit NTP-Zeit: %02d:%02d\n", currentHour, currentMinute);
    moveToTime(currentHour, currentMinute);
    
    return true;
} 
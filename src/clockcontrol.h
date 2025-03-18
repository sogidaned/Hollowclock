#pragma once

#include <AccelStepper.h>
#include <ezTime.h>
#include <EEPROM.h>

class ClockControl {
public:
    ClockControl();
    void begin();
    void update();
    void setPosition(int position);
    void moveToTime(int hour, int minute);
    void adjustPosition(int adjustment);
    bool syncToNTPTime();  // Rückgabewert auf bool geändert
    int getCurrentPosition();
    void savePosition();
    int loadPosition();
    String getCurrentTimeStr();
    String getNTPTimeStr();
    time_t getNTPTime();
    int calculateStepsFromTime(int hour, int minute);

private:
    AccelStepper stepper;
    Timezone berlinTZ;
    int lastPosition;
    long lastPositionLong;
    unsigned long lastTimeCheck;  // Timer für Zeitüberprüfung
    int lastHour;                 // Gespeicherte Stunde
    int lastMinute;               // Gespeicherte Minute
    void initStepper();
    void updateStoredTime();      // Neue Funktion zum Aktualisieren der gespeicherten Zeit
}; 
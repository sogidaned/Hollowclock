# Hollowclock Standalone

A standalone version of the Hollowclock that works without Home Assistant.

## Installation

### Direct Installation (recommended)
The easiest way to install the firmware:

1. Visit [https://hollowclock.netlify.app](https://hollowclock.netlify.app) 
2. Connect your Wemos D1 Mini to the USB port
3. Click "Install" and follow the instructions
4. After installation, the clock is ready to use

*Note: This installer uses WebUSB and requires Chrome, Edge, or Opera browser. Firefox is not supported.*

## First Setup

1. After the first start, the clock creates a WiFi access point named "Hollowclock-XXXXXX"
2. Connect to this network (password: 12345678)
3. A configuration portal will open automatically (if not, open http://192.168.4.1)
4. Select your WiFi network and enter the password
5. The clock will restart and connect to your WiFi

## Hardware Connections

Connect the ULN2003 stepper motor driver as follows:
- IN1 -> D8
- IN2 -> D7
- IN3 -> D6
- IN4 -> D5

![Wiring Diagram for Hollowclock](https://raw.githubusercontent.com/sogidaned/Hollowclock/main/docs/wiring_diagram.png)
*Wiring diagram: Wemos D1 Mini with ULN2003 driver and 28BYJ-48 stepper motor*

## Usage

1. After startup, the clock automatically connects to your WiFi
2. Open http://hollowclock.local in a web browser (or use the IP address)
3. Through the web interface, you can:
   - Set the time manually
   - Make fine adjustments (+30 seconds)
   - Reset WiFi settings
   - Manage MQTT configuration (optional)
   - Perform firmware updates

## Firmware Updates

To update your clock:

1. Go to http://hollowclock.local/firmware in your browser
2. Download the latest firmware.bin and littlefs.bin files from [GitHub Releases](https://github.com/sogidaned/Hollowclock/releases/latest)
3. First upload firmware.bin using the "Firmware Update" form
4. After the device restarts, upload littlefs.bin using the "Filesystem Update" form
5. The device will restart again with the new version

> **Tip:** If the /firmware page is not accessible after an update, you can use http://hollowclock.local/update as a fallback. This built-in page provides basic update functionality and works even if the filesystem is damaged.

## MQTT Integration (Optional)

The clock supports optional MQTT integration with Home Assistant:

1. Open the clock's web interface
2. Go to MQTT configuration
3. Enable MQTT using the switch
4. Enter the MQTT server data:
   - Broker address (e.g., homeassistant.local)
   - Port (default: 1883)
   - Client ID (unique name)
   - Topic (unique topic prefix)
   - Optional: Username and password
5. Click "Save MQTT Settings"

### Home Assistant Integration

The clock sends the following information to Home Assistant:
- Current time in HH:MM format
- Current position of the stepper motor

You can control the clock via Home Assistant with the following MQTT command:
```yaml
service: mqtt.publish
data:
  topic: hollowclock/set
  payload: |
    {"set_time": {"hour": 3, "minute": 30}}
```

#### Setting up Home Assistant Automation

For better integration, you can use an `input_datetime` helper in Home Assistant to control the clock. Create the following:

1. Add an `input_datetime` helper named `hollowclock` in Home Assistant (Configuration → Helpers)
2. Create an automation that updates the clock whenever the input_datetime value changes:

```yaml
alias: Hollowclock set
description: ""
triggers:
  - trigger: state
    entity_id:
      - input_datetime.hollowclock
    for:
      hours: 0
      minutes: 0
      seconds: 5
    from: null
    to: null
conditions: []
actions:
  - action: mqtt.publish
    metadata: {}
    data:
      evaluate_payload: false
      qos: 0
      retain: false
      topic: Hollowclock/set
      payload: >-
        {"set_time": {"hour": {{
        states('input_datetime.hollowclock').split(':')[0] | int }}, "minute":
        {{ states('input_datetime.hollowclock').split(':')[1] | int }}}}
mode: single
```

This automation will send the time from the input_datetime helper to the clock 5 seconds after any change.

## Reset WiFi Settings

If you want to change the WiFi settings:
1. Open the clock's web interface
2. Click "Reset WiFi Settings"
3. The clock will restart and create an access point again
4. Follow the steps under "First Setup"

## Troubleshooting

If the clock is not working properly:

1. Make sure the hardware connections are correct
2. Check that the power supply is sufficient (5V/1A recommended)
3. If the time is wrong, check your internet connection
4. For WiFi problems, reset the WiFi settings as described above
5. If the web interface looks broken after a firmware update, update the file system as described in the "Firmware Update" section

## Source Code

The complete source code for this project is available on GitHub: [sogidaned/hollowclock](https://github.com/sogidaned/hollowclock)

---

# Deutsche Version / German Version

# Hollowclock Standalone

Eine eigenständige Version der Hollowclock, die ohne Home Assistant funktioniert.

## Installation

### Direkte Installation (empfohlen)
Der einfachste Weg, die Firmware zu installieren:

1. Besuche [https://hollowclock.netlify.app](https://hollowclock.netlify.app) 
2. Verbinde deinen Wemos D1 Mini mit dem USB-Anschluss
3. Klicke auf "Installieren" und folge den Anweisungen
4. Nach der Installation ist die Uhr einsatzbereit

*Hinweis: Dieser Installer nutzt WebUSB und benötigt Chrome, Edge oder Opera. Firefox wird nicht unterstützt.*

## Erste Einrichtung

1. Nach dem ersten Start erzeugt die Uhr einen WLAN-Zugangspunkt mit dem Namen "Hollowclock-XXXXXX"
2. Verbinde dich mit diesem Netzwerk (Passwort: 12345678)
3. Ein Konfigurationsportal öffnet sich automatisch (falls nicht, öffne http://192.168.4.1)
4. Wähle dein WLAN-Netzwerk aus und gib das Passwort ein
5. Die Uhr startet neu und verbindet sich mit deinem WLAN

## Hardware-Anschlüsse

Verbinde den ULN2003 Schrittmotor-Treiber wie folgt:
- IN1 -> D8
- IN2 -> D7
- IN3 -> D6
- IN4 -> D5

![Verkabelungsplan für Hollowclock](https://raw.githubusercontent.com/sogidaned/Hollowclock/main/docs/wiring_diagram.png)
*Verkabelungsplan: Wemos D1 Mini mit ULN2003-Treiber und 28BYJ-48 Schrittmotor*

## Benutzung

1. Nach dem Start verbindet sich die Uhr automatisch mit deinem WLAN
2. Öffne http://hollowclock.local in einem Webbrowser (oder verwende die IP-Adresse)
3. Über die Weboberfläche kannst du:
   - Die Zeit manuell einstellen
   - Feineinstellungen vornehmen (+30 Sekunden)
   - WLAN-Einstellungen zurücksetzen
   - MQTT-Konfiguration verwalten (optional)
   - Firmware-Updates durchführen

## Firmware-Updates

So führst du ein Update durch:

1. Gehe zu http://hollowclock.local/firmware in deinem Browser
2. Lade die neuesten firmware.bin und littlefs.bin Dateien von [GitHub Releases](https://github.com/sogidaned/Hollowclock/releases/latest) herunter
3. Lade zuerst firmware.bin über das "Firmware Update" Formular hoch
4. Nach dem Neustart des Geräts lade littlefs.bin über das "Filesystem Update" Formular hoch
5. Das Gerät startet erneut mit der neuen Version

> **Tipp:** Wenn die /firmware Seite nach einem Update nicht erreichbar ist, kannst du http://hollowclock.local/update als Ausweichlösung verwenden. Diese eingebaute Seite bietet grundlegende Update-Funktionen und funktioniert auch, wenn das Dateisystem beschädigt ist.

## MQTT-Integration (Optional)

Die Uhr unterstützt optional die MQTT-Integration mit Home Assistant:

1. Öffne die Weboberfläche der Uhr
2. Gehe zur MQTT-Konfiguration
3. Aktiviere MQTT mit dem Schalter
4. Gib die MQTT-Server-Daten ein:
   - Broker-Adresse (z.B. homeassistant.local)
   - Port (Standard: 1883)
   - Client-ID (eindeutiger Name)
   - Topic (eindeutiges Topic-Präfix)
   - Optional: Benutzername und Passwort
5. Klicke auf "MQTT-Einstellungen speichern"

### Home Assistant Integration

Die Uhr sendet folgende Informationen an Home Assistant:
- Aktuelle Zeit im HH:MM Format
- Aktuelle Position des Schrittmotors

Du kannst die Uhr über Home Assistant mit folgendem MQTT-Befehl steuern:
```yaml
service: mqtt.publish
data:
  topic: hollowclock/set
  payload: |
    {"set_time": {"hour": 3, "minute": 30}}
```

#### Einrichten einer Home Assistant Automation

Für eine bessere Integration kannst du einen `input_datetime`-Helfer in Home Assistant verwenden, um die Uhr zu steuern. Erstelle Folgendes:

1. Füge einen `input_datetime`-Helfer namens `hollowclock` in Home Assistant hinzu (Konfiguration → Hilfsmittel)
2. Erstelle eine Automation, die die Uhr aktualisiert, wenn sich der Wert des input_datetime ändert:

```yaml
alias: Hollowclock einstellen
description: ""
triggers:
  - trigger: state
    entity_id:
      - input_datetime.hollowclock
    for:
      hours: 0
      minutes: 0
      seconds: 5
    from: null
    to: null
conditions: []
actions:
  - action: mqtt.publish
    metadata: {}
    data:
      evaluate_payload: false
      qos: 0
      retain: false
      topic: Hollowclock/set
      payload: >-
        {"set_time": {"hour": {{
        states('input_datetime.hollowclock').split(':')[0] | int }}, "minute":
        {{ states('input_datetime.hollowclock').split(':')[1] | int }}}}
mode: single
```

Diese Automation sendet die Zeit vom input_datetime-Helfer an die Uhr, 5 Sekunden nach jeder Änderung.

## WLAN-Einstellungen zurücksetzen

Wenn du die WLAN-Einstellungen ändern möchtest:
1. Öffne die Weboberfläche der Uhr
2. Klicke auf "WLAN-Einstellungen zurücksetzen"
3. Die Uhr startet neu und erzeugt erneut einen Zugangspunkt
4. Folge den Schritten unter "Erste Einrichtung"

## Fehlerbehebung

Wenn die Uhr nicht richtig funktioniert:

1. Stelle sicher, dass die Hardware-Anschlüsse korrekt sind
2. Prüfe, ob die Stromversorgung ausreichend ist (5V/1A empfohlen)
3. Wenn die Zeit falsch ist, überprüfe deine Internetverbindung
4. Bei WLAN-Problemen setze die WLAN-Einstellungen wie oben beschrieben zurück
5. Wenn die Weboberfläche nach einem Firmware-Update fehlerhaft aussieht, aktualisiere das Dateisystem wie im Abschnitt "Firmware-Update" beschrieben

## Quellcode

Der vollständige Quellcode für dieses Projekt ist auf GitHub verfügbar: [sogidaned/hollowclock](https://github.com/sogidaned/hollowclock)



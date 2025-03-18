# Hollowclock Web Installer

Dieses Verzeichnis enthält die Dateien für den ESP Web Tools-basierten Installer der Hollowclock Firmware.

## Voraussetzungen

Um die Installer-Webseite zu nutzen, benötigen Sie:

1. Einen modernen Webbrowser mit WebUSB-Unterstützung:
   - Google Chrome
   - Microsoft Edge
   - Opera
   (Firefox unterstützt WebUSB derzeit nicht)

2. Einen Wemos D1 Mini oder kompatiblen ESP8266-Mikrocontroller

## Dateien in diesem Verzeichnis

- `install.html` - Die Installer-Webseite
- `manifest.json` - Manifestdatei für ESP Web Tools mit Informationen zur Firmware
- `firmware.bin` - Die kompilierte Firmware (ersetzen Sie diese mit Ihrer eigenen)
- `littlefs.bin` - Das Dateisystem-Image (ersetzen Sie dieses mit Ihrem eigenen)

## Anpassung für Ihr eigenes Projekt

1. Platzieren Sie Ihre eigene `firmware.bin` und `littlefs.bin` in diesem Verzeichnis
2. Passen Sie ggf. die `manifest.json` an Ihre spezifischen Bedürfnisse an
3. Ändern Sie die `install.html` nach Wunsch, um Ihr eigenes Branding einzufügen

## Hosting des Installers

Sie können den Installer auf verschiedene Arten hosten:

### Option 1: GitHub Pages

1. Laden Sie dieses Repository auf GitHub hoch
2. Aktivieren Sie GitHub Pages für das Repository
3. Die Installer-Webseite ist dann unter `https://IHR_BENUTZERNAME.github.io/IHR_REPOSITORY/web/install.html` verfügbar

### Option 2: Beliebiger Webserver

Sie können die Dateien auf jedem Webserver hosten, der statische Dateien bereitstellen kann.

## Lokales Testen

Für lokales Testen können Sie einen einfachen Webserver verwenden:

```bash
# Mit Python 3
cd web
python -m http.server 8000

# Mit Node.js
npx serve web
```

Öffnen Sie dann http://localhost:8000 in Ihrem Browser.

## Wichtige Hinweise

- WebUSB funktioniert nur über HTTPS oder localhost
- Bei Problemen mit der Erkennung des ESP8266:
  - Stellen Sie sicher, dass Sie ein hochwertiges USB-Kabel verwenden
  - Versuchen Sie, den ESP8266 in den Bootloader-Modus zu versetzen (halten Sie den BOOT/FLASH-Knopf gedrückt, während Sie ihn anschließen)
  - Aktualisieren Sie Ihren Browser auf die neueste Version 
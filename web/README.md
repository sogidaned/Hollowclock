# Hollowclock Installationsseite

Diese Webseite ermöglicht die einfache Installation der Hollowclock-Firmware über den Browser.

## Bereitstellung auf Netlify

1. Erstelle ein Repository auf GitHub und lade alle Dateien hoch
2. Melde dich bei [Netlify](https://www.netlify.com/) an (kostenlos)
3. Klicke auf "Add new site" → "Import an existing project"
4. Wähle dein GitHub-Repository aus
5. Bei den Einstellungen:
   - **Publish directory**: `web`
   - Keine Build-Befehle erforderlich
6. Klicke auf "Deploy site"
7. Nach dem Deployment ist deine Installer-Webseite unter der Netlify-Domain verfügbar

Die Installer-Seite kann dann unter `https://deine-domain.netlify.app` aufgerufen werden.

## Anpassung der Domain (optional)

1. Unter "Site settings" kannst du die automatisch generierte Netlify-Domain ändern
2. Klicke auf "Change site name" und wähle einen benutzerfreundlicheren Namen
3. Die Webseite ist dann unter `https://dein-name.netlify.app` erreichbar

## Firmware-Updates

Wenn du neue Versionen der Firmware oder des Dateisystems veröffentlichst:

1. Erstelle neue `firmware.bin` und `littlefs.bin` Dateien
2. Ersetze die alten Dateien im `web`-Verzeichnis
3. Lade die Änderungen auf GitHub hoch
4. Netlify wird automatisch ein neues Deployment erstellen

## Dateien in diesem Verzeichnis

- `install.html`: Die Hauptseite für die Installation
- `manifest.json`: Konfigurationsdatei für ESP Web Tools
- `firmware.bin`: Die aktuelle Firmware für den ESP8266
- `littlefs.bin`: Das Dateisystem mit der Weboberfläche
- `netlify.toml`: Konfigurationsdatei für Netlify
- `_redirects`: Umleitungsregeln für Netlify

## ESP Web Tools

Diese Seite nutzt [ESP Web Tools](https://esphome.github.io/esp-web-tools/), eine Bibliothek, die das Flashen von ESP-Geräten direkt über den Browser ermöglicht.

Unterstützte Browser:
- Chrome
- Edge
- Opera

Firefox unterstützt WebUSB derzeit nicht und kann daher nicht verwendet werden.

## Voraussetzungen

Um die Installer-Webseite zu nutzen, benötigen Sie:

1. Einen modernen Webbrowser mit WebUSB-Unterstützung:
   - Google Chrome
   - Microsoft Edge
   - Opera
   (Firefox unterstützt WebUSB derzeit nicht)

2. Einen Wemos D1 Mini oder kompatiblen ESP8266-Mikrocontroller

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
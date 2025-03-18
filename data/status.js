import API from '/api.js';
import LanguageManager from '/language.js';

class StatusManager {
    constructor() {
        this.updateInterval = null;
        this.isUpdating = false;
    }

    // WLAN-Signal in benutzerfreundliches Format umwandeln
    formatWifiSignal(rssi) {
        // dBm in Prozent umrechnen (gebräuchliche Näherung)
        // Typische Werte: -50 dBm (sehr gut) bis -100 dBm (sehr schwach)
        let percent = 0;
        if (rssi >= -50) {
            percent = 100;
        } else if (rssi <= -100) {
            percent = 0;
        } else {
            percent = 2 * (rssi + 100);
        }
        
        // Signalstärke-Text
        let qualityKey = "";
        let signalBars = "";
        
        if (percent >= 80) {
            qualityKey = "very_good";
            signalBars = "●●●●●";
        } else if (percent >= 60) {
            qualityKey = "good";
            signalBars = "●●●●○";
        } else if (percent >= 40) {
            qualityKey = "medium";
            signalBars = "●●●○○";
        } else if (percent >= 20) {
            qualityKey = "weak";
            signalBars = "●●○○○";
        } else {
            qualityKey = "very_weak";
            signalBars = "●○○○○";
        }
        
        // Übersetzte Qualitätsbezeichnung
        const quality = LanguageManager.translate(qualityKey);
        
        return {
            rssi: rssi,
            percent: Math.round(percent),
            quality: quality,
            signalBars: signalBars
        };
    }

    // Status-Elemente aktualisieren
    async updateStatus() {
        if (this.isUpdating) return;
        this.isUpdating = true;

        try {
            const data = await API.getStatus();
            console.log('Status-Daten empfangen:', data);

            // System-Informationen aktualisieren
            document.getElementById('version').textContent = data.version;
            document.getElementById('ip-address').textContent = data.ip;
            
            // WLAN-Signal formatieren und anzeigen
            const wifiSignal = this.formatWifiSignal(data.rssi);
            document.getElementById('wifi-signal').innerHTML = 
                `<span class="signal-quality">${wifiSignal.quality}</span> 
                 <span class="signal-bars">${wifiSignal.signalBars}</span>
                 <span class="signal-details">(${wifiSignal.percent}% / ${wifiSignal.rssi} dBm)</span>`;
            
            // Uptime formatieren
            const uptime = data.uptime;
            const hours = Math.floor(uptime / 3600);
            const minutes = Math.floor((uptime % 3600) / 60);
            const seconds = uptime % 60;
            document.getElementById('uptime').textContent = 
                `${hours}h ${minutes}m ${seconds}s`;

            // Uhr-Informationen aktualisieren
            document.getElementById('current-position').textContent = data.currentTime;
            document.getElementById('ntp-time').textContent = data.ntpTime;
            document.getElementById('step-position').textContent = 
                `${data.stepPosition} (${data.stepPercent}%)`;

            console.log('Status-Anzeige aktualisiert');
        } catch (error) {
            console.error('Fehler beim Status-Update:', error);
        } finally {
            this.isUpdating = false;
        }
    }

    // Status-Updates starten
    start() {
        console.log('Starte Status-Updates...');
        // Ersten Update sofort durchführen
        this.updateStatus()
            .then(() => {
                console.log('Erster Status-Update erfolgreich');
                // Regelmäßige Updates starten
                this.updateInterval = setInterval(() => this.updateStatus(), 1000);
                console.log('Regelmäßige Updates aktiviert');
            })
            .catch(error => {
                console.error('Fehler beim ersten Status-Update:', error);
            });
            
        // Event-Listener für Sprachwechsel
        document.addEventListener('languageChanged', () => {
            this.updateStatus(); // Status mit neuer Sprache aktualisieren
        });
    }

    // Status-Updates stoppen
    stop() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
            console.log('Status-Updates gestoppt');
        }
    }
}

export default new StatusManager(); 
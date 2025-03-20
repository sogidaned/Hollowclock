// API-Endpunkte
const API = {
    // Basis-URL für API-Aufrufe
    baseUrl: window.location.hostname === 'localhost' ? 'http://hollowclock.local' : '',

    // Status abrufen
    async getStatus() {
        try {
            const response = await fetch(`${this.baseUrl}/api/status`);
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Abrufen des Status:', error);
            throw error;
        }
    },

    // Zeit setzen
    async setTime(hour, minute) {
        try {
            const response = await fetch('/api/setTime', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `hour=${hour}&minute=${minute}`
            });
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Setzen der Zeit:', error);
            throw error;
        }
    },

    // Zeit anpassen
    async adjustTime(adjustment) {
        try {
            const response = await fetch('/api/adjustTime', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `adjustment=${adjustment}`
            });
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler bei der Zeitanpassung:', error);
            throw error;
        }
    },

    // WLAN zurücksetzen
    async resetWiFi() {
        try {
            const response = await fetch('/api/reset_wifi', { method: 'POST' });
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim WLAN-Reset:', error);
            throw error;
        }
    },

    // Gerät neustarten
    async restartDevice() {
        try {
            const response = await fetch('/api/restart', { method: 'POST' });
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Neustart:', error);
            throw error;
        }
    },

    // MQTT-Konfiguration abrufen
    async getMqttConfig() {
        try {
            const response = await fetch('/api/mqtt/config');
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Abrufen der MQTT-Konfiguration:', error);
            throw error;
        }
    },

    // MQTT-Konfiguration speichern
    async setMqttConfig(config) {
        try {
            const response = await fetch('/api/mqtt/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(config)
            });
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Speichern der MQTT-Konfiguration:', error);
            throw error;
        }
    },

    // MQTT-Status abrufen
    async getMqttStatus() {
        try {
            const response = await fetch('/api/mqtt/status');
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Abrufen des MQTT-Status:', error);
            throw error;
        }
    },

    // MQTT-Konfiguration zurücksetzen
    async resetMqttConfig() {
        try {
            const response = await fetch('/api/mqtt/reset', {
                method: 'POST'
            });
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Zurücksetzen der MQTT-Konfiguration:', error);
            throw error;
        }
    },
    
    // Alle Einstellungen zurücksetzen
    async resetAllSettings() {
        try {
            const response = await fetch('/api/reset_all', {
                method: 'POST'
            });
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error('Fehler beim Zurücksetzen aller Einstellungen:', error);
            throw error;
        }
    },

    async saveMqttConfig(config) {
        const response = await fetch('/api/mqtt/config', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(config)
        });
        return await response.json();
    },

    // Update-Status prüfen
    async checkUpdate() {
        console.group("🔍 Update-Prüfung");
        console.log("Update-Prüfung durch manuelles Update ersetzt");
        
        // Einfach die Firmware-Version zurückgeben
        const processedData = {
            currentVersion: FW_VERSION || "1.0.0",
            latestVersion: "",
            firmwareUrl: "",
            filesystemUrl: "",
            error: null,
            isFallback: false
        };
        
        console.log("✅ Aktuelle Version:", processedData.currentVersion);
        console.groupEnd();
        return processedData;
    },

    // Update starten
    async startUpdate(type, url) {
        console.log(`Diese Methode wird nicht mehr verwendet. Bitte verwenden Sie das manuelle Update.`);
        return { status: "error", error: "Diese Funktion wurde entfernt. Bitte verwenden Sie das manuelle Update über die Weboberfläche." };
    }
};

export default API; 
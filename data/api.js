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
        console.log("API-Anfrage: Update-Prüfung gestartet");
        
        // Timeout nach 10 Sekunden für die Anfrage
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), 15000); // 15 Sekunden Timeout
        
        try {
            // Messung der Zeit für die Anfrage
            const startTime = performance.now();
            
            // Anfrage an die API
            const requestUrl = `${this.baseUrl}/api/check-update`;
            console.log("🌐 Request URL:", requestUrl, "| Host:", window.location.hostname);
            
            const response = await fetch(requestUrl, { 
                signal: controller.signal,
                headers: {
                    'Cache-Control': 'no-cache',
                    'Pragma': 'no-cache'
                }
            });
            const endTime = performance.now();
            console.log(`⏱️ Fetch-Dauer: ${(endTime - startTime).toFixed(2)}ms | Status: ${response.status}`);
            
            if (!response.ok) {
                console.error(`❌ HTTP-Statusfehler: ${response.status}`);
                throw new Error(`Fehler beim Abrufen der Update-Informationen: ${response.status} ${response.statusText}`);
            }
            
            const data = await response.json();
            console.log("📊 API-Antwort erhalten:", data);
            
            // Prüfen auf erforderliche Felder
            if (!data.currentVersion) {
                console.warn("⚠️ Aktuelle Version fehlt in der API-Antwort");
            }
            
            if (!data.latestVersion) {
                console.warn("⚠️ Neueste Version fehlt in der API-Antwort");
            }
            
            // Fallback-Werte hinzufügen, wenn nötig
            const processedData = {
                currentVersion: data.currentVersion || "1.0.0",
                latestVersion: data.latestVersion || "1.0.0",
                firmwareUrl: data.firmwareUrl || "",
                filesystemUrl: data.filesystemUrl || "",
                error: data.error || null,
                isFallback: false // Standard: keine Fallback-Daten
            };
            
            // Prüfen, ob die URLs vollständige Pfade sind
            if (processedData.firmwareUrl && !processedData.firmwareUrl.startsWith('http')) {
                if (processedData.firmwareUrl.startsWith('/')) {
                    processedData.firmwareUrl = `${this.baseUrl}${processedData.firmwareUrl}`;
                } else {
                    processedData.firmwareUrl = `${this.baseUrl}/${processedData.firmwareUrl}`;
                }
                console.log("🔄 Firmware URL zu absolutem Pfad ergänzt:", processedData.firmwareUrl);
            }
            
            if (processedData.filesystemUrl && !processedData.filesystemUrl.startsWith('http')) {
                if (processedData.filesystemUrl.startsWith('/')) {
                    processedData.filesystemUrl = `${this.baseUrl}${processedData.filesystemUrl}`;
                } else {
                    processedData.filesystemUrl = `${this.baseUrl}/${processedData.filesystemUrl}`;
                }
                console.log("🔄 Filesystem URL zu absolutem Pfad ergänzt:", processedData.filesystemUrl);
            }
            
            console.log("✅ Verarbeitete Daten:", processedData);
            console.groupEnd();
            return processedData;
            
        } catch (error) {
            console.error("❌ Fehler bei der Update-Prüfung:", error.message);
            
            // Fehler zurückgeben, aber keine Fallback-URLs
            const errorData = {
                currentVersion: FW_VERSION || "1.0.0",
                latestVersion: "", 
                firmwareUrl: "",
                filesystemUrl: "",
                error: error.message,
                isFallback: false
            };
            
            console.log("🔄 Fehler bei der Update-Prüfung:", errorData);
            console.groupEnd();
            return errorData;
        } finally {
            clearTimeout(timeoutId);
        }
    },

    // Update starten
    async startUpdate(type, url) {
        console.log(`Starte ${type} Update mit URL: ${url}`);
        console.log(`Update-Zeitstempel: ${new Date().toISOString()}`);
        
        try {
            // Timeout für die Anfrage erhöhen (30 Sekunden)
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 30000);
            
            const startTime = performance.now();
            console.log(`Start Update-Anfrage: ${startTime}ms`);
            
            const response = await fetch('/api/start-update', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                    'Cache-Control': 'no-cache'
                },
                body: `type=${encodeURIComponent(type)}&url=${encodeURIComponent(url)}`,
                signal: controller.signal
            });
            
            const endTime = performance.now();
            console.log(`Ende Update-Anfrage: ${endTime}ms, Dauer: ${endTime - startTime}ms`);
            
            if (!response.ok) {
                console.error(`HTTP error! status: ${response.status}`);
                if (response.status === 500) {
                    console.error("Server-Fehler: Möglicherweise Timeout oder Verbindungsproblem");
                }
                
                const errorText = await response.text();
                throw new Error(`HTTP error! status: ${response.status}, Details: ${errorText}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error(`Fehler beim ${type}-Update:`, error);
            if (error.name === 'AbortError') {
                throw new Error("Update-Anfrage hat das Zeitlimit überschritten (30 Sekunden)");
            }
            throw error;
        }
    }
};

export default API; 
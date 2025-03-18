import StatusManager from '/status.js';
import UI from '/ui.js';
import LanguageManager from '/language.js';

// Haupt-App-Klasse
class App {
    constructor() {
        console.log('Initialisiere App...');
    }

    // App starten
    async init() {
        try {
            // Zuerst Spracheinstellungen laden
            LanguageManager.init();
            
            // UI initialisieren
            await UI.init();
            console.log('UI initialisiert');
            
            // Status-Updates starten
            StatusManager.start();
            console.log('Status-Updates gestartet');
            
            // Event-Listener für Buttons
            this.setupEventListeners();
            
            console.log('App erfolgreich initialisiert');
        } catch (error) {
            console.error('Fehler bei der Initialisierung:', error);
        }
    }
    
    setupEventListeners() {
        // Zeit setzen
        document.getElementById('set-time-btn').addEventListener('click', () => {
            UI.setTime();
        });

        // Zeit anpassen
        document.getElementById('adjust-minus-30-btn').addEventListener('click', () => {
            UI.adjustTime(-30);
        });
        
        document.getElementById('adjust-plus-30-btn').addEventListener('click', () => {
            UI.adjustTime(30);
        });

        // System-Funktionen
        document.getElementById('reset-wifi-btn').addEventListener('click', () => {
            UI.resetWiFi();
        });

        document.getElementById('restart-btn').addEventListener('click', () => {
            UI.restartDevice();
        });

        document.getElementById('update-btn').addEventListener('click', () => {
            UI.startUpdate();
        });
        
        // MQTT-Buttons
        const saveMqttBtn = document.getElementById('save-mqtt-btn');
        if (saveMqttBtn) {
            saveMqttBtn.addEventListener('click', () => {
                UI.saveMqttConfig();
            });
        }
        
        const resetMqttBtn = document.getElementById('reset-mqtt-btn');
        if (resetMqttBtn) {
            resetMqttBtn.addEventListener('click', () => {
                UI.resetMqttConfig();
            });
        }
        
        // Reset-All Button
        const resetAllBtn = document.getElementById('reset-all-btn');
        if (resetAllBtn) {
            resetAllBtn.addEventListener('click', () => {
                UI.resetAllSettings();
            });
        }
    }
}

// App starten
document.addEventListener('DOMContentLoaded', () => {
    const app = new App();
    app.init();
});

export default new App(); 
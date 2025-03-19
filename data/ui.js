import API from '/api.js';
import LanguageManager from '/language.js';

class UI {
    constructor() {
        this.hourWheel = null;
        this.minuteWheel = null;
        this.isInitialized = false;
        this.latestFirmwareUrl = null;
        this.latestFilesystemUrl = null;
    }

    // UI initialisieren
    async init() {
        if (this.isInitialized) return;
        
        console.log('Initialisiere UI...');
        try {
            await this.initTimeWheels();
            this.initMqttConfig();
            this.setupUpdateButton();
            this.isInitialized = true;
            console.log('UI erfolgreich initialisiert');
        } catch (error) {
            console.error('Fehler bei der UI-Initialisierung:', error);
            throw error;
        }
    }

    // Time Wheels initialisieren
    async initTimeWheels() {
        console.log('Initialisiere Time Wheels');
        
        // Time Wheels finden
        this.hourWheel = document.getElementById('hour-wheel');
        this.minuteWheel = document.getElementById('minute-wheel');
        
        if (!this.hourWheel || !this.minuteWheel) {
            throw new Error('Time Wheels nicht gefunden!');
        }

        // Time Wheels mit Werten füllen
        this.hourWheel.innerHTML = this.generateTimeItems(1, 12, 'hour');
        this.minuteWheel.innerHTML = this.generateTimeItems(0, 59, 'minute');

        // Event-Listener hinzufügen
        [this.hourWheel, this.minuteWheel].forEach(wheel => {
            this.setupWheelEvents(wheel);
        });

        // Initiale Position setzen
        this.scrollToTime(6, 30);
    }

    // Zeit-Elemente generieren
    generateTimeItems(start, end, type) {
        let items = '';
        for (let i = start; i <= end; i++) {
            const value = type === 'minute' && i < 10 ? `0${i}` : i;
            items += `<div class="time-item" data-value="${i}">${value}</div>`;
        }
        return items;
    }

    // Event-Listener für Time Wheels
    setupWheelEvents(wheel) {
        const itemHeight = 50;
        let touchStartY = 0;
        let scrollStartY = 0;
        let lastTouchY = 0;
        let scrollVelocity = 0;
        let lastTouchTime = 0;
        let isScrolling = false;

        // Mausrad-Events
        wheel.addEventListener('wheel', (e) => {
            e.preventDefault();
            const direction = e.deltaY > 0 ? 1 : -1;
            wheel.scrollTop += direction * itemHeight;
            this.updateSelectedItems(wheel);
            clearTimeout(wheel.scrollTimeout);
            wheel.scrollTimeout = setTimeout(() => this.snapToClosestItem(wheel), 50);
        }, { passive: false });

        // Touch-Events
        wheel.addEventListener('touchstart', (e) => {
            touchStartY = e.touches[0].clientY;
            lastTouchY = touchStartY;
            scrollStartY = wheel.scrollTop;
            lastTouchTime = Date.now();
            scrollVelocity = 0;
            isScrolling = true;
            wheel.style.scrollBehavior = 'auto';
        }, { passive: true });

        wheel.addEventListener('touchmove', (e) => {
            if (!isScrolling) return;
            const touch = e.touches[0];
            const currentY = touch.clientY;
            const deltaY = lastTouchY - currentY;
            const currentTime = Date.now();
            const deltaTime = currentTime - lastTouchTime;

            if (deltaTime > 0) {
                scrollVelocity = deltaY / deltaTime;
            }

            wheel.scrollTop = scrollStartY + (touchStartY - currentY);
            this.updateSelectedItems(wheel);

            lastTouchY = currentY;
            lastTouchTime = currentTime;
        }, { passive: true });

        wheel.addEventListener('touchend', () => {
            isScrolling = false;
            wheel.style.scrollBehavior = 'smooth';

            const momentum = Math.abs(scrollVelocity) > 0.5;
            if (momentum) {
                const direction = scrollVelocity > 0 ? 1 : -1;
                const speed = Math.min(Math.abs(scrollVelocity * 100), 300);
                wheel.scrollTop += direction * speed;
            }

            setTimeout(() => this.snapToClosestItem(wheel), momentum ? 300 : 0);
        });

        wheel.addEventListener('scroll', () => {
            if (!isScrolling) {
                this.updateSelectedItems(wheel);
            }
        });
    }

    // Ausgewählte Elemente aktualisieren
    updateSelectedItems(wheel) {
        const items = wheel.querySelectorAll('.time-item');
        const itemHeight = 50;
        const centerIndex = Math.round(wheel.scrollTop / itemHeight);
        
        items.forEach((item, index) => {
            if (index === centerIndex) {
                item.classList.add('selected');
            } else {
                item.classList.remove('selected');
            }
        });
    }

    // Zum nächsten Element snappen
    snapToClosestItem(wheel) {
        const itemHeight = 50;
        const scrollTop = wheel.scrollTop;
        const targetIndex = Math.round(scrollTop / itemHeight);
        const targetScroll = targetIndex * itemHeight;

        if (scrollTop !== targetScroll) {
            wheel.scrollTo({
                top: targetScroll,
                behavior: 'smooth'
            });
        }
    }

    // Zu bestimmter Zeit scrollen
    scrollToTime(hour, minute) {
        const itemHeight = 50;
        this.hourWheel.scrollTop = (hour - 1) * itemHeight;
        this.minuteWheel.scrollTop = minute * itemHeight;
        this.updateSelectedItems(this.hourWheel);
        this.updateSelectedItems(this.minuteWheel);
    }

    // Ausgewählte Zeit abrufen
    getSelectedTime() {
        const getSelectedValue = (wheel) => {
            const itemHeight = 50;
            const index = Math.round(wheel.scrollTop / itemHeight);
            const items = wheel.querySelectorAll('.time-item');
            return items[index] ? parseInt(items[index].getAttribute('data-value')) : 0;
        };

        return {
            hour: getSelectedValue(this.hourWheel),
            minute: getSelectedValue(this.minuteWheel)
        };
    }

    // Zeit setzen
    async setTime() {
        try {
            const time = this.getSelectedTime();
            console.log('Setze Zeit:', time);
            await API.setTime(time.hour, time.minute);
            console.log('Zeit erfolgreich gesetzt');
        } catch (error) {
            console.error('Fehler beim Setzen der Zeit:', error);
            alert(LanguageManager.translate('error_time_set'));
        }
    }

    // Zeit anpassen
    async adjustTime(adjustment) {
        try {
            console.log('Passe Zeit an:', adjustment);
            await API.adjustTime(adjustment);
            console.log('Zeit erfolgreich angepasst');
        } catch (error) {
            console.error('Fehler bei der Zeitanpassung:', error);
            alert(LanguageManager.translate('error_time_adjust'));
        }
    }

    // WLAN zurücksetzen
    async resetWiFi() {
        if (confirm(LanguageManager.translate('confirm_wifi_reset'))) {
            try {
                await API.resetWiFi();
                alert(LanguageManager.translate('wifi_reset_success'));
            } catch (error) {
                console.error('Fehler beim WLAN-Reset:', error);
                alert(LanguageManager.translate('wifi_reset_error'));
            }
        }
    }

    // Gerät neustarten
    async restartDevice() {
        if (confirm(LanguageManager.translate('confirm_restart'))) {
            try {
                await API.restartDevice();
                alert(LanguageManager.translate('restart_success'));
                setTimeout(() => window.location.reload(), 5000);
            } catch (error) {
                console.error('Fehler beim Neustart:', error);
                alert(LanguageManager.translate('restart_error'));
                setTimeout(() => window.location.reload(), 5000);
            }
        }
    }

    // Setup Update Button
    setupUpdateButton() {
        const updateButton = document.getElementById('update-btn');
        const updateDialog = document.getElementById('update-dialog');
        const closeButton = document.getElementById('close-update-dialog');
        
        if (updateButton) {
            updateButton.onclick = () => {
                updateDialog.style.display = 'flex';
                this.showManualUpdateOptions();
                this.checkForUpdates();
            };
        }
        
        if (closeButton) {
            closeButton.onclick = () => {
                updateDialog.style.display = 'none';
            };
        }
        
        // Schließen beim Klick außerhalb des Dialogs
        if (updateDialog) {
            updateDialog.onclick = (e) => {
                if (e.target === updateDialog) {
                    updateDialog.style.display = 'none';
                }
            };
        }
    }

    // Update-Funktionalität starten (für Kompatibilität mit alten Aufrufen)
    startUpdate() {
        try {
            this.showManualUpdateOptions();
            this.checkForUpdates();
        } catch (error) {
            console.error('Fehler beim Aufruf des Update-Prozesses:', error);
            alert('Fehler beim Starten des Update-Prozesses: ' + error.message);
        }
    }
    
    // Check for updates and update the menu with status
    async checkForUpdates() {
        const statusElement = document.getElementById('update-status');
        if (statusElement) {
            statusElement.innerHTML = `
                <div class="update-status-message">
                    ${LanguageManager.translate('checking_updates')}
                </div>
            `;
        }
        
        try {
            const updateInfo = await API.checkUpdate();
            console.log('Update-Info erhalten:', updateInfo);
            
            // Versionsvergleich
            const current = updateInfo.currentVersion || FW_VERSION || "1.0.0";
            const latest = updateInfo.latestVersion || "unbekannt";
            
            // Update-Status bestimmen
            const updateAvailable = this.compareVersions(latest, current) > 0;
            
            // Status-Objekt erstellen
            const statusInfo = {
                error: updateInfo.error,
                errorDetails: updateInfo.errorDetails,
                updateAvailable: updateAvailable,
                firmwareUrl: updateInfo.firmwareUrl,
                filesystemUrl: updateInfo.filesystemUrl,
                currentVersion: current,
                latestVersion: latest
            };
            
            // Status anzeigen
            this.updateStatusDisplay(statusInfo);
            
        } catch (error) {
            console.error('Fehler bei der Update-Prüfung:', error);
            this.updateStatusDisplay({
                error: error.message,
                errorDetails: 'Bitte versuchen Sie es später erneut.'
            });
        }
    }
    
    // Manuelle Update-Optionen anzeigen
    showManualUpdateOptions() {
        const updateDialog = document.getElementById('update-dialog');
        const updateContent = document.getElementById('update-content');
        
        if (!updateContent) {
            console.error('Update-Content Element nicht gefunden');
            return;
        }
        
        updateContent.innerHTML = `
            <div class="update-menu">
                <h3>${LanguageManager.translate('check_update')}</h3>
                <div id="update-status"></div>
                <div class="update-buttons">
                    <div class="update-option" id="view-releases-btn" onclick="window.open('https://github.com/sogidaned/Hollowclock/releases', '_blank')">
                        <i class="fas fa-external-link-alt"></i>
                        <span>${LanguageManager.translate('view_releases')}</span>
                    </div>
                    <div class="update-option" id="manual-update-btn" onclick="window.location.href='/firmware'">
                        <i class="fas fa-upload"></i>
                        <span>${LanguageManager.translate('manual_update')}</span>
                    </div>
                </div>
            </div>
        `;
        
        updateDialog.style.display = 'flex';
    }

    // Update-Status aktualisieren
    updateStatusDisplay(updateInfo) {
        const statusElement = document.getElementById('update-status');
        if (!statusElement) return;

        // Speichere die URLs für die spätere Verwendung
        this.latestFirmwareUrl = updateInfo.firmwareUrl;
        this.latestFilesystemUrl = updateInfo.filesystemUrl;

        let content = '';
        
        // Version-Info anzeigen
        content += `
            <div class="version-info">
                <div>${LanguageManager.translate('current_version') || 'Aktuelle Version'}: <strong>${updateInfo.currentVersion}</strong></div>
                <div>${LanguageManager.translate('latest_version') || 'Neueste Version'}: <strong>${updateInfo.latestVersion}</strong></div>
            </div>
        `;
        
        if (updateInfo.error) {
            content += `
                <div class="error-message">
                    ${updateInfo.error}
                    ${updateInfo.errorDetails || ''}
                </div>
                <button id="retry-update-check" class="update-retry-btn" onclick="window.ui.checkForUpdates()">
                    ${LanguageManager.translate('retry_update') || 'Erneut prüfen'}
                </button>
            `;
        } else if (updateInfo.updateAvailable) {
            // Prüfen, ob beide Update-URLs vorhanden sind
            const hasValidUrls = updateInfo.firmwareUrl && updateInfo.filesystemUrl;
            
            content += `
                <div class="update-available-message">
                    ${LanguageManager.translate('update_available').split('\n')[0] || 'Update verfügbar!'}
                </div>
            `;
            
            if (hasValidUrls) {
                content += `
                    <div class="update-option" onclick="window.ui.startAutoUpdate('${updateInfo.firmwareUrl}', '${updateInfo.filesystemUrl}')">
                        <i class="fas fa-cloud-download-alt"></i>
                        <span>${LanguageManager.translate('auto_update') || 'Automatische Aktualisierung'}</span>
                    </div>
                    <div class="update-option" onclick="window.ui.startFirmwareUpdate('${updateInfo.firmwareUrl}')">
                        <i class="fas fa-microchip"></i>
                        <span>${LanguageManager.translate('update_firmware') || 'Firmware aktualisieren'}</span>
                    </div>
                    <div class="update-option" onclick="window.ui.startFilesystemUpdate('${updateInfo.filesystemUrl}')">
                        <i class="fas fa-hdd"></i>
                        <span>${LanguageManager.translate('update_filesystem') || 'Dateisystem aktualisieren'}</span>
                    </div>
                `;
            } else {
                content += `
                    <div class="update-error-message">
                        ${LanguageManager.translate('update_files_missing') || 'Update-Dateien nicht gefunden'}
                    </div>
                    <div class="update-option disabled">
                        <i class="fas fa-cloud-download-alt"></i>
                        <span>${LanguageManager.translate('auto_update') || 'Automatische Aktualisierung'}</span>
                    </div>
                    <div class="update-option" id="view-releases-btn" onclick="window.open('https://github.com/sogidaned/Hollowclock/releases/tag/v${updateInfo.latestVersion}', '_blank')">
                        <i class="fas fa-external-link-alt"></i>
                        <span>${LanguageManager.translate('view_releases') || 'Releases anzeigen'}</span>
                    </div>
                `;
            }
        } else {
            content += `
                <div class="status-message">
                    ${LanguageManager.translate('no_update_available').replace('{version}', updateInfo.currentVersion) || 'Kein Update verfügbar'}
                </div>
            `;
        }
        
        statusElement.innerHTML = content;
    }

    // Firmware Update starten
    async startFirmwareUpdate(url) {
        const statusElement = document.getElementById('update-status');
        if (statusElement) {
            statusElement.innerHTML = `
                <div class="update-status-message">
                    <p>${LanguageManager.translate('updating_firmware', 'Firmware wird aktualisiert...')}</p>
                    <div class="loading-spinner"></div>
                </div>
            `;
        }
        
        try {
            const response = await API.startUpdate('firmware', url);
            if (response.status === 'ok') {
                if (statusElement) {
                    statusElement.innerHTML = `
                        <div class="update-status-message">
                            <p>✅ ${LanguageManager.translate('update_success', 'Update erfolgreich!')}</p>
                            <p>${LanguageManager.translate('restarting_device', 'Gerät startet neu...')}</p>
                        </div>
                    `;
                }
                
                // Verzögerter Reload, um Statusmeldung anzuzeigen
                setTimeout(() => {
                    location.reload();
                }, 5000);
            } else {
                throw new Error(response.error || 'Unbekannter Fehler');
            }
        } catch (error) {
            console.error('Firmware Update Fehler:', error);
            
            if (statusElement) {
                statusElement.innerHTML = `
                    <div class="update-status-message error">
                        <p>❌ ${LanguageManager.translate('update_error', 'Fehler beim Update')}: ${error.message}</p>
                        <button class="btn" onclick="window.location.reload()">${LanguageManager.translate('retry_update', 'Erneut versuchen')}</button>
                    </div>
                `;
            } else {
                alert(LanguageManager.translate('update_error') + ': ' + error.message);
            }
        }
    }

    // Filesystem Update starten
    async startFilesystemUpdate(url) {
        const statusElement = document.getElementById('update-status');
        if (statusElement) {
            statusElement.innerHTML = `
                <div class="update-status-message">
                    <p>${LanguageManager.translate('updating_filesystem', 'Dateisystem wird aktualisiert...')}</p>
                    <div class="loading-spinner"></div>
                </div>
            `;
        }
        
        try {
            const response = await API.startUpdate('filesystem', url);
            if (response.status === 'ok') {
                if (statusElement) {
                    statusElement.innerHTML = `
                        <div class="update-status-message">
                            <p>✅ ${LanguageManager.translate('update_success', 'Update erfolgreich!')}</p>
                            <p>${LanguageManager.translate('restarting_device', 'Gerät startet neu...')}</p>
                        </div>
                    `;
                }
                
                // Verzögerter Reload, um Statusmeldung anzuzeigen
                setTimeout(() => {
                    location.reload();
                }, 5000);
            } else {
                throw new Error(response.error || 'Unbekannter Fehler');
            }
        } catch (error) {
            console.error('Filesystem Update Fehler:', error);
            
            if (statusElement) {
                statusElement.innerHTML = `
                    <div class="update-status-message error">
                        <p>❌ ${LanguageManager.translate('update_error', 'Fehler beim Update')}: ${error.message}</p>
                        <button class="btn" onclick="window.location.reload()">${LanguageManager.translate('retry_update', 'Erneut versuchen')}</button>
                    </div>
                `;
            } else {
                alert(LanguageManager.translate('update_error') + ': ' + error.message);
            }
        }
    }

    // Automatisches Update starten
    async startAutoUpdate(firmwareUrl, filesystemUrl) {
        // Nutze übergebene URLs oder gespeicherte URLs
        const fwUrl = firmwareUrl || this.latestFirmwareUrl;
        const fsUrl = filesystemUrl || this.latestFilesystemUrl;

        if(!confirm(LanguageManager.translate('confirm_update', 'Möchten Sie ein automatisches Update durchführen?'))) {
            return;
        }

        try {
            const statusElement = document.getElementById('update-status');
            statusElement.innerHTML = `<p>${LanguageManager.translate('updating', 'Aktualisierung wird durchgeführt...')}</p>`;
            
            // Firmware-Update starten
            console.log('Starte Firmware-Update...');
            const firmwareResult = await API.startUpdate('firmware', fwUrl);
            
            if (firmwareResult && !firmwareResult.error) {
                statusElement.innerHTML = `
                    <p>✅ ${LanguageManager.translate('firmware_updated', 'Firmware erfolgreich aktualisiert!')}</p>
                    <p>${LanguageManager.translate('updating_filesystem', 'Dateisystem wird aktualisiert...')}</p>
                `;
                
                // Dateisystem-Update starten
                console.log('Starte Dateisystem-Update...');
                const fsResult = await API.startUpdate('filesystem', fsUrl);
                
                if (fsResult && !fsResult.error) {
                    statusElement.innerHTML = `
                        <p>✅ ${LanguageManager.translate('update_complete', 'Update abgeschlossen!')}</p>
                        <p>${LanguageManager.translate('device_restarting', 'Gerät wird neu gestartet...')}</p>
                    `;
                    // Nach 5 Sekunden die Seite neu laden
                    setTimeout(() => {
                        window.location.reload();
                    }, 5000);
                } else {
                    statusElement.innerHTML = `
                        <p>✅ ${LanguageManager.translate('firmware_updated', 'Firmware erfolgreich aktualisiert')}</p>
                        <p>❌ ${LanguageManager.translate('filesystem_update_failed', 'Dateisystem-Update fehlgeschlagen')}: ${fsResult ? fsResult.error : 'Unbekannter Fehler'}</p>
                        <button class="btn" onclick="window.location.reload()">${LanguageManager.translate('retry_update', 'Erneut versuchen')}</button>
                    `;
                }
            } else {
                statusElement.innerHTML = `
                    <p>❌ ${LanguageManager.translate('firmware_update_failed', 'Firmware-Update fehlgeschlagen')}: ${firmwareResult ? firmwareResult.error : 'Unbekannter Fehler'}</p>
                    <button class="btn" onclick="window.location.reload()">${LanguageManager.translate('retry_update', 'Erneut versuchen')}</button>
                `;
            }
        } catch (error) {
            console.error('Fehler beim automatischen Update:', error);
            const statusElement = document.getElementById('update-status');
            statusElement.innerHTML = `
                <p>❌ ${LanguageManager.translate('update_error', 'Fehler beim Update')}: ${error.message}</p>
                <button class="btn" onclick="window.location.reload()">${LanguageManager.translate('retry_update', 'Erneut versuchen')}</button>
            `;
        }
    }

    // Versionen vergleichen (returns 1 if v1 > v2, -1 if v1 < v2, 0 if equal)
    compareVersions(v1, v2) {
        const v1parts = v1.split('.').map(Number);
        const v2parts = v2.split('.').map(Number);
        
        for (let i = 0; i < v1parts.length; ++i) {
            if (v2parts.length === i) {
                return 1;
            }
            if (v1parts[i] > v2parts[i]) {
                return 1;
            }
            if (v1parts[i] < v2parts[i]) {
                return -1;
            }
        }
        
        if (v1parts.length !== v2parts.length) {
            return -1;
        }
        
        return 0;
    }

    // MQTT-Konfiguration speichern
    async saveMqttConfig() {
        console.log('Speichere MQTT-Konfiguration...');
        const mqttStatus = document.getElementById('mqtt-status');
        const mqttEnabled = document.getElementById('mqtt-enabled').checked;
            let server = document.getElementById('mqtt-broker').value.trim();
            
        // Entferne mqtt:// falls vorhanden
            if (server.startsWith('mqtt://')) {
                server = server.substring(7);
            }
            
            const config = {
            enabled: mqttEnabled,  // MQTT-Aktivierungsstatus
                server: server,
                port: parseInt(document.getElementById('mqtt-port').value),
                clientId: document.getElementById('mqtt-client-id').value.trim(),
                topic: document.getElementById('mqtt-topic').value.trim(),
                user: document.getElementById('mqtt-username').value.trim(),
                password: document.getElementById('mqtt-password').value
            };

        // Validiere die Eingaben nur wenn MQTT aktiviert ist
        if (mqttEnabled) {
            if (!config.server) {
                alert(LanguageManager.translate('mqtt_broker_empty'));
                return;
            }

            if (isNaN(config.port) || config.port < 1 || config.port > 65535) {
                alert(LanguageManager.translate('mqtt_port_invalid'));
                return;
            }

            if (!config.clientId) {
                alert(LanguageManager.translate('mqtt_client_empty'));
                return;
            }

            if (!config.topic) {
                alert(LanguageManager.translate('mqtt_topic_empty'));
                return;
            }
            }

            try {
                await API.setMqttConfig(config);
            mqttStatus.textContent = LanguageManager.translate('mqtt_config_saved');
                mqttStatus.className = 'status-indicator success';
            
            // Aktualisiere Status nach kurzer Verzögerung
            setTimeout(async () => {
                const status = await API.getMqttStatus();
                this.updateMqttStatus(status);
            }, 2000);
        } catch (error) {
            console.error('Fehler beim Speichern der MQTT-Konfiguration:', error);
            this.updateMqttStatus({ connected: false, lastError: LanguageManager.translate('mqtt_save_error') });
        }
    }

    // MQTT-Konfiguration zurücksetzen
    async resetMqttConfig() {
        console.log('Setze MQTT-Konfiguration zurück...');
        const mqttStatus = document.getElementById('mqtt-status');
        
        if (confirm(LanguageManager.translate('confirm_mqtt_reset'))) {
            try {
                // MQTT-Konfiguration zurücksetzen
                await API.resetMqttConfig();
                
                // UI aktualisieren
                mqttStatus.textContent = LanguageManager.translate('mqtt_reset_success');
                mqttStatus.className = 'status-indicator warning';
                
                // Formular leeren
                document.getElementById('mqtt-broker').value = 'homeassistant.local';
                document.getElementById('mqtt-port').value = '1883';
                document.getElementById('mqtt-client-id').value = 'Hollowclock';
                document.getElementById('mqtt-topic').value = 'Hollowclock';
                document.getElementById('mqtt-username').value = '';
                document.getElementById('mqtt-password').value = '';
                
                // Aktualisiere Status nach kurzer Verzögerung
                setTimeout(async () => {
                    const status = await API.getMqttStatus();
                    this.updateMqttStatus(status);
                }, 2000);
            } catch (error) {
                console.error('Fehler beim Zurücksetzen der MQTT-Konfiguration:', error);
                mqttStatus.textContent = LanguageManager.translate('mqtt_reset_error');
                mqttStatus.className = 'status-indicator error';
            }
        }
    }

    // MQTT-Konfiguration initialisieren
    async initMqttConfig() {
        console.log('Initialisiere MQTT-Konfiguration...');
        
        const mqttStatus = document.getElementById('mqtt-status');
        const mqttBrokerInput = document.getElementById('mqtt-broker');
        const mqttEnabledSwitch = document.getElementById('mqtt-enabled');
        
        // Lade aktuelle Konfiguration
        try {
            const config = await API.getMqttConfig();
            
            // MQTT-Aktivierung setzen (standardmäßig deaktiviert)
            mqttEnabledSwitch.checked = config.enabled === true;
            this.toggleMqttFields(config.enabled === true);
            
            // Setze Standardwerte, falls keine Konfiguration existiert
            mqttBrokerInput.value = config.server || 'homeassistant.local';
            document.getElementById('mqtt-port').value = config.port || 1883;
            document.getElementById('mqtt-client-id').value = config.clientId || 'ESP32test';
            document.getElementById('mqtt-topic').value = config.topic || 'ESP32test';
            document.getElementById('mqtt-username').value = config.user || '';
            document.getElementById('mqtt-password').value = ''; // Passwort wird nie angezeigt
            
            // Zeige aktuellen MQTT-Status
            const status = await API.getMqttStatus();
            this.updateMqttStatus(status, config.enabled === true);
            
            // Event-Listener für MQTT-Aktivierung
            mqttEnabledSwitch.addEventListener('change', (e) => {
                this.toggleMqttFields(e.target.checked);
            });
        } catch (error) {
            console.error('Fehler beim Laden der MQTT-Konfiguration:', error);
            this.updateMqttStatus({ connected: false, lastError: LanguageManager.translate('mqtt_load_error') }, false);
        }
        
        // Hinweis: Event-Listener werden jetzt zentral in app.js gesetzt
    }
    
    // MQTT-Felder ein-/ausblenden je nach Status
    toggleMqttFields(enabled) {
        const mqttFieldsContainer = document.getElementById('mqtt-fields');
        if (mqttFieldsContainer) {
            mqttFieldsContainer.style.opacity = enabled ? '1' : '0.5';
            mqttFieldsContainer.style.pointerEvents = enabled ? 'all' : 'none';
        }
        
        // Status aktualisieren, wenn MQTT deaktiviert ist
        if (!enabled) {
            const mqttStatus = document.getElementById('mqtt-status');
            const mqttError = document.getElementById('mqtt-error');
            
            if (mqttStatus) {
                mqttStatus.textContent = LanguageManager.translate('mqtt_disconnected');
                mqttStatus.className = 'status-indicator inactive';
            }
            
            if (mqttError) {
                mqttError.textContent = LanguageManager.translate('mqtt_disabled');
            }
        }
    }

    // MQTT-Status aktualisieren
    updateMqttStatus(status, enabled = true) {
        const mqttStatus = document.getElementById('mqtt-status');
        const mqttError = document.getElementById('mqtt-error');
        
        if (!enabled) {
            mqttStatus.textContent = LanguageManager.translate('mqtt_disconnected');
            mqttStatus.className = 'status-indicator inactive';
            mqttError.textContent = LanguageManager.translate('mqtt_disabled');
            return;
        }
        
        if (status.connected) {
            mqttStatus.textContent = LanguageManager.translate('mqtt_connected');
            mqttStatus.className = 'status-indicator connected';
            mqttError.textContent = '';
        } else {
            mqttStatus.textContent = LanguageManager.translate('mqtt_disconnected');
            mqttStatus.className = 'status-indicator disconnected';
            mqttError.textContent = status.lastError || LanguageManager.translate('mqtt_no_connection');
        }
    }

    // Alle Einstellungen zurücksetzen
    async resetAllSettings() {
        if (confirm(LanguageManager.translate('confirm_reset_all'))) {
            try {
                await API.resetAllSettings();
                alert(LanguageManager.translate('reset_all_success'));
                // Verzögerung, um dem Gerät Zeit zum Neustarten zu geben
                setTimeout(() => {
                    window.location.reload();
                }, 5000);
            } catch (error) {
                console.error('Fehler beim Zurücksetzen aller Einstellungen:', error);
                alert(LanguageManager.translate('reset_all_error'));
            }
        }
    }
}

// UI-Instanz erstellen und global verfügbar machen
const ui = new UI();
window.ui = ui; // Global verfügbar machen

export default ui; 
// Übersetzungen für die HollowClock-Oberfläche
const translations = {
    'de': {
        // Allgemeine Texte
        'title': 'Hollowclock',
        'subtitle': 'Smart-Steuerung für Hollow Clock',
        
        // Sektionen
        'current_time': 'Aktuelle Uhrzeit',
        'current_position': 'Aktuelle Position:',
        'ntp_time': 'NTP-Serverzeit:',
        'motor_position': 'Motorposition:',
        
        'calibrate': 'Position kalibrieren',
        'calibrate_desc': 'Stellen Sie die aktuelle Zeit der Uhr hier ein:',
        'calibrate_note': 'Bei einem 12-Uhr-Übergang während der Kalibrierung wird die Weboberfläche kurzfristig pausiert, um eine Fehlkalibrierung zu verhindern. Bitte warten Sie, bis der Vorgang abgeschlossen ist.',
        'save_position': 'Position speichern',
        
        'fine_adjustment': 'Feinjustierung',
        'fine_adjustment_desc': 'Passen Sie die Position der Uhrzeiger mit diesen Schaltflächen an:',
        'minus_30': '- 30 Sekunden',
        'plus_30': '+ 30 Sekunden',
        
        'system_info': 'System-Informationen',
        'version': 'VERSION:',
        'ip_address': 'IP-ADRESSE:',
        'wifi_signal': 'WLAN-SIGNAL:',
        'uptime': 'LAUFZEIT:',
        
        'system_functions': 'System-Funktionen',
        'firmware_update': 'Firmware-Update',
        'restart': 'Neustart',
        'reset_wifi': 'WLAN zurücksetzen',
        'reset_all': 'Alle Einstellungen zurücksetzen',
        
        'mqtt_config': 'MQTT-Konfiguration',
        'mqtt_info': 'Verbinden Sie Ihre Uhr mit einer MQTT-Instanz, um sie über Home Assistant zu steuern.',
        'mqtt_broker': 'MQTT Broker:',
        'port': 'Port:',
        'client_id': 'Client-ID:',
        'topic': 'Topic:',
        'username': 'Benutzername:',
        'password': 'Passwort:',
        'status': 'Status:',
        'save_mqtt': 'MQTT-Einstellungen speichern',
        'reset_mqtt': 'MQTT zurücksetzen',
        'save_settings': 'Einstellungen speichern',
        
        // WLAN-Signal
        'very_good': 'Sehr gut',
        'good': 'Gut',
        'medium': 'Mittel',
        'weak': 'Schwach',
        'very_weak': 'Sehr schwach',
        
        // Footer
        'footer': 'Hollowclock v1.0 - ',
        
        // Sprachumschalter
        'language': 'Sprache:',
        
        // Benachrichtigungen
        'error_time_set': 'Fehler beim Setzen der Zeit',
        'error_time_adjust': 'Fehler bei der Zeitanpassung',
        'confirm_wifi_reset': 'WLAN-Einstellungen wirklich zurücksetzen? Das Gerät startet danach neu im Access-Point-Modus.',
        'wifi_reset_success': 'WLAN-Einstellungen wurden zurückgesetzt. Verbinden Sie sich mit dem WLAN-Hotspot der Uhr.',
        'wifi_reset_error': 'WLAN-Einstellungen wurden möglicherweise zurückgesetzt. Verbinden Sie sich mit dem WLAN-Hotspot der Uhr.',
        'confirm_restart': 'Gerät wirklich neu starten?',
        'restart_success': 'Das Gerät wird neu gestartet...',
        'restart_error': 'Das Gerät wird möglicherweise neu gestartet...',
        'update_message': 'Wählen Sie die Update-Art:\n\n1. Nur Firmware aktualisieren\n2. Firmware UND Dateisystem aktualisieren\n\nHinweis: Nach einem Firmware-Update ohne Dateisystem-Update bleiben alle Webseiten-Dateien erhalten. Bei einem kompletten Update muss das Dateisystem auch neu geladen werden.',
        'mqtt_broker_empty': 'Bitte geben Sie eine Broker-Adresse ein',
        'mqtt_port_invalid': 'Bitte geben Sie einen gültigen Port ein (1-65535)',
        'mqtt_client_empty': 'Bitte geben Sie eine Client-ID ein',
        'mqtt_topic_empty': 'Bitte geben Sie ein Topic ein',
        'mqtt_config_saved': 'Konfiguration gespeichert',
        'mqtt_save_error': 'Fehler beim Speichern',
        'mqtt_load_error': 'Fehler beim Laden der MQTT-Konfiguration',
        'confirm_mqtt_reset': 'Möchten Sie wirklich die MQTT-Konfiguration zurücksetzen? Die Verbindung zum MQTT-Broker wird getrennt.',
        'mqtt_reset_success': 'MQTT-Konfiguration zurückgesetzt',
        'mqtt_reset_error': 'Fehler beim Zurücksetzen',
        'mqtt_disconnected': 'Nicht verbunden',
        'mqtt_no_connection': 'Keine Verbindung',
        'mqtt_disabled': 'MQTT ist deaktiviert',
        'mqtt_connected': 'Verbunden',
        'confirm_reset_all': 'WARNUNG: Alle Einstellungen (WLAN, MQTT, Position) werden zurückgesetzt und das Gerät startet neu. Fortfahren?',
        'reset_all_success': 'Alle Einstellungen wurden zurückgesetzt. Das Gerät startet neu...',
        'reset_all_error': 'Fehler beim Zurücksetzen aller Einstellungen. Das Gerät startet möglicherweise neu.',
        'checking_updates': 'Suche nach Updates...',
        'updating_firmware': 'Firmware wird aktualisiert...',
        'updating_filesystem': 'Webinterface wird aktualisiert...',
        'update_complete_restart': 'Update erfolgreich! Die Uhr startet neu...',
        'firmware_update_failed': 'Firmware-Update fehlgeschlagen',
        'filesystem_update_failed': 'Webinterface-Update fehlgeschlagen',
        'update_check_error': 'Fehler beim Prüfen auf Updates',
        'update_check_error_confirm': 'Es konnte keine Verbindung zur GitHub API hergestellt werden. Möchten Sie die manuelle Update-Seite öffnen?',
        'view_releases': 'Releases anzeigen',
        'no_update_available': 'Kein Update verfügbar. Sie verwenden die neueste Version ({version}).',
        'update_available': 'Update verfügbar!\n\nAktuelle Version: {current}\nNeue Version: {latest}\n\nMöchten Sie das Update jetzt installieren?',
        'update_error': 'Update-Fehler',
        'manual_update': 'Manuelles Update',
        'auto_update': 'Automatisches Update',
        'confirm_auto_update': 'Möchten Sie wirklich die automatische Aktualisierung starten? Die Uhr wird nach dem Update neu gestartet.',
        'check_updates': 'Auf Updates prüfen',
        'update_options': 'Update-Optionen',
        'manual_update_options': 'Manuelle Update-Optionen',
        
        // Update-Übersetzungen
        'current_version': 'Aktuelle Version',
        'latest_version': 'Neueste Version',
        'update_firmware': 'Firmware aktualisieren',
        'update_filesystem': 'Dateisystem aktualisieren',
        'retry_update': 'Erneut versuchen',
        'update_files_missing': 'Update-Dateien nicht gefunden. Bitte besuchen Sie die Releases-Seite für einen manuellen Download.',
        'update_success': 'Update erfolgreich! Das Gerät wird neu gestartet.',
    },
    'en': {
        // General texts
        'title': 'Hollowclock',
        'subtitle': 'Smart control for Hollow Clock',
        
        // Sections
        'current_time': 'Current Time',
        'current_position': 'Current Position:',
        'ntp_time': 'NTP Server Time:',
        'motor_position': 'Motor Position:',
        
        'calibrate': 'Calibrate Position',
        'calibrate_desc': 'Set the current time of the clock here:',
        'calibrate_note': 'During a 12-hour transition in calibration, the web interface will pause briefly to prevent miscalibration. Please wait until the process is complete.',
        'save_position': 'Save Position',
        
        'fine_adjustment': 'Fine Adjustment',
        'fine_adjustment_desc': 'Adjust the clock hands position with these buttons:',
        'minus_30': '- 30 Seconds',
        'plus_30': '+ 30 Seconds',
        
        'system_info': 'System Information',
        'version': 'VERSION:',
        'ip_address': 'IP ADDRESS:',
        'wifi_signal': 'WIFI SIGNAL:',
        'uptime': 'UPTIME:',
        
        'system_functions': 'System Functions',
        'firmware_update': 'Firmware Update',
        'restart': 'Restart',
        'reset_wifi': 'Reset WiFi',
        'reset_all': 'Reset All Settings',
        
        'mqtt_config': 'MQTT Configuration',
        'mqtt_info': 'Connect your clock to an MQTT instance to control it with Home Assistant.',
        'mqtt_broker': 'MQTT Broker:',
        'port': 'Port:',
        'client_id': 'Client ID:',
        'topic': 'Topic:',
        'username': 'Username:',
        'password': 'Password:',
        'status': 'Status:',
        'save_mqtt': 'Save MQTT Settings',
        'reset_mqtt': 'Reset MQTT',
        'save_settings': 'Save Settings',
        
        // WiFi Signal
        'very_good': 'Very Good',
        'good': 'Good',
        'medium': 'Medium',
        'weak': 'Weak',
        'very_weak': 'Very Weak',
        
        // Footer
        'footer': 'Hollowclock v1.0 - ',
        
        // Language switcher
        'language': 'Language:',
        
        // Notifications
        'error_time_set': 'Error setting time',
        'error_time_adjust': 'Error adjusting time',
        'confirm_wifi_reset': 'Really reset WiFi settings? The device will restart in access point mode.',
        'wifi_reset_success': 'WiFi settings have been reset. Connect to the clock\'s WiFi hotspot.',
        'wifi_reset_error': 'WiFi settings may have been reset. Connect to the clock\'s WiFi hotspot.',
        'confirm_restart': 'Really restart device?',
        'restart_success': 'The device is restarting...',
        'restart_error': 'The device may be restarting...',
        'update_message': 'Choose update type:\n\n1. Update firmware only\n2. Update firmware AND file system\n\nNote: After a firmware update without file system update, all website files remain intact. With a complete update, the file system must also be reloaded.',
        'mqtt_broker_empty': 'Please enter a broker address',
        'mqtt_port_invalid': 'Please enter a valid port (1-65535)',
        'mqtt_client_empty': 'Please enter a client ID',
        'mqtt_topic_empty': 'Please enter a topic',
        'mqtt_config_saved': 'Configuration saved',
        'mqtt_save_error': 'Error saving',
        'mqtt_load_error': 'Error loading MQTT configuration',
        'confirm_mqtt_reset': 'Do you really want to reset the MQTT configuration? The connection to the MQTT broker will be disconnected.',
        'mqtt_reset_success': 'MQTT configuration reset',
        'mqtt_reset_error': 'Error resetting',
        'mqtt_disconnected': 'Not connected',
        'mqtt_no_connection': 'No connection',
        'mqtt_disabled': 'MQTT is disabled',
        'mqtt_connected': 'Connected',
        'confirm_reset_all': 'WARNING: All settings (WiFi, MQTT, position) will be reset and the device will restart. Continue?',
        'reset_all_success': 'All settings have been reset. The device is restarting...',
        'reset_all_error': 'Error resetting all settings. The device may restart.',
        'checking_updates': 'Checking for updates...',
        'updating_firmware': 'Updating firmware...',
        'updating_filesystem': 'Updating web interface...',
        'update_complete_restart': 'Update successful! Clock is restarting...',
        'firmware_update_failed': 'Firmware update failed',
        'filesystem_update_failed': 'Web interface update failed',
        'update_check_error': 'Error checking for updates',
        'update_check_error_confirm': 'Could not connect to the GitHub API. Would you like to open the manual update page?',
        'view_releases': 'View GitHub Releases',
        'no_update_available': 'No update available. You are using the latest version ({version}).',
        'update_available': 'Update available!\n\nCurrent version: {current}\nNew version: {latest}\n\nDo you want to install the update now?',
        'update_error': 'Update error',
        'manual_update': 'Manual Update',
        'auto_update': 'Automatic Update',
        'confirm_auto_update': 'Do you really want to start the automatic update? The clock will restart after the update.',
        'check_updates': 'Check for Updates',
        'update_options': 'Update Options',
        'manual_update_options': 'Manual Update Options',
        
        // Update translations
        'current_version': 'Current Version',
        'latest_version': 'Latest Version',
        'update_firmware': 'Update Firmware',
        'update_filesystem': 'Update Filesystem',
        'retry_update': 'Retry',
        'update_files_missing': 'Update files not found. Please visit the releases page to download manually.',
        'update_success': 'Update successful! The device will restart.',
    },
    'by': {
        // Allgemeine Texte
        'title': 'Hoiglock',
        'subtitle': 'Pfiffige Steiarung füa Durchschaug-Uhr',
        
        // Sektionen
        'current_time': 'Aktuelle Zeid',
        'current_position': 'Wo da Zeiga steht:',
        'ntp_time': 'Internetzeid vom Server:',
        'motor_position': 'Wo da Motor steht:',
        
        'calibrate': 'Uhr nei eistelln',
        'calibrate_desc': 'Stoi de Zeid von da Uhr do ei:',
        'calibrate_note': 'Wenn da Zeiga üba 12e rutschn muaß, geht kuaz nix am Bildschirm. Des is normal, sunst kimmt de Uhr durchananda. Wart hoid a weng.',
        'save_position': 'Speichan',
        
        'fine_adjustment': 'Genau eistelln',
        'fine_adjustment_desc': 'Stoi den Uahzeiga genau ei mid de Knepf:',
        'minus_30': '- 30 Sekund\'n',
        'plus_30': '+ 30 Sekund\'n',
        
        'system_info': 'Technischs Glumpad',
        'version': 'VERSION:',
        'ip_address': 'IP-NUMMER:',
        'wifi_signal': 'FUNK-STÄRKN:',
        'uptime': 'LAFFT SCHO:',
        
        'system_functions': 'Technische Knepf',
        'firmware_update': 'Neis Programm aufspuin',
        'restart': 'Nei startma',
        'reset_wifi': 'WLAN zruggsetzn',
        'reset_all': 'Ois zruggsetzn',
        
        'mqtt_config': 'MQTT-Eistellunga',
        'mqtt_info': 'Verbind dei Uhr mid am MQTT-Dingsbums füan Home Assistant.',
        'mqtt_broker': 'MQTT Dings:',
        'port': 'Port:',
        'client_id': 'Kennung:',
        'topic': 'Thema:',
        'username': 'Nutzaname:',
        'password': 'Passwort:',
        'status': 'Zuastand:',
        'save_mqtt': 'Eistellunga speichan',
        'reset_mqtt': 'MQTT zruggsetzn',
        'save_settings': 'Eistellunga speichan',
        
        // WLAN-Signal
        'very_good': 'Subba',
        'good': 'Basst scho',
        'medium': 'Geht so',
        'weak': 'A bisserl schwa',
        'very_weak': 'Miserabel',
        
        // Footer
        'footer': 'Hoiglock v1.0 - ',
        
        // Sprachumschalter
        'language': 'Sproch:',
        
        // Benachrichtigungen
        'error_time_set': 'Basst ned mit da Zeiteistellung',
        'error_time_adjust': 'Basst ned mit da Zeitanpassung',
        'confirm_wifi_reset': 'Wuist wirkli des WLAN zrucksetzn? Des Gerät startet dann nei im Hotspot-Modus.',
        'wifi_reset_success': 'WLAN is zruckgsetzt. Verbind di mit dem WLAN-Hotspot vo da Uhr.',
        'wifi_reset_error': 'WLAN is vielleicht zruckgsetzt. Verbind di mit dem WLAN-Hotspot vo da Uhr.',
        'confirm_restart': 'Wuist de Uhr wirkli nei startn?',
        'restart_success': 'De Uhr wird nei gstartet...',
        'restart_error': 'Kennt sei, dass de Uhr nei gstartet werd...',
        'update_message': 'Wähl, wos neigspiut werdn soi:\n\n1. Nua s\'Programm aktualisiern\n2. Programm UND Dateisystem aktualisiern\n\nMerk da: Nach am kloan Update bleim olle Webseitn gleich. Bei am komplettn Update werd ois nei afgspiut.',
        'mqtt_broker_empty': 'Do muast scho a Adress eigebn',
        'mqtt_port_invalid': 'Gib an richtign Port ei (1-65535)',
        'mqtt_client_empty': 'Do muast scho a Kennung eigebn',
        'mqtt_topic_empty': 'Do muast scho a Thema eigebn',
        'mqtt_config_saved': 'Is gspeichert',
        'mqtt_save_error': 'Hat ned klappt mid\'m Speichern',
        'mqtt_load_error': 'Konn de MQTT-Eistellunga ned ladn',
        'confirm_mqtt_reset': 'Wuist wirkli de MQTT-Eistellunga zrücksetzn? De Verbindung zum MQTT-Dings werd dann wega.',
        'mqtt_reset_success': 'MQTT-Eistellunga san zruckgsetzt',
        'mqtt_reset_error': 'Beim Zrücksetzn is wos schiafglaufa',
        'mqtt_disconnected': 'Ned verbundn',
        'mqtt_no_connection': 'Koa Verbindung do',
        'mqtt_disabled': 'MQTT is ausgmacht',
        'mqtt_connected': 'Verbundn',
        'confirm_reset_all': 'OBACHT: Olle Eistellunga (WLAN, MQTT, Position) werdn zrückgsetzt und dei Uhr startet nei. Weitamachn?',
        'reset_all_success': 'Ois is zruckgsetzt worn. De Uhr startet glei nei...',
        'reset_all_error': 'Beim Zrücksetzn is wos schiafglaffa. Kennt sei, dass de Uhr trotzdem nei startet.',
        'checking_updates': 'Schau noch Updats...',
        'updating_firmware': 'Spui a neis Programm auf...',
        'updating_filesystem': 'Spui neie Webseitn auf...',
        'update_complete_restart': 'Ois guat! D\'Uhr startet nei...',
        'firmware_update_failed': 'Updaten hot ned funktioniert',
        'filesystem_update_failed': 'Webseitn-Update hot ned funktioniert',
        'update_check_error': 'Konnt ned schaun ob\'s wos Neis gibt',
        'update_check_error_confirm': 'Ka Verbindung zum GitHub. Mogst du s\'manuelle Update sengn?',
        'view_releases': 'Releases oschaugn',
        'no_update_available': 'Koa Update verfügbar. Du host scho die neieste Version ({version}).',
        'update_available': 'Es gibt wos Neis!\n\nJetzige Version: {current}\nNeie Version: {latest}\n\nWuist des gleich installiern?',
        'update_error': 'Update-Fehla',
        'manual_update': 'Manuelles Update',
        'auto_update': 'Automatisches Update',
        'confirm_auto_update': 'Mogst wirklich automatisch updaten? D\'Uhr startet danoch nei.',
        'check_updates': 'Noch wos Neis?',
        'update_options': 'Update-Möglichkeiten',
        'manual_update_options': 'Manuelles Update',
        
        // Update translations
        'current_version': 'Aktuäi Version',
        'latest_version': 'Neieste Version',
        'update_firmware': 'Firmware updaten',
        'update_filesystem': 'Dateisystem updaten',
        'retry_update': 'Nomoi probiern',
        'update_files_missing': 'Update-Datein ned gfundn. Bitte schau auf da Releases-Seitn für an manuelln Download.',
        'update_success': "Update fertig! S'Gerät startet nei.",
    },
    'bar': {
        // ... existing code ...
        'update_check_error': 'Konnt ned schaun ob\'s wos Neis gibt',
        'update_check_error_confirm': 'Ka Verbindung zum GitHub. Mogst du s\'manuelle Update sengn?',
        'view_releases': 'GitHub Releases oschaun',
        // ... existing code ...
    }
};

export default translations; 
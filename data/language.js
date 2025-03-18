import translations from '/translations.js';

class LanguageManager {
    constructor() {
        this.currentLang = localStorage.getItem('hollowclock_lang') || 'de';
        this.translations = translations;
    }

    // Initialisiert den Sprachumschalter und wendet die gespeicherte Sprache an
    init() {
        console.log('Initialisiere Spracheinstellungen...');
        this.updateLanguageSwitch();
        this.applyTranslations();
        
        // Event-Listener für den Sprachumschalter
        document.getElementById('language-switch').addEventListener('change', (e) => {
            this.setLanguage(e.target.value);
        });
    }
    
    // Aktualisiert den Sprachumschalter mit der aktuellen Sprache
    updateLanguageSwitch() {
        const languageSwitch = document.getElementById('language-switch');
        if (languageSwitch) {
            languageSwitch.value = this.currentLang;
        }
    }

    // Wechselt die Sprache und speichert die Auswahl im localStorage
    setLanguage(lang) {
        console.log(`Wechsle Sprache zu ${lang}`);
        if (this.translations[lang]) {
            this.currentLang = lang;
            localStorage.setItem('hollowclock_lang', lang);
            this.applyTranslations();
        } else {
            console.error(`Sprache ${lang} nicht gefunden!`);
        }
    }

    // Wendet Übersetzungen auf alle Elemente mit dem data-i18n-Attribut an
    applyTranslations() {
        const elements = document.querySelectorAll('[data-i18n]');
        elements.forEach(el => {
            const key = el.getAttribute('data-i18n');
            if (this.translations[this.currentLang][key]) {
                el.innerHTML = this.translations[this.currentLang][key];
            }
        });
        
        // Passt den Titel der Seite an
        document.title = this.translations[this.currentLang]['title'];
        
        // Signalisiere, dass die Sprache gewechselt wurde (für andere Komponenten)
        document.dispatchEvent(new CustomEvent('languageChanged', { 
            detail: { language: this.currentLang }
        }));
    }

    // Gibt eine Übersetzung für einen bestimmten Schlüssel zurück
    translate(key) {
        if (this.translations[this.currentLang][key]) {
            return this.translations[this.currentLang][key];
        }
        console.warn(`Übersetzung für "${key}" nicht gefunden!`);
        return key;
    }
}

export default new LanguageManager(); 
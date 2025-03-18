import os
import subprocess
import shutil
import sys

def create_data_dir():
    """Erstellt das data-Verzeichnis, falls es nicht existiert."""
    if not os.path.exists('data'):
        os.makedirs('data')
        print("data-Verzeichnis erstellt")

def create_directories():
    """Erstellt die benötigten Unterverzeichnisse."""
    dirs = ['data/css', 'data/js']
    for dir in dirs:
        if not os.path.exists(dir):
            os.makedirs(dir)
            print(f"{dir}-Verzeichnis erstellt")

def check_platformio():
    """Überprüft, ob PlatformIO installiert ist."""
    try:
        subprocess.run(['pio', '--version'], capture_output=True, check=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False

def build_filesystem():
    """Erstellt das LittleFS-Dateisystem und lädt es hoch."""
    print("\nErstelle LittleFS-Image...")
    result = subprocess.run(['pio', 'run', '--target', 'buildfs'], capture_output=True, text=True)
    
    if result.returncode != 0:
        print("Fehler beim Erstellen des Dateisystems:")
        print(result.stderr)
        return False
    
    print("LittleFS-Image erstellt")
    
    print("\nLade Dateisystem hoch...")
    result = subprocess.run(['pio', 'run', '--target', 'uploadfs'], capture_output=True, text=True)
    
    if result.returncode != 0:
        print("Fehler beim Hochladen des Dateisystems:")
        print(result.stderr)
        return False
    
    print("Dateisystem erfolgreich hochgeladen!")
    return True

def main():
    """Hauptfunktion"""
    print("=== LittleFS Upload Tool ===")
    
    # PlatformIO überprüfen
    if not check_platformio():
        print("Fehler: PlatformIO ist nicht installiert!")
        print("Bitte installieren Sie PlatformIO CLI: pip install platformio")
        sys.exit(1)
    
    # Verzeichnisse erstellen
    create_data_dir()
    create_directories()
    
    # Dateisystem erstellen und hochladen
    if build_filesystem():
        print("\nProzess abgeschlossen!")
    else:
        print("\nFehler beim Hochladen des Dateisystems")
        sys.exit(1)

if __name__ == "__main__":
    main() 
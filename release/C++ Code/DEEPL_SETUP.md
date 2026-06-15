# DeepL API Integration – Übersetzung von EXIF-Tags

## Funktionsübersicht

Diese Erweiterung fügt die Möglichkeit hinzu, **EXIF-Tags ins Deutsche zu übersetzen**, ohne die bestehende Mapping-Struktur zu verändern und ohne die Originaldatei zu modifizieren.

## Neue Dateien

* `deepl.hpp` – Deklarationen der DeepL-API-Funktionen
* `deepl.cpp` – Implementierung der DeepL-API

## Abhängigkeiten

### 1. CURL-Bibliothek

Wird für HTTP-Anfragen an die DeepL API verwendet:

```bash
# Windows (vcpkg)
vcpkg install curl:x64-windows
```

### 2. nlohmann/json Bibliothek

Wird für die Verarbeitung von JSON-Daten verwendet:

```bash
# Windows (vcpkg)
vcpkg install nlohmann-json:x64-windows
```

## Kompilierung

### Mit MinGW/g++

```bash
g++ -std=c++17 -o exifprogramm.exe \
    exifprogramm.cpp \
    exiv2_funk.cpp \
    mapping.cpp \
    valid.cpp \
    deepl.cpp \
    -lexiv2 \
    -lcurl \
    -lstdc++fs
```

### Mit Visual Studio

1. Include-Pfade für CURL und nlohmann/json hinzufügen
2. Im Linker folgende Bibliotheken einbinden:

   * `libcurl.lib`
   * `exiv2.lib`
3. Projekt kompilieren

## Verwendung

### 1. DeepL API-Schlüssel beziehen

Registrieren Sie sich unter:

https://www.deepl.com/docs-api

und erstellen Sie einen API-Schlüssel.

### 2. Programm starten

Beim Start erscheint folgende Abfrage:

```text
--- DeepL API Konfiguration ---
Möchten Sie DeepL Übersetzung aktivieren? (j/n):
```

Geben Sie `j` ein und fügen Sie Ihren API-Schlüssel ein.

### 3. Übersetzungsfunktion verwenden

Verfügbare Befehle:

* `lesen` – EXIF-Daten anzeigen
* `schreiben` – EXIF-Daten bearbeiten
* `loeschen` – EXIF-Daten löschen
* `export` – EXIF-Daten exportieren
* `uebersetzen` – EXIF-Tags ins Deutsche übersetzen
* `zuruck` – Zur Dateiauswahl zurückkehren
* `exit` – Programm beenden

### 4. Beispiel

Im Bearbeitungsmenü:

```text
> uebersetzen
```

Ausgabe:

```text
EXIF-Tags werden ins Deutsche übersetzt...
Übersetzung abgeschlossen.

--- Übersetzte EXIF-Tags ---

Exif.Image.Make (Hersteller) = Canon
Exif.Image.Model (Kameramodell) = EOS 90D
Exif.Image.ImageWidth (Bildbreite) = 6000
...
```

Die Übersetzungen werden ausschließlich zur Anzeige verwendet. Die Originaldatei bleibt unverändert.

## Eigenschaften

✅ Unterstützung der deutschen Sprache

✅ Keine Änderungen an der Originaldatei

✅ Übersetzung von EXIF-Tags zur Anzeige

✅ Nutzung vorhandener Mapping-Einträge

✅ Optionaler Einsatz der DeepL API

✅ Fehlerbehandlung bei API-Problemen

## Fehlerbehebung

### „DeepL API-Schlüssel nicht gesetzt“

Stellen Sie sicher, dass beim Programmstart ein gültiger API-Schlüssel eingegeben wurde.

### „DeepL-Verbindung fehlgeschlagen“

* Internetverbindung prüfen
* API-Schlüssel überprüfen
* DeepL-Dienststatus kontrollieren
* Dokumentation unter:
  https://support.deepl.com

### CURL- oder JSON-Bibliothek nicht gefunden

Installieren Sie die benötigten Abhängigkeiten:

```bash
vcpkg install curl:x64-windows nlohmann-json:x64-windows
```

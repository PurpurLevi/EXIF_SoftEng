#include <exiv2/exiv2.hpp>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <fstream>

using namespace std;

//*Texabgleich
string toLower(string text) {

    transform(
        text.begin(),
        text.end(),
        text.begin(),
        ::tolower
    );

    return text;
}

//*Datenausgabe
void printExifData(
    const Exiv2::ExifData& exifData,
    const map<string, string>& exifToUser
    ) {
    cout << "\nEXIF DATEN\n";
    cout << "===================================\n";

    for (const auto& entry : exifData) {

        string key = entry.key();
        string value = entry.value().toString();

        if (exifToUser.find(key) != exifToUser.end()) {

            cout << exifToUser.at(key)
                 << " = "
                 << value
                 << "\n";
        }
        else {

            cout << key
                 << " = "
                 << value
                 << "\n";
        }
    }
}

//*Datenschreiben
void setExif(
    const string& filename,
    const string& key,
    const string& value
    ) {

    auto image = Exiv2::ImageFactory::open(filename);

    if (!image) {
        cout << "Datei konnte nicht geöffnet werden.\n";
        return;
    }

    image->readMetadata();

    Exiv2::ExifData& exifData = image->exifData();

    exifData[key] = value;

    image->setExifData(exifData);

    image->writeMetadata();

    cout << "EXIF Wert geschrieben.\n";
}

//*Datenlöschen
void removeExif(
    const string& filename,
    const string& key
    ) {

    auto image = Exiv2::ImageFactory::open(filename);

    if (!image) {
        cout << "Datei konnte nicht geöffnet werden.\n";
        return;
    }

    image->readMetadata();

    Exiv2::ExifData& exifData = image->exifData();

    auto pos = exifData.findKey(Exiv2::ExifKey(key));

    if (pos == exifData.end()) {

        cout << "Tag nicht gefunden.\n";
        return;
    }

    exifData.erase(pos);

    image->setExifData(exifData);

    image->writeMetadata();

    cout << "EXIF Tag gelöscht.\n";
}

//*Datenexport
void exportExifToFile(
    const string& filename,
    const Exiv2::ExifData& exifData,
    const map<string, string>& exifToUser
    ){
    ofstream file("exif_export.txt");

    if (!file.is_open()) {
        cout << "Datei konnte nicht erstellt werden.\n";
        return;
    }

    for (const auto& entry : exifData) {

        string key = entry.key();
        string value = entry.value().toString();

        string label;

        if (exifToUser.find(key) != exifToUser.end()) {
            label = exifToUser.at(key);
        } else {
            label = key;
        }

        file << label << " = " << value << "\n";
    }

    file.close();

    cout << "Export abgeschlossen: exif_export.txt\n";
}

//!Hauptfunktion
int main() {
while (true) { //*Schleife über gesamte Programm
    
    string filename;

    cout << "\nBilddatei Verzeichnis eingeben\n"; //*Eingabe von Bildverzeichnis
    cout << "(oder 'exit' zum Beenden):\n";
        
    getline(cin, filename);

    auto image = Exiv2::ImageFactory::open(filename); //*Prüfung und Öffnung von Dateien

    if (!image) { //*Feler: Datei wurde nicht geöffnet
        cout << "Datei konnte nicht geöffnet werden.\n";
        continue;
    }

    image->readMetadata(); //*Lesen von Exif Daten

    Exiv2::ExifData& exifData = image->exifData(); //*Speichern von Exif Daten in Buffer

    if (exifData.empty()) { //*Fehler: Daten wurden nicht gefunden
        cout << "Keine EXIF Daten gefunden.\n";
        continue;
    }
    
    //?Angezeigte Daten
    map<string, string> exiftoUser = {
        {"Exif.Image.Make", "Hersteller"},
        {"Exif.Image.Model", "Kameramodell"},
        {"Exif.Image.Orientation", "Orientierung"},
        {"Exif.Image.XResolution", "X Aufloesung"},
        {"Exif.Image.YResolution", "Y Aufloesung"},
        {"Exif.Image.ResolutionUnit", "Aufloesungseinheit"},
        {"Exif.Image.Software", "Software"},
        {"Exif.Image.DateTime", "Datum"},
        {"Exif.Image.YCbCrPositioning", "YCbCr Positionierung"},
        {"Exif.Image.ExifTag", "EXIF Tag"},
        {"Exif.Photo.0x0100", "Bildbreite"},
        {"Exif.Photo.0x0101", "Bildhoehe"},
        {"Exif.Photo.ExposureTime", "Belichtungszeit"},
        {"Exif.Photo.FNumber", "Blende"},
        {"Exif.Photo.ExposureProgram", "Belichtungsprogramm"},
        {"Exif.Photo.ISOSpeedRatings", "ISO"},
        {"Exif.Photo.ExifVersion", "EXIF Version"},
        {"Exif.Photo.DateTimeOriginal", "Aufnahmedatum"},
        {"Exif.Photo.DateTimeDigitized", "Digitalisiert"},
        {"Exif.Photo.ComponentsConfiguration", "Komponenten"},
        {"Exif.Photo.ShutterSpeedValue", "Verschlusszeit"},
        {"Exif.Photo.ApertureValue", "Blendenwert"},
        {"Exif.Photo.BrightnessValue", "Helligkeit"},
        {"Exif.Photo.ExposureBiasValue", "Belichtungskorrektur"},
        {"Exif.Photo.MaxApertureValue", "Max Blende"},
        {"Exif.Photo.MeteringMode", "Messmodus"},
        {"Exif.Photo.Flash", "Blitz"},
        {"Exif.Photo.FocalLength", "Brennweite"},
        {"Exif.Photo.SubSecTime", "Millisekunden"},
        {"Exif.Photo.SubSecTimeOriginal", "Original Millisekunden"},
        {"Exif.Photo.SubSecTimeDigitized", "Digitalisiert Millisekunden"},
        {"Exif.Photo.ColorSpace", "Farbraum"},
        {"Exif.Photo.WhiteBalance", "Weissabgleich"},
        {"Exif.Photo.FocalLengthIn35mmFilm", "35mm Brennweite"},
        {"Exif.Photo.SceneCaptureType", "Szenentyp"},
        {"Exif.Photo.ImageUniqueID", "Bild ID"}
    };

    //?Eingegebene Daten
    map<string, string> userToExif = {
        {"hersteller", "Exif.Image.Make"},
        {"kameramodell", "Exif.Image.Model"},
        {"orientierung", "Exif.Image.Orientation"},
        {"xaufloesung", "Exif.Image.XResolution"},
        {"yaufloesung", "Exif.Image.YResolution"},
        {"aufloesungseinheit", "Exif.Image.ResolutionUnit"},
        {"software", "Exif.Image.Software"},
        {"datum", "Exif.Image.DateTime"},
        {"ycbcr positionierung", "Exif.Image.YCbCrPositioning"},
        {"exif tag", "Exif.Image.ExifTag"},
        {"bildbreite", "Exif.Photo.0x0100"},
        {"bildhoehe", "Exif.Photo.0x0101"},
        {"belichtungszeit", "Exif.Photo.ExposureTime"},
        {"blende", "Exif.Photo.FNumber"},
        {"belichtungsprogramm", "Exif.Photo.ExposureProgram"},
        {"iso", "Exif.Photo.ISOSpeedRatings"},
        {"exif version", "Exif.Photo.ExifVersion"},
        {"aufnahmedatum", "Exif.Photo.DateTimeOriginal"},
        {"digitalisiert", "Exif.Photo.DateTimeDigitized"},
        {"komponenten", "Exif.Photo.ComponentsConfiguration"},
        {"verschlusszeit", "Exif.Photo.ShutterSpeedValue"},
        {"blendenwert", "Exif.Photo.ApertureValue"},
        {"helligkeit", "Exif.Photo.BrightnessValue"},
        {"belichtungskorrektur", "Exif.Photo.ExposureBiasValue"},
        {"maxblende", "Exif.Photo.MaxApertureValue"},
        {"messmodus", "Exif.Photo.MeteringMode"},
        {"blitz", "Exif.Photo.Flash"},
        {"brennweite", "Exif.Photo.FocalLength"},
        {"millisekunden", "Exif.Photo.SubSecTime"},
        {"originalmillisekunden", "Exif.Photo.SubSecTimeOriginal"},
        {"digitalisiert millisekunden", "Exif.Photo.SubSecTimeDigitized"},
        {"farbraum", "Exif.Photo.ColorSpace"},
        {"weissabgleich", "Exif.Photo.WhiteBalance"},
        {"35mmbrennweite", "Exif.Photo.FocalLengthIn35mmFilm"},
        {"szenentyp", "Exif.Photo.SceneCaptureType"},
        {"bildid", "Exif.Photo.ImageUniqueID"}
    };

    cout << "\nEXIF DATEN\n"; 
    cout << "===================================\n";
    printExifData(exifData, exiftoUser); //*Datenausgabe

    //*Aktion wählen
    string command;

    while(true){ //*Schleife über Datei bearbeitung
        cout << "\nBefehl eingeben:\n";
        cout << "lesen / schreiben / loeschen / export / zuruck / exit\n";

        getline(cin, command);
        command = toLower(command); //Prüft eingegebenen Text auf Großbuchstaben
            
        if (command == "zuruck") //Gang zurück zum Datei Auswahl
            break;

        if (command == "exit") //Beendet das gesamte Programm
            return 0;

        //!Block zur Bearbeitung von Daten
        try { 

            if (command == "lesen"){ //*Daten werden gedruckt
                printExifData(exifData, exiftoUser);
            }
            else if (command == "schreiben") { //*Daten werden umgeschrieben
                string key;
                string value;

                cout << "EXIF Name:\n";
                getline(cin, key);
                key = toLower(key);

                cout << "Neue Wert:\n";
                getline(cin, value);

                if (userToExif.find(key) != userToExif.end()) 
                {
                    setExif(filename, userToExif[key], value);
                    image->readMetadata();
                }
                else 
                    cout << "Unbekannter EXIF Name.\n";
            }
            else if (command == "loeschen") { //*Daten werden gelöscht
                string key;

                cout << "EXIF Name:\n";
                getline(cin, key);
                key = toLower(key);
                if (userToExif.find(key) != userToExif.end()) 
                {
                    removeExif(filename, userToExif[key]);
                    image->readMetadata();
                }
                else 
                    cout << "Unbekannter EXIF Name.\n"; 
            }
            else if(command == "export"){
                exportExifToFile(filename, exifData, exiftoUser);
            }
            else
                cout << "Unbekannter Befehl.\n";

            }
        catch (const Exiv2::Error& e) { //*Fehler von Bibliothek

            cout << "\nEXIV2 FEHLER:\n";
            cout << e.what() << "\n";
        }
        catch (const exception& e) { //*Allgemeine Fehler
            cout << "\nALLGEMEINER FEHLER:\n"; 
            cout << e.what() << "\n";
        }
    }
}

cout << "\nProgramm beendet.\n";

return 0;
}
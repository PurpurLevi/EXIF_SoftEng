#include <exiv2/exiv2.hpp> //*Exif Bibliothek
#include <iostream>
#include <string>
#include <fstream> //* Bibliothek für export
#include <filesystem> //*Bibliothek zur Kopieerstellung
#include "mapping.hpp" //*Übersetzung durch Mapping
#include "valid.hpp" //*Validierung
#include "exiv2_funk.hpp" //*Exif funktionen: shreiben, lesen, löschen
#include "deepl.hpp" //*Überstezung durch DeepL

using namespace std;
namespace fs = filesystem;

//*Zeigt die Bilder in Ordner beim eingabe relativer Pfad an
void showImages(const fs::path& currentDir)
{
    int count = 0;

    for (const auto& entry : fs::directory_iterator(currentDir))
    {
        if (!entry.is_regular_file())
            continue;

        string ext = entry.path().extension().string();

        transform(ext.begin(), ext.end(),
                  ext.begin(), ::tolower);

        if (ext == ".jpg"  || //Unteschtüzbare Formate
            ext == ".jpeg" ||
            ext == ".png"  ||
            ext == ".tif"  ||
            ext == ".tiff" ||
            ext == ".CA2")
        {
            count++;
            cout << count << ". "<< entry.path().filename().string()<< '\n';
        }
    }
}

//*Erzeugt Kopie von Datei in selber Ordner
string createEditableCopy(const string& originalPath) {

    fs::path original(originalPath);

    fs::path copyPath =
        original.stem().string() + "_kopie" + original.extension().string();

    fs::copy_file(
        original,
        copyPath,
        fs::copy_options::overwrite_existing
    );

    return copyPath.string();
}

//*Datenexport in TXT oder CSV
void exportExifToFile(
    const Exiv2::ExifData& exifData,
    const map<string, string>& exifToUser
    ){

    string name;
    string format;

    ofstream file;

    cout << "Im welchen Format Exportieren?\n";
    cout << "TXT / CSV\n";

    getline(cin, format);

    format = toLower(format);

    if (format == "txt") {
        cout << "Name des .txt Datei eingeben:\n";
        getline(cin, name);
        file.open(name + ".txt");
    }
    else if (format == "csv") {
        cout << "Name des .csv Datei eingeben:\n";
        getline(cin, name);
        file.open(name + ".csv");
    }
    else {
        cout << "Unbekanntes Format.\n";
        return;
    }

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
        }
        else {
            label = key;
        }

        if (format == "csv")
            file << label << ";" << value << "\n";
        else
            file << label << " = " << value << "\n";
    }

    file.close();

    cout << "Export abgeschlossen: " << name << "." << format << "\n";
}

//!Konfiguriert DeepL API zu Programmbeginn
void setupDeepL() {
    cout << "\n--- DeepL API Konfiguration ---\n";
    cout << "Möchten Sie DeepL Übersetzung aktivieren? (j/n):\n";
    
    string choice;
    getline(cin, choice);
    choice = toLower(choice);
    
    if (choice == "j") {
        string apiKey;
        cout << "DeepL API-Schlüssel eingeben:\n";
        getline(cin, apiKey);
        
        setDeepLApiKey(apiKey);
        
        if (testDeepLConnection()) {
            cout << "DeepL erfolgreich konfiguriert!\n";
        }
        else {
            cout << "Warnung: DeepL-Verbindung fehlgeschlagen.\n";
        }
    }
}

//!Hauptfunktion
int main() {
    //* Stellt die Konsolencodierung auf UTF-8 um (Windows)
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif
    
    //* Aktiviert die UTF-8-Lokalisierung
    locale::global(locale(""));
    
    //* DeepL Setup
    setupDeepL();

    fs::path currentDir = fs::current_path();
    while (true) { //*Schleife über gesamte Programm

        cout << "\nAktueller Ordner:\n";
        cout << currentDir << "\n\n";

        showImages(currentDir); //*Funktion zur Bildanzeige in Ordner

        string filename;

        cout << "\nBilddatei oder Ordner Verzeichnis eingeben\n";
        cout << "(.. eine Ebene nach oben oder 'exit' zum Beenden):\n";
        
        getline(cin, filename);

        filename = removeQuotes(filename); //*Löscht die Zitaten von Pfad

        if (filename == "exit")
            return 0;

        fs::path path(filename);

        //* Relative Pfade erlauben

        if (filename == "..")
        {
            currentDir = currentDir.parent_path();
            continue;
        }

        if (path.is_relative()) {
            path = currentDir / path;
        }

        if (fs::is_directory(path))
        {
            currentDir = path;
            continue;
        }

        shared_ptr<Exiv2::Image> image;
        
        cout << "Pfad: " << path << '\n';

        if (!fs::exists(path)) //*Prüft und erlaubt Dateiname ohne Format einzugeben
        {
            vector<string> exts = {
                ".jpg",
                ".jpeg",
                ".png",
                ".tif",
                ".tiff",
                ".CA2"
            };

            bool found = false;

            for (const auto& ext : exts)
            {
                fs::path testPath = path;
                testPath += ext;

                if (fs::exists(testPath))
                {
                    path = testPath;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                cout << "Datei existiert nicht.\n";
                continue;
            }
        }

        try{
            image = Exiv2::ImageFactory::open(path.string());
        }
        catch (const Exiv2::Error& e) {
            cout << "Datei konnte nicht geoffnet werden (Exiv2-Fehler):\n"<< e.what() << "\n";
            continue; // zurück zur Dateiauswahl
        }  
        if (image.get() == 0) {
            cout << "Datei konnte nicht geoffnet werden.\n";
            continue;
        } 

        //* Macht eine Kopie von Origilandatei
        string backup = createEditableCopy(path.string());

        image->readMetadata(); //*Lesen von Exif Daten
        
        Exiv2::ExifData& exifData = image->exifData(); //*Speichern von Exif Daten in Buffer

        if (exifData.empty()) { //*Fehler: Daten wurden nicht gefunden
            cout << "Keine EXIF Daten gefunden.\n";
            continue;
        }

        if (isDeepLEnabled()) { //*Wenn Deepl gewählt war: Übersetzt

            map<string, string> translatedLabels =
                translateExifLabelsForData(
                    exifData,
                    exiftoUser
                );

            printExifData(
                exifData,
                exiftoUser,
                &translatedLabels
            );
        }
        else {

            printExifData(
                exifData,
                exiftoUser,
                nullptr
            );
        }

        //*Aktion wählen
        string command;

        while(true){ //*Schleife über Datei bearbeitung
            cout << "\nBefehl eingeben:\n";
            cout << "lesen(l) / schreiben(s) / loeschen(lo) / export(ex) / zuruck(z) / exit\n";

            getline(cin, command);
            command = toLower(command); //Prüft eingegebenen Text auf Großbuchstaben
                
            if (command == "zuruck" or command == "z") //Gang zurück zum Datei Auswahl
                break;

            if (command == "exit") //Beendet das gesamte Programm
                return 0;

            //!Block zur Bearbeitung von Daten
            try { 

                if (command == "lesen" or command == "l"){ //*Daten werden gedruckt
                    printExifData(exifData, exiftoUser);
                }
                else if (command == "schreiben" or command == "s") { //*Daten werden umgeschrieben
                    string key;
                    string value;

                    cout << "EXIF Name:\n";
                    getline(cin, key);

                    string lowerKey = toLower(key);

                    if(userToExif.find(lowerKey) != userToExif.end()){ //Prüft nach Key in Map
                        
                        cout << "Neue Wert:\n";
                        getline(cin, value);

                        setExif(*image, userToExif[lowerKey], value);
                        image->readMetadata();
                    }
                    else{
                        try { 
                            Exiv2::ExifKey exifKey(key); //Wenn kein Key in Map, sucht nach Exifkey
                            cout << "Neue Wert:\n";
                            getline(cin, value);

                            if (fieldType[key] == "date")
                                value = normaldate(value);

                            if (fieldType[key] == "digits")
                                value = digits(value);

                            setExif(*image, key, value);
                            image->readMetadata();
                        }
                        catch (...) {
                            cout << "Unbekannter EXIF Name.\n";
                        }
                    } 
                }
                else if (command == "loeschen" or command == "lo") { //*Daten werden gelöscht
                    string key;

                    cout << "EXIF Name:\n";
                    getline(cin, key);

                    string lowerKey = toLower(key);
                    
                    if (userToExif.find(lowerKey) != userToExif.end()) 
                    {
                        removeExif(*image, userToExif[lowerKey]);
                        image->readMetadata();
                    }
                    else{
                        try { 
                            Exiv2::ExifKey exifKey(key);

                            removeExif(*image, key);
                            image->readMetadata();
                        }
                        catch (...) {
                            cout << "Unbekannter EXIF Name.\n";
                        }
                    } 
                }
                else if(command == "export" or command == "ex"){ //*exportiert die Daten
                    exportExifToFile(exifData, exiftoUser);
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

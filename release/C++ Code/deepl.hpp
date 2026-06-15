#ifndef DEEPL_HPP
#define DEEPL_HPP

#include <string>          //* String-Datentyp 
#include <map>             //* Schlüssel-Wert Zuordnung 
#include <iostream>        //* Ein- und Ausgabe 
#include <exiv2/exiv2.hpp> //* EXIF Bibliothek

//* Speichert den DeepL API-Schlüssel
void setDeepLApiKey(const std::string& apiKey);

//* Prüft ob DeepL aktiviert wurde
bool isDeepLEnabled();

//* Übersetzt einen Text nach Deutsch
std::string translateToGerman(
    const std::string& text
);

//* Übersetzt alle EXIF-Tags der aktuellen Datei
std::map<std::string,std::string>
translateExifLabelsForData(
    const Exiv2::ExifData& exifData,
    const std::map<std::string,std::string>& exifToUser
);

//* Übersetzt EXIF-Werte
std::map<std::string,std::string>
translateExifValues(
    const Exiv2::ExifData& exifData
);

//* Testet die Verbindung zur DeepL API
bool testDeepLConnection();

#endif
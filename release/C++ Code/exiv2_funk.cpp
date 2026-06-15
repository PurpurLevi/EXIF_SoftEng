#include "exiv2_funk.hpp"
#include <iostream>

void printExifData(
    const Exiv2::ExifData& exifData,
    const std::map<std::string, std::string>& exifToUser,
    const std::map<std::string, std::string>* translatedLabels
    ) {
    std::cout << "\nEXIF DATEN\n";
    std::cout << "===================================\n";

    for (const auto& entry : exifData) {

        std::string key = entry.key();
        std::string value = entry.value().toString();

        if (translatedLabels != nullptr &&
            translatedLabels->find(key) != translatedLabels->end()) {

            std::cout
                << key
                << " ("
                << translatedLabels->at(key)
                << ") = "
                << value
                << "\n";
        }
        else if (exifToUser.find(key) != exifToUser.end()) {

            std::cout
                << key
                << " ("
                << exifToUser.at(key)
                << ") = "
                << value
                << "\n";
        }
        else {
            std::cout
                << key
                << " = "
                << value
                << "\n";
        }
    }
}

//*Datenschreiben
void setExif(
    Exiv2::Image& image,
    const string& key,
    const string& value
    ) {

    image.readMetadata();

    Exiv2::ExifData& exifData = image.exifData();

    exifData[key] = value;

    image.setExifData(exifData);

    image.writeMetadata();

    std::cout << "EXIF Wert geschrieben.\n";
}

//*Datenlöschen
void removeExif(
    Exiv2::Image& image,
    const string& key
    ) {

    image.readMetadata();

    Exiv2::ExifData& exifData = image.exifData();

    auto pos = exifData.findKey(Exiv2::ExifKey(key));

    if (pos == exifData.end()) {

        std::cout << "Tag nicht gefunden.\n";
        return;
    }

    exifData.erase(pos);

    image.setExifData(exifData);

    image.writeMetadata();

    cout << "EXIF Tag gelöscht.\n";
}
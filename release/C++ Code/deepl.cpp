#include "deepl.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cctype>

//* JSON Bibliothek
using json = nlohmann::json;

//* Globaler DeepL API-Schlüssel
std::string DEEPL_API_KEY = "";

//* DeepL API URL
std::string DEEPL_API_URL = "https://api-free.deepl.com/v1/translate";


//* CURL Callback-Funktion zum Speichern der Antwort
static size_t WriteCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    std::string* s
) {
    s->append(
        static_cast<char*>(contents),
        size * nmemb
    );

    return size * nmemb;
}


//* Speichert den DeepL API-Schlüssel
void setDeepLApiKey(const std::string& apiKey) {

    DEEPL_API_KEY = apiKey;

}


//* Prüft ob DeepL aktiviert wurde
bool isDeepLEnabled() {

    return !DEEPL_API_KEY.empty();

}


//* Übersetzt einen Text nach Deutsch
std::string translateToGerman(const std::string& text)
{
    if (DEEPL_API_KEY.empty()) {
        std::cerr << "Fehler: Kein DeepL API-Schlüssel vorhanden.\n";
        return text;
    }

    CURL* curl = curl_easy_init();

    if (!curl) {
        std::cerr << "Fehler: CURL initialization failed.\n";
        return text;
    }

    try {
        //* Erstellt JSON Anfrage
        json requestBody = {
            {"text", {text}},
            {"target_lang", "DE"}
        };

        std::string postData = requestBody.dump();

        //* Antwort der API
        std::string response;

        //* HTTP Header vorbereiten
        struct curl_slist* headers = nullptr;

        headers = curl_slist_append(
            headers,
            "Content-Type: application/json"
        );

        headers = curl_slist_append(
            headers,
            "User-Agent: exifprogramm/1.0"
        );

        std::string authHeader =
            "Authorization: DeepL-Auth-Key " + DEEPL_API_KEY;

        headers = curl_slist_append(
            headers,
            authHeader.c_str()
        );

        //* CURL Optionen setzen
        curl_easy_setopt(
            curl,
            CURLOPT_URL,
            DEEPL_API_URL.c_str()
        );

        curl_easy_setopt(
            curl,
            CURLOPT_HTTPHEADER,
            headers
        );

        curl_easy_setopt(
            curl,
            CURLOPT_POSTFIELDS,
            postData.c_str()
        );

        curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            WriteCallback
        );

        curl_easy_setopt(
            curl,
            CURLOPT_WRITEDATA,
            &response
        );

        curl_easy_setopt(
            curl,
            CURLOPT_TIMEOUT,
            10L
        );

        //* Debug-Ausgabe deaktiviert
        curl_easy_setopt(
            curl,
            CURLOPT_VERBOSE,
            0L
        );

        //* SSL Prüfung deaktiviert für MSYS2 Windows Umgebung
        curl_easy_setopt(
            curl,
            CURLOPT_SSL_VERIFYPEER,
            0L
        );

        curl_easy_setopt(
            curl,
            CURLOPT_SSL_VERIFYHOST,
            0L
        );

        //* Anfrage senden
        CURLcode res = curl_easy_perform(curl);

        //* HTTP Status lesen
        long httpCode = 0;

        curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &httpCode
        );

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "\n*** CURL Fehler ***\n";
            std::cerr << curl_easy_strerror(res) << "\n";
            return text;
        }

        if (httpCode != 200) {
            std::cerr << "\n*** API Fehler ***\n";
            std::cerr << "HTTP Status: " << httpCode << "\n";
            std::cerr << "Antwort: " << response << "\n";
            return text;
        }

        //* JSON Antwort auswerten
        json responseJson = json::parse(response);

        if (
            responseJson.contains("translations") &&
            responseJson["translations"].size() > 0
        ) {
            return responseJson["translations"][0]["text"];
        }

        std::cerr << "Fehler: Ungültige API-Antwort.\n";
        return text;

    }
    catch (const std::exception& e) {
        std::cerr << "DeepL Anfrage Fehler: " << e.what() << "\n";

        curl_easy_cleanup(curl);

        return text;
    }
}


//* Wandelt EXIF-Key in lesbaren Text um
static std::string makeReadableExifName(
    const std::string& key
) {
    std::string name = key;

    //* Entfernt alles vor dem letzten Punkt
    size_t pos = name.rfind('.');

    if (pos != std::string::npos) {
        name = name.substr(pos + 1);
    }

    std::string result;

    //* Fügt Leerzeichen zwischen Klein- und Großbuchstaben ein
    for (size_t i = 0; i < name.size(); ++i) {

        char c = name[i];

        if (
            i > 0 &&
            std::isupper(static_cast<unsigned char>(c)) &&
            std::islower(static_cast<unsigned char>(name[i - 1]))
        ) {
            result += ' ';
        }

        result += c;
    }

    return result;
}


//* Übersetzt alle EXIF-Tags der aktuellen Datei mit DeepL
std::map<std::string, std::string> translateExifLabelsForData(
    const Exiv2::ExifData& exifData,
    const std::map<std::string, std::string>& exifToUser
) {
    std::map<std::string, std::string> labels;

    std::cout << "Übersetzung aller EXIF-Tags mit DeepL...\n";

    for (const auto& entry : exifData) {

        std::string key = entry.key();

        std::string readableName =
            makeReadableExifName(key);

        if (isDeepLEnabled()) {

            std::string readableName =
                makeReadableExifName(key);

            std::cout
                << "[DeepL] "
                 << readableName
                << " -> ";

            std::string translated =
                translateToGerman(readableName);

            std::cout << translated << "\n";

            labels[key] = translated;
        }
        else {
            if (exifToUser.find(key) != exifToUser.end()) {
                labels[key] = exifToUser.at(key);
            }
        }
    }

    std::cout << "Übersetzung aller EXIF-Tags abgeschlossen.\n";

    return labels;
}


//* Übersetzt EXIF-Werte
std::map<std::string, std::string> translateExifValues(
    const Exiv2::ExifData& exifData
) {
    std::map<std::string, std::string> translatedValues;

    std::cout << "Übersetzung der EXIF-Werte in Deutsch...\n";

    for (const auto& entry : exifData) {

        std::string key = entry.key();
        std::string value = entry.value().toString();

        if (value.empty()) {
            continue;
        }

        std::string translated =
            translateToGerman(value);

        if (translated != value) {
            translatedValues[key] = translated;
        }
    }

    std::cout << "EXIF-Werte Übersetzung abgeschlossen.\n";

    return translatedValues;
}


//* Testet die Verbindung zur DeepL API
bool testDeepLConnection() {

    if (DEEPL_API_KEY.empty()) {
        std::cout << "Fehler: DeepL API-Schlüssel ist nicht gesetzt.\n";
        return false;
    }

    std::string testPhrase = "Hello";

    std::string testResult =
        translateToGerman(testPhrase);

    if (
        !testResult.empty() &&
        testResult != testPhrase
    ) {
        std::cout << "DeepL Verbindung erfolgreich!\n";
        return true;
    }

    std::cout << "DeepL Verbindung fehlgeschlagen.\n";
    return false;
}
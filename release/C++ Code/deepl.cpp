#include "deepl.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cctype>

using namespace std;

//* JSON Bibliothek
using json = nlohmann::json;

//* Globaler DeepL API-Schlüssel
string DEEPL_API_KEY = "";

//* DeepL API URL
string DEEPL_API_URL = "https://api-free.deepl.com/v1/translate";


//* CURL Callback-Funktion zum Speichern der Antwort
static size_t WriteCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    string* s
) {
    s->append(
        static_cast<char*>(contents),
        size * nmemb
    );

    return size * nmemb;
}


//* Speichert den DeepL API-Schlüssel
void setDeepLApiKey(const string& apiKey) {

    DEEPL_API_KEY = apiKey;

}


//* Prüft ob DeepL aktiviert wurde
bool isDeepLEnabled() {

    return !DEEPL_API_KEY.empty();

}


//* Übersetzt einen Text nach Deutsch
string translateToGerman(const string& text)
{
    if (DEEPL_API_KEY.empty()) {
        cerr << "Fehler: Kein DeepL API-Schlüssel vorhanden.\n";
        return text;
    }

    CURL* curl = curl_easy_init();

    if (!curl) {
        cerr << "Fehler: CURL initialization failed.\n";
        return text;
    }

    try {
        //* Erstellt JSON Anfrage
        json requestBody = {
            {"text", {text}},
            {"target_lang", "DE"}
        };

        string postData = requestBody.dump();

        //* Antwort der API
        string response;

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

        string authHeader =
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
            cerr << "\n*** CURL Fehler ***\n";
            cerr << curl_easy_strerror(res) << "\n";
            return text;
        }

        if (httpCode != 200) {
            cerr << "\n*** API Fehler ***\n";
            cerr << "HTTP Status: " << httpCode << "\n";
            cerr << "Antwort: " << response << "\n";
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

        cerr << "Fehler: Ungültige API-Antwort.\n";
        return text;

    }
    catch (const exception& e) {
        cerr << "DeepL Anfrage Fehler: " << e.what() << "\n";

        curl_easy_cleanup(curl);

        return text;
    }
}


//* Wandelt EXIF-Key in lesbaren Text um
static string makeReadableExifName(
    const string& key
) {
    string name = key;

    //* Entfernt alles vor dem letzten Punkt
    size_t pos = name.rfind('.');

    if (pos != string::npos) {
        name = name.substr(pos + 1);
    }

    string result;

    //* Fügt Leerzeichen zwischen Klein- und Großbuchstaben ein
    for (size_t i = 0; i < name.size(); ++i) {

        char c = name[i];

        if (
            i > 0 &&
            isupper(static_cast<unsigned char>(c)) &&
            islower(static_cast<unsigned char>(name[i - 1]))
        ) {
            result += ' ';
        }

        result += c;
    }

    return result;
}


//* Übersetzt alle EXIF-Tags der aktuellen Datei mit DeepL
map<string, string> translateExifLabelsForData(
    const Exiv2::ExifData& exifData,
    const map<string, string>& exifToUser
) {
    map<string, string> labels;

    cout << "Übersetzung aller EXIF-Tags mit DeepL...\n";

    for (const auto& entry : exifData) {

        string key = entry.key();

        string readableName =
            makeReadableExifName(key);

        if (isDeepLEnabled()) {

            string readableName =
                makeReadableExifName(key);

            cout
                << "[DeepL] "
                 << readableName
                << " -> ";

            string translated =
                translateToGerman(readableName);

            cout << translated << "\n";

            labels[key] = translated;
        }
        else {
            if (exifToUser.find(key) != exifToUser.end()) {
                labels[key] = exifToUser.at(key);
            }
        }
    }

    cout << "Übersetzung aller EXIF-Tags abgeschlossen.\n";

    return labels;
}


//* Übersetzt EXIF-Werte
map<string, string> translateExifValues(
    const Exiv2::ExifData& exifData
) {
    map<string, string> translatedValues;

    cout << "Übersetzung der EXIF-Werte in Deutsch...\n";

    for (const auto& entry : exifData) {

        string key = entry.key();
        string value = entry.value().toString();

        if (value.empty()) {
            continue;
        }

        string translated =
            translateToGerman(value);

        if (translated != value) {
            translatedValues[key] = translated;
        }
    }

    cout << "EXIF-Werte Übersetzung abgeschlossen.\n";

    return translatedValues;
}


//* Testet die Verbindung zur DeepL API
bool testDeepLConnection() {

    if (DEEPL_API_KEY.empty()) {
        cout << "Fehler: DeepL API-Schlüssel ist nicht gesetzt.\n";
        return false;
    }

    string testPhrase = "Hello";

    string testResult =
        translateToGerman(testPhrase);

    if (
        !testResult.empty() &&
        testResult != testPhrase
    ) {
        cout << "DeepL Verbindung erfolgreich!\n";
        return true;
    }

    cout << "DeepL Verbindung fehlgeschlagen.\n";
    return false;
}
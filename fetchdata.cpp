#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;
using namespace std;

// Write curl data to string buffer
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    ((string*)stream)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

// Return pollution value from sensorID
float getSensorValue(int sensorID, string paramCode) {
    //Initiate curl
    CURL *curl;
    CURLcode result;
    string readBuffer;
    curl = curl_easy_init();

    //Handle HTTP request exception
    if (curl == nullptr) {
        fprintf(stderr, "HTTP request failed\n");
        return 0;
    }

    string queryURL = "https://api.gios.gov.pl/pjp-api/rest/data/getData/" + to_string(sensorID); // Create custom API URL
    curl_easy_setopt(curl, CURLOPT_URL, queryURL.c_str()); // Set URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set write function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);

    // Handle curl request exception
    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return 0;
    }

    // Try to parse JSON data
    try {
        json data = json::parse(readBuffer); //Parse
        if (data["key"] == paramCode) {
            if (data.contains("values") && data["values"].is_array() && !data["values"].empty()) {
                const json& latest = data["values"].front(); // Get latest value
                if (latest.contains("value") && latest["value"].is_number_float()) {
                    curl_easy_cleanup(curl); // Cleanup curl
                    return latest["value"].get<float>();
                }
            }
        }
        // Handle JSON exceptions
    }catch (const json::parse_error& e) {
        fprintf(stderr, "JSON parse error: %s\n", e.what());
    }catch (const std::exception& e) {
        fprintf(stderr, "Error: Unknown exception\n");
    }

    curl_easy_cleanup(curl);
    return 0;
}


string getStationData(int stationID) {
    //Initiate curl
    ostringstream oss;
    CURL *curl;
    CURLcode result;
    string readBuffer;
    curl = curl_easy_init();

    //Handle exception
    if (curl == nullptr) {
        fprintf(stderr, "HTTP request failed\n");
        return "";
    }

    string queryURL = "https://api.gios.gov.pl/pjp-api/rest/station/sensors/" + to_string(stationID); // Create custom API URL
    curl_easy_setopt(curl, CURLOPT_URL, queryURL.c_str()); // Set URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set write function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);
    // Handle HTTP request exception
    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return "";
    }
    // Try to parse JSON data
    try {
        json data = json::parse(readBuffer);
        ostringstream oss;
        for(const auto& sensor : data) {
            if (sensor["param"]["paramName"].is_string() && sensor["id"].is_number()) {
                curl_easy_cleanup(curl);
                // Print sensor name and values
                oss << sensor["param"]["paramName"].get<string>() << ": " << getSensorValue(sensor["id"].get<int>(), sensor["param"]["paramCode"].get<string>()) << endl;
            }

        }
        return oss.str();
        // Handle JSON exceptions
    }catch (const json::parse_error& e) {
        fprintf(stderr, "JSON parse error: %s\n", e.what());
    }catch (const std::exception& e) {
        fprintf(stderr, "Error: Unknown exception\n");
    }

    curl_easy_cleanup(curl);
    return "";
}


string getStationID(string city) {
    //Initiate curl
    ostringstream oss;
    CURL *curl;
    CURLcode result;
    string readBuffer;
    curl = curl_easy_init();

    //Handle exception
    if (curl == nullptr) {
        fprintf(stderr, "HTTP request failed\n");
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.gios.gov.pl/pjp-api/rest/station/findAll"); // Set URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set write function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);
    // Handle HTTP request exception
    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return "";
    }
    // Try to parse JSON data
    try {
        json data = json::parse(readBuffer);

        bool stationFound = false;
        // Search for stations in city
        for(const auto& station : data){
            if (station.contains("city") && station["city"].contains("name") && station["city"]["name"].is_string() && station["city"]["name"] == city) {
                if (station.contains("id") && station["id"].is_number()){
                    stationFound = true;
                    oss << station["stationName"].get<string>() << endl;
                    curl_easy_cleanup(curl);
                    oss << getStationData(station["id"].get<int>());
                }

            }
        }
        if (stationFound) {
            return oss.str();
        }
        fprintf(stderr, "Error: Station not found\n"); // Handle station not found
        return "Error: Station not found\n";
        // Handle JSON exceptions
    }catch (const json::parse_error& e) {
        fprintf(stderr, "JSON parse error: %s\n", e.what());
    }catch (const std::exception& e) {
        fprintf(stderr, "Error: Unknown exception\n");
    }

    curl_easy_cleanup(curl);
    return "";
}

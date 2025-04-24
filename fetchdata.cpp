#include "fetchdata.h"
#include "translate.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

/// @brief Macro inspired by gettext, simplifies translations immensly
#define _(phrase, lang) string(Translate((phrase), (lang)))

using json = nlohmann::json;
using namespace std;

///@defgroup fetchdata FetchData Helper Functions
/// Helper functions for methods in FetchData class

/**
 * @brief Helper function: Updates local json files with new data
 * @details If file no in path /data/[city].json, create it and save sensorData to it
 * else: function updates it with newer data
 * @param city Name of city. Necessary to create a [city].json file
 * @param sensorData Formatted json data fetched from GIOS API
 * @ingroup fetchdata
 */
void save_json_to_file(const string& city, const json& sensorData) {
    namespace fs = std::filesystem;

    // Create "data" directory if it doesn't exist
    fs::create_directory("data");

    // Prepare filename
    string filename = "data/" + city + ".json";

    json existingData;

    // Load existing data if file exists
    if (ifstream fileIn{filename}; fileIn.is_open()) {
        fileIn >> existingData;
    }

    // Go through sensorData and merge with existingData
    for (auto& [stationName, substances] : sensorData.items()) {
        for (auto& [substance, timestamps] : substances.items()) {
            for (auto& [timestamp, value] : timestamps.items()) {

                // If data point doesn't exist - add it
                if (!existingData[stationName][substance].contains(timestamp)) {
                    existingData[stationName][substance][timestamp] = value;
                }
            }
        }
    }
    // Sort data according to date
    for (auto& [stationName, substances] : existingData.items()) {
        for (auto& [substance, timestamps] : substances.items()) {
            // Sort timestamps
            std::map<string, json> sortedTimestamps;
            for (auto& [timestamp, value] : timestamps.items()) {
                sortedTimestamps[timestamp] = value;
            }
            // Overwrite data
            existingData[stationName][substance] = sortedTimestamps;
        }
    }

    // Save to file
    if (ofstream fileOut{filename}; fileOut.is_open()) {
        fileOut << std::setw(4) << existingData << std::endl;
    }
}

/**
 * @brief Helper function: write curl data to string buffer
 * @return write_data
 * @ingroup fetchdata
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    ((string*)stream)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

json FetchData::getSensorValue(const int& sensorID, const string& paramCode) const{
    //Initiate curl
    CURL *curl;
    CURLcode result;
    string readBuffer;
    curl = curl_easy_init();

    //Handle HTTP request exception
    if (curl == nullptr) {
        throw runtime_error(_("Błąd curl", lang_));
    }
    string queryURL = "https://api.gios.gov.pl/pjp-api/rest/data/getData/" + to_string(sensorID); // Create custom API URL
    curl_easy_setopt(curl, CURLOPT_URL, queryURL.c_str()); // Set URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set write function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);

    // Handle curl request exception
    if (result != CURLE_OK) {
        throw runtime_error(_("Dane sensora: ", lang_) + string(curl_easy_strerror(result)));
    }

    // Try to parse JSON data
    try {
        json data = json::parse(readBuffer); //Parse
        if (data["key"] == paramCode) {
            if (data.contains("values") && data["values"].is_array() && !data["values"].empty()) {
                int i = 0;
                json sensorData;
                for(auto& vals : data["values"]){
                    if(!vals["value"].is_null()){
                        sensorData[vals["date"]] = vals["value"];
                        i++;
                    }
                }
                return sensorData;
            }
        }else{
            throw runtime_error(_("Wystąpił problem z danymi GIOS", lang_));
        }
        // Handle JSON exceptions
    }catch (const json::parse_error& e) {
        throw runtime_error(e.what());
    }

    curl_easy_cleanup(curl);
    return "";
}

json FetchData::getStationData(int stationID) const{
    //Initiate curl
    CURL *curl;
    CURLcode result;
    string readBuffer;
    curl = curl_easy_init();

    //Handle curl exception
    if (curl == nullptr) {
        throw runtime_error(_("Błąd curl", lang_));
    }

    string queryURL = "https://api.gios.gov.pl/pjp-api/rest/station/sensors/" + to_string(stationID); // Create custom API URL
    curl_easy_setopt(curl, CURLOPT_URL, queryURL.c_str()); // Set URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set write function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);
    // Handle HTTP request exception
    if (result != CURLE_OK) {
        throw runtime_error(_("Dane stacji: ", lang_) + string((curl_easy_strerror(result))));
    }
    // Try to parse JSON data
    try {
        json data = json::parse(readBuffer);
        json stationData;
        for(const auto& sensor : data) {
            if (sensor["param"]["paramName"].is_string() && sensor["id"].is_number()) {
                curl_easy_cleanup(curl);
                // Print sensor name and values
                stationData[sensor["param"]["paramName"].get<string>()] = getSensorValue(sensor["id"].get<int>(), sensor["param"]["paramCode"].get<string>());
            }

        }
        return stationData;
        // Handle JSON exceptions
    }catch (const json::parse_error& e) {
        throw runtime_error(e.what());
    }
    curl_easy_cleanup(curl);
    return "";
}

FetchData::FetchData(const string& city, const string& lang) {
    //Initiate curl
    CURL *curl;
    CURLcode result;
    string readBuffer;
    curl = curl_easy_init();
    lang_ = lang;
    //Handle exception
    if (curl == nullptr) {
        throw runtime_error(_("Błąd curl", lang_));
    }
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.gios.gov.pl/pjp-api/rest/station/findAll"); // Set URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set write function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);
    // Handle HTTP request exception
    if (result != CURLE_OK) {
        throw runtime_error(_("Wszystkie Stacje: ", lang_) + string(curl_easy_strerror(result)));
    }
    // Try to parse JSON data
    try {
        json data = json::parse(readBuffer);
        json newData;
        bool stationFound = false;
        // Search for stations in city
        for(const auto& station : data){
            if (station.contains("city") && station["city"].contains("name") && station["city"]["name"].is_string() && station["city"]["name"] == city) {
                if (station.contains("id") && station["id"].is_number()){
                    stationFound = true;
                    newData[station["stationName"].get<string>()] = getStationData(station["id"].get<int>());
                    save_json_to_file(city, newData);
                }

            }
        }
        curl_easy_cleanup(curl);
        if (stationFound) {
            return;
        }
        throw runtime_error(_("Brak stacji pomiarowej w tym mieście", lang_));
        // Handle JSON exceptions
    }catch (const json::parse_error& e) {
        throw runtime_error(e.what());
    }catch (const std::exception& e) {
        throw runtime_error(e.what());
    }

    curl_easy_cleanup(curl);
}

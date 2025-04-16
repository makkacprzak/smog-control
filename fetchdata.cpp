#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;
using namespace std;

// Save sensor data to json database
void save_json_to_file(const string& city, const string& time, const json& sensorData){
    // Create data directory unless exists
    filesystem::create_directory("data");

    // Format city file
    string filename = "data/" + city + ".json";
    ifstream fileIn(filename);

    json cityData;

    // If city's file exists, load data
    if (fileIn.is_open()){
        fileIn >> cityData; // Load data into variable
        fileIn.close();
    }

    // Add new data to variable
    cityData = sensorData;

    ofstream fileOut(filename);

    // Save updated data to city's file
    if(fileOut.is_open()){
        fileOut << setw(4) << cityData << endl;
        fileOut.close();
    }
}

// Write curl data to string buffer
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    ((string*)stream)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

// Return pollution value from sensorID
json getSensorValue(int sensorID, string paramCode) {
    //Initiate curl
    CURL *curl;
    CURLcode result;
    string readBuffer;
    curl = curl_easy_init();

    //Handle HTTP request exception
    if (curl == nullptr) {
        fprintf(stderr, "HTTP request failed\n");
        return "";
    }

    string queryURL = "https://api.gios.gov.pl/pjp-api/rest/data/getData/" + to_string(sensorID); // Create custom API URL
    curl_easy_setopt(curl, CURLOPT_URL, queryURL.c_str()); // Set URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set write function
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    result = curl_easy_perform(curl);

    // Handle curl request exception
    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return "";
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
                        sensorData[i][vals["date"]] = vals["value"];
                        i++;
                    }
                    if(i == 48){
                        return sensorData;
                    }
                }
                return sensorData;
            }
        }
        // Handle JSON exceptions
    }catch (const json::parse_error& e) {
        fprintf(stderr, "JSON parse error: %s\n", e.what());
    }catch (const std::exception& e) {
        fprintf(stderr, "Error: Unknown exception\n");
    }

    curl_easy_cleanup(curl);
    return "";
}


json getStationData(int stationID) {
    //Initiate curl
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
        json newData;
        bool stationFound = false;
        // Search for stations in city
        for(const auto& station : data){
            if (station.contains("city") && station["city"].contains("name") && station["city"]["name"].is_string() && station["city"]["name"] == city) {
                if (station.contains("id") && station["id"].is_number()){
                    stationFound = true;
                    curl_easy_cleanup(curl);
                    newData[station["stationName"].get<string>()] = getStationData(station["id"].get<int>());
                    save_json_to_file(city, "latest", newData);
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

#ifndef FETCHDATA_H
#define FETCHDATA_H
#include <string>
#include <nlohmann/json.hpp>

/**
 * @brief The FetchData class exists to fetch relevant data, and save it in a local .json database
 * @details It is used only via the constructor, since the only data stored in a FetchData object itself is the selected language. It's aim twofold:\n
 * 1. To create the .json files, that are then read in other places in the code.\n
 * 2. To handle any errors that may arrise from a mismatch of data between different APIs, or network issues
 */
class FetchData{
private:
    ///@brief Stores language selected by the user, to be used
    std::string lang_;
    /**
     * @brief Helper function: fetch sensor data
     * @param sensorID ID of requested sensor
     * @param paramCode Parameter code. Useful to verify data from sensor API againt data from station API
     * @return json object of {date string, value double} pairs
     */
    nlohmann::json getSensorValue(const int& sensorID, const std::string& paramCode) const;
    /**
     * @brief Helper function: fetch data on all sensors in a given station
     * @details Fetch data from GIOS API (the relevant parts are IDs of all sensors in a given station and the parameters these sensors track).\n
     * Then using sensorIDs call getSensorData(), and return a neatly formatted parameter{measurments} json object.
     * @param stationID ID of a station
     * @return formated json data
     */
    nlohmann::json getStationData(int stationID) const;
public:
    /**
     * @brief FetchData constructor. Obtains a list of all stations, and starts the process leading to saving data to .json file
     * @param city City requested by user
     * @param lang Specified language. Necessary to be able to translate error messages
     */
    FetchData(const std::string& city, const std::string& lang = "pl");
};

#endif // FETCHDATA_H

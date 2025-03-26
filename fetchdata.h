#ifndef FETCHDATA_H
#define FETCHDATA_H
#include <string>
#include <curl/curl.h>

// Write curl data to string buffer
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

// Return pollution value from sensorID
float getSensorValue(int sensorID, std::string paramCode);

// Get sensors ID
std::string getStationData(int stationID);

// Get station ID from location
std::string getStationID(std::string city);

#endif // FETCHDATA_H

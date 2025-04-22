#include "jfile.h"
#include "fetchdata.h"
#include <fstream>
#include <algorithm>
#include <QDateTime>
#include <QString>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

using json = nlohmann::json;

Jfile::Jfile(const string& city){
    // Initiate API to .json sequence
    try{
        getStationID(city);
    }catch (const exception& e){
        errorMessage_ = e.what();
        HTTPError_ = true;
    }
    city_ = QString::fromStdString(city);
    // Read file at correct path
    string filename = "data/" + city + ".json";
    ifstream myFile(filename);

    // If file valid, save to file_
    if(myFile.is_open() ){
        myFile >> file_;
    }else{
        if(HTTPError_){
            throw runtime_error("Wystąpił błąd przy zapytaniu HTTP. Brak danych zapisanych lokalnie.\n" + errorMessage_);
        }
        throw runtime_error("Brak danych o tym mieście");
    }

    populateVectors();
}

void Jfile::populateVectors(){
    for(const auto& [station, parameters] : file_.items()){
        stations_.push_back(station);

        if(parameters.is_object()){
            for(const auto& [paramName, _] : parameters.items()){
                if (std::find(params_.begin(), params_.end(), paramName) == params_.end()) {
                    params_.push_back(paramName);
                }
            }
        }
    }

    return;
}

QString Jfile::getCity() const{
    if(!city_.isEmpty()){
        return city_;
    }else{
        throw runtime_error("Pole 'Miasto' jest puste");
    }
}

vector<string> Jfile::getStations() const{
    if(!stations_.empty()){
        return stations_;
    }else{
        throw runtime_error("Brak stacji w mieście");
    }
}

vector<string> Jfile::getParams() const{
    if(!params_.empty()){
        return params_;
    }else{
        throw runtime_error("Brak sensorów w mieście");
    }
}


vector<string> Jfile::getStationParams(const QString& station) const{
    vector<string> params;
    if(file_.contains(station) && !file_[station].empty()){
        for(const auto& [param, _] : file_[station].items()){
            params.push_back(param);
        }
    }else{
        throw runtime_error("Stacja nie istnieje bądź nie posiada sensorów");
    }

    return params;
}

// Helper function: convert string to tm
tm parseTimestamp(const std::string& timestamp) {
    tm tm{};
    stringstream ss(timestamp);
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return tm;
}

// Helper function: convert tm to string
string formatTimestamp(const std::tm& tm) {
    ostringstream ss;
    ss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Helper function: convert time_t to string
std::string convertTimeTToString(time_t timestamp) {
    std::tm* time_info = localtime(&timestamp);
    return formatTimestamp(*time_info);
}

// Function to single out only relevant entries
json Jfile::getDataSet(const string& station, const string& param) const{
    json dataSet;

    // Check if user wants average, or a certain station
    if (station == "Średnia wszystkich stacji" && param != ""){
        string timeRange[2];

        // Find the time range for the specified parameter
        for (const auto& [stationName, parameters] : file_.items()){
            if(parameters.contains(param)){

                // Single out first and last date
                string first = parameters[param].begin().key();
                string last = parameters[param].rbegin().key();

                if(timeRange[0] == "" && timeRange[1] == ""){ // If no entries, set first and last
                    timeRange[0] = first;
                    timeRange[1] = last;
                    continue;
                }
                if (first < timeRange[0]){
                    timeRange[0] = first;
                }
                if (last > timeRange[1]){
                    timeRange[1] = last;
                }

            }
        }

        // Convert date string to tm structure
        tm startTm = parseTimestamp(timeRange[0]);
        tm endTm = parseTimestamp(timeRange[1]);

        time_t startTime = mktime(&startTm);
        time_t endTime = mktime(&endTm);

        for (time_t t = startTime; t <= endTime; t += 3600) { //Iterate over entire time range with 1h increment
            vector<double> values;
            for (const auto& [stationName, parameters] : file_.items()) { // Iterate over each station
                if (parameters.contains(param)) {
                    string timeKey = convertTimeTToString(t);
                    if (parameters[param].contains(timeKey)) { // See if entry for this time exists
                        double value = parameters[param][timeKey].get<double>();
                        values.push_back(value);
                    }
                }
            }

            // Calculate average for this hour
            double average = 0;
            if (!values.empty()){
                average = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
            }
            // Add average to dataSet
            dataSet[convertTimeTToString(t)] = average;
        }
    }else if (station != "" && param != ""){ // If specific station requested
        if(!file_[station].contains(param)){ // Check if station has the parameter
            throw runtime_error("Brak danych dla wybranej stacji");
        }
        dataSet = file_[station][param];
    }

    return dataSet;
}

QVector<QPointF> Jfile::getDataPoints(const string& station, const string& param) const {
    QVector<QPointF> points;

    try{
        // Get data set for parameters
        const auto& measurments = getDataSet(station, param);

    if (!measurments.is_object()) {
        throw runtime_error("Błąd struktury bazy danych");
    }

    if (measurments.empty()) {
        throw runtime_error("Brak danych dla stacji");
    }

    // Iterate over entries
    for (const auto& entry : measurments.items()) {

        string time = entry.key(); // Single out time

        // If value valid, save to double
        if (!entry.value().is_number()) {
            fprintf(stderr, "Brak wartości liczbowej dla parametru\n");
            continue;
        }
        double y = entry.value().get<double>();

        // Crop date to "MM-dd HH:mm"
        QString ts = QString::fromStdString(time);
        QString shortened = ts.mid(5, 11);

        // Parse date into QDateTime format
        QDateTime dt = QDateTime::fromString(shortened, "MM-dd HH:mm");

        // If valid, convert to epoch time
        if (!dt.isValid()) {
            fprintf(stderr, "Incorrect date-time format\n");
            continue;
        }
        qint64 x = dt.toMSecsSinceEpoch();

        // Add point to vector
        points.append(QPointF(x, y));
    }
    }catch(const exception& e){
        throw runtime_error("Błąd w przetwarzaniu danych");
    }

    return points;
}

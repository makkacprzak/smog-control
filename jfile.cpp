#include "jfile.h"
#include "fetchdata.h"
#include "translate.h"
#include <fstream>
#include <algorithm>
#include <QDateTime>
#include <QString>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#define _(phrase, lang) string(Translate((phrase), (lang)))

using namespace std;

/// @defgroup JfileHelper Jfile Helper Functions
/// Helper Functions for Jfile methods

using json = nlohmann::json;

Jfile::Jfile(const string& city, const string& lang){
    HTTPError_ = false;
    // Initiate API to .json sequence
    try{
        FetchData(city, lang);
    }catch (const exception& e){
        errorMessage_ = e.what();
        HTTPError_ = true;
    }
    city_ = QString::fromStdString(city);
    lang_ = lang;
    // Read file at correct path
    string filename = "data/" + city + ".json";
    ifstream myFile(filename);

    // If file valid, save to file_
    if(myFile.is_open() ){
        myFile >> file_;
    }else{
        if(HTTPError_){
            throw runtime_error(_("Wystąpił błąd przy zapytaniu HTTP. Brak danych zapisanych lokalnie.\n", lang_) + errorMessage_);
        }
        throw runtime_error(_("Brak danych o tym mieście", lang_));
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
        throw runtime_error(_("Pole 'Miasto' jest puste", lang_));
    }
}

vector<string> Jfile::getStations() const{
    if(!stations_.empty()){
        return stations_;
    }else{
        throw runtime_error(_("Brak stacji w mieście", lang_));
    }
}

vector<string> Jfile::getParams() const{
    if(!params_.empty()){
        return params_;
    }else{
        throw runtime_error(_("Brak sensorów w mieście", lang_));
    }
}


vector<string> Jfile::getStationParams(const QString& station) const{
    vector<string> params;
    if(file_.contains(station) && !file_[station].empty()){
        for(const auto& [param, _] : file_[station].items()){
            params.push_back(param);
        }
    }else{
        throw runtime_error(_("Stacja nie istnieje bądź nie posiada sensorów", lang_));
    }

    return params;
}

/// @brief Helper function: convert string to tm format
/// @ingroup JfileHelper
tm parseTimestamp(const std::string& timestamp) {
    tm tm{};
    stringstream ss(timestamp);
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return tm;
}

/// @brief Helper function: convert tm format to string
/// @ingroup JfileHelper
string formatTimestamp(const std::tm& tm) {
    ostringstream ss;
    ss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

/// @brief Helper function: convert time_t format to string
/// @ingroup JfileHelper
std::string convertTimeTToString(time_t timestamp) {
    std::tm* time_info = localtime(&timestamp);
    return formatTimestamp(*time_info);
}

json Jfile::getDataSet(const string& station, const string& param, const json& file, const string& time) const{
    json dataSet;

    // Check if user wants average, or a certain station
    if (station == "Średnia wszystkich stacji" && param != ""){
        string timeRange[2];

        // Find the time range for the specified parameter
        for (const auto& [stationName, parameters] : file.items()){
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
        time_t endTime;
        if(time == "24h"){
            endTime = startTime + (24*60*60);
        }else if(time == "48h"){
            endTime = startTime + (48*60*60);
        }else if(time == "Max"){
            endTime = mktime(&endTm);
        }else{
            throw runtime_error(_("Niepoprawny wybrany czas", lang_));
        }

        for (time_t t = startTime; t <= endTime; t += 3600) { //Iterate over entire time range with 1h increment
            vector<double> values;
            for (const auto& [stationName, parameters] : file.items()) { // Iterate over each station
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
        if(!file[station].contains(param)){ // Check if station has the parameter
            throw runtime_error("Brak danych dla wybranej stacji");
        }
        int limit; // How many measurments to include in dataset
        if(time == "24h"){
            limit = 24;
        }else if(time == "48h"){
            limit = 48;
        }else if(time == "Max"){
            limit = file[station][param].size();
        }else{
            throw runtime_error(_("Niepoprawny wybrany czas", lang_));
        }

        // Sprawdzamy, czy file zawiera odpowiednią strukturę
        if (file.contains(station) && file[station].contains(param) && file[station][param].is_object()) {
            // Pobieramy obiekt "param", który jest obiektem
            auto param_object = file[station][param];

            int count = 0;
            for (auto& el : param_object.items()) {
                if (count <= limit) {
                    dataSet[el.key()] = el.value();  // Dodajemy element do wyniku
                    ++count;
                } else {
                    return dataSet;
                }
            }
        }
    }

    return dataSet;
}

QVector<QPointF> Jfile::getDataPoints(const string& station, const string& param, const string& time) const {
    QVector<QPointF> points;

    try{
        // Get data set for parameters
        const auto& measurments = getDataSet(station, param, file_, time);

        if (!measurments.is_object()) {
            throw runtime_error(_("Błąd struktury bazy danych", lang_));
        }

        if (measurments.empty()) {
            throw runtime_error(_("Brak danych dla stacji", lang_));
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
        throw runtime_error(_("Błąd w przetwarzaniu danych", lang_) + ":\n" + e.what());
    }

    return points;
}

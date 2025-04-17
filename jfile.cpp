#include "jfile.h"
#include "fetchdata.h"
#include <fstream>
#include <algorithm>
#include <QDateTime>
#include <QString>

using namespace std;

using json = nlohmann::json;

Jfile::Jfile(const string& city) {
    // Initiate API to .json sequence
    getStationID(city);

    // Read file at correct path
    string filename = "data/" + city + ".json";
    ifstream newFile(filename);

    // If file valid, save to file_
    if(newFile.is_open()){
        newFile >> file_;
    }else{
        fprintf(stderr, "No data on city\n");
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

vector<string> Jfile::getStations() const{
    return stations_;
}

vector<string> Jfile::getParams() const{
    return params_;
}

QVector<QPointF> Jfile::getDataPoints(const string& station, const string& param) const {
    QVector<QPointF> points;

    if (!file_.contains(station)) {
        fprintf(stderr, "Brak danych dla podanej stacji\n");
        return points;
    }

    if (!file_[station].contains(param)) {
        fprintf(stderr, "Brak danych dla podanego parametru\n");
        return points;
    }

    // Select only relevant part of json
    const auto& measurments = file_[station][param];

    if (!measurments.is_array()) {
        fprintf(stderr, "Dane nie są tablicą\n");
        return points;
    }

    // Iterate backwards to go from descending to ascending order
    for (auto i = measurments.rbegin(); i != measurments.rend(); ++i) {
        const auto& entry = *i;
        if (!entry.is_object() || entry.empty()) {
            continue;
        }

        const auto& pair = entry.begin(); // Select date-value pair
        string time = pair.key(); // Single out time

        // If value valid, save to double
        if (!pair.value().is_number()) {
            fprintf(stderr, "Brak wartości liczbowej dla parametru\n");
            continue;
        }
        double y = pair.value().get<double>();

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

    return points;
}

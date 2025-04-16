#include "jfile.h"
#include "fetchdata.h"
#include <fstream>
#include <algorithm>
#include <QDateTime>
#include <QString>

using namespace std;

using json = nlohmann::json;

Jfile::Jfile(const string& city) {
    getStationID(city);

    string filename = "data/" + city + ".json";
    ifstream newFile(filename);

    if(newFile.is_open()){
        newFile >> file_;
    }else{
        fprintf(stderr, "No data on city\n");
    }
    populateVectors();
}


bool Jfile::contains(const vector<string> vec, const string& str) const{
    return find(vec.begin(), vec.end(), str) != vec.end();
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
    for (const auto& item : stations_) {
        fprintf(stdout, "%s; ", item.c_str());
    }
    fprintf(stdout, "\n");

    for (const auto& item : params_) {
        fprintf(stdout, "%s; ", item.c_str());
    }
    fprintf(stdout, "\n");

    fflush(stdout);
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
        qDebug() << "Brak danych dla podanej stacji";
        return points;
    }

    if (!file_[station].contains(param)) {
        qDebug() << "Brak danych dla podanego parametru";
        return points;
    }

    const auto& measurments = file_[station][param];

    if (!measurments.is_array()) {
        qDebug() << "Dane nie są tablicą";
        return points;
    }

    qDebug() << "Przetwarzanie danych dla stacji: " << QString::fromStdString(station);

    for (auto i = measurments.rbegin(); i != measurments.rend(); ++i) {
        const auto& entry = *i;
        if (!entry.is_object() || entry.empty()) {
            continue;
        }

        const auto& pair = entry.begin();
        string time = pair.key();
        qDebug() << "Czas: " << QString::fromStdString(time);

        if (!pair.value().is_number()) {
            qDebug() << "Brak wartości liczbowej dla parametru";
            continue;
        }

        double y = pair.value().get<double>();

        // Przycięcie daty do formatu "MM-dd HH:mm"
        QString ts = QString::fromStdString(time);
        QString shortened = ts.mid(5, 11);  // Przycinamy do "MM-dd HH:mm"
        qDebug() << "Przycięty czas: " << shortened;

        // Parsowanie daty
        QDateTime dt = QDateTime::fromString(shortened, "MM-dd HH:mm");

        if (!dt.isValid()) {
            qDebug() << "Błędny format daty: " << shortened;
            continue;
        }

        // Przechodzimy do timestampu
        qint64 x = dt.toMSecsSinceEpoch();

        qDebug() << "Dodano punkt: (" << x << ", " << y << ")";

        // Dodanie punktu do wektora
        points.append(QPointF(x, y));
    }

    qDebug() << "Zakończono przetwarzanie. Liczba punktów: " << points.size();

    return points;
}

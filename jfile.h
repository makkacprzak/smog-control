#ifndef JFILE_H
#define JFILE_H
#include <nlohmann/json.hpp>
#include <vector>
#include <QString>
#include <QVector>
#include <QPointF>

class Jfile
{
private:
    nlohmann::json file_;
    std::vector<std::string> params_;
    std::vector<std::string> stations_;
    QString city_;
public:
    Jfile(const std::string& city);
    void populateVectors();
    QString getCity() const;
    std::vector<std::string> getStations() const;
    std::vector<std::string> getParams() const;
    bool isNew(const QString& station) const;
    std::vector<std::string> getStationParams(const QString& station) const;
    nlohmann::json getDataSet(const std::string& station, const std::string& param) const;
    QVector <QPointF> getDataPoints(const std::string& station, const std::string& param) const;
};

#endif // JFILE_H

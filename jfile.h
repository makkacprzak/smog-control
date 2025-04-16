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
public:
    Jfile(const std::string& city);
    bool contains(const std::vector<std::string> vec,const std::string& str) const;
    void populateVectors();
    std::vector<std::string> getStations() const;
    std::vector<std::string> getParams() const;
    QVector <QPointF> getDataPoints(const std::string& station, const std::string& param) const;
};

#endif // JFILE_H

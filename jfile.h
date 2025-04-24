#ifndef JFILE_H
#define JFILE_H
#include <nlohmann/json.hpp>
#include <vector>
#include <QString>
#include <QVector>
#include <QPointF>

/**
 * @brief The Jfile class stores and processes .json data created by the FetchData class
 */
class Jfile
{
private:
    /// @brief Stores the entirety of the .json file
    nlohmann::json file_;
    /// @brief Stores all unique air parameters in city
    std::vector<std::string> params_;
    /// @brief Stores all stations in city
    std::vector<std::string> stations_;
    /// @brief Stores the name of the city in question
    /// @details Only used to make chart description more readable
    QString city_;
public:
    std::string lang_;
    /// @brief True if HTTPError_ occured during request
    bool HTTPError_;
    /// @brief Stores curl's HTTP request error message
    std::string errorMessage_;
    // These args are here to simplify transfering errors
    /**
     * @brief Jfile class constructor
     * @param city City requested by user
     */
    Jfile(const std::string& city, const std::string& lang);
    /// @brief Populates {params_, stations_} vectors with data from .json
    void populateVectors();
    /// @brief Access function to the city_ attribute
    /// @return city_ private attribute
    QString getCity() const;
    /// @brief Access function to the stations_ attribute
    /// @return stations_ private attribute
    std::vector<std::string> getStations() const;
    /// @brief Access function to the params_ attribute
    /// @return params_ private attribute
    std::vector<std::string> getParams() const;
    /// @brief Get all parameters tracked by station
    /// @param station Exact name of station requested by user
    /// @return A vector of air parameters tracked by station
    std::vector<std::string> getStationParams(const QString& station) const;
    /**
     * @brief Single out only relevant data from .json file
     * @details Function checks whether the user has requested data from a single station, or the average of all stations in a given city.\n
     * If average is selected, then function checks every station that tracks the selected air parameter, averages all the measurments, and returns them in a single neat json object.\n
     * The function is created in such a way, that even if some stations have holes in their data, e.g. a few hours of downtime, or a delay in uploading the latest measurments to the API,
     * it will still use every single measurment that has been conducted in the city.
     * @param station Exact name of station requested by user, or string representing average values have been requested
     * @param param Exact name of air parameter requested by user
     * @param file Reference to json file being processed
     * @param time Time span selected by user
     * @return Json data that contains only a single object of {date string, value double number} pair
    */
    nlohmann::json getDataSet(const std::string& station, const std::string& param, const nlohmann::json& file, const std::string& time) const;
    /**
     * @brief Get data displayable on QChartView for specific station-param pair
     * @details First separates only relevant parts of file_ attribute\n
     * Then parses this data into data a format that is easily interpreted by the QChartView UI element
     * @param station Exact name of station requested by user, or string that represents the average of all measurments in city
     * @param param Exact name of air parameter requested by user
     * @param time Selects the timespan to show user
     * @return A QVector of QPointFs, which are {date in qint64 ms since epoch format, value in double format} pairs sorted in ascending order
     */
    QVector <QPointF> getDataPoints(const std::string& station, const std::string& param, const std::string& time) const;
};

#endif // JFILE_H

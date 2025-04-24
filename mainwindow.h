#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "jfile.h"
#include <QtCharts/QChartView>


QT_BEGIN_NAMESPACE
namespace Ui {
class mainwindow;
}
QT_END_NAMESPACE

/**
 * @brief The mainwindow class is the middleman between the code and the QT-based GUI.
 * @details This class interacts directly with the QT-based GUI, either by sending or recieving data to and from different GUI segments, or by detecting different actions taking place on screen.
 */
class mainwindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief Constructor creates the initial look of the GUI, and creates necessary slot connections
     */
    mainwindow(QWidget *parent = nullptr);
    ~mainwindow();
    ///@brief Reads user input from the text box
    std::string getTextBox() const;
    ///@brief Populates the drop-down menu containing stations the user can choose from
    void populateStationComboBox();
    ///@brief Populates the drop-down menu containing parameters the user can choose from
    void populateParameterComboBox(const QString& station);
    ///@brief Helper Function: Populates the drop-down menu containing available languages
    void populateLangSelect(const std::string& currentLang);
    ///@brief Makes all the necessary changes in the GUI to switch languages
    void changeLang();
    /**
     * @brief Displays requested data on a graph type specified by user
     * @param points A QVector of pre-processed points ready to be ploted onto a graph
     * @param station The station user requested. Used for preparing the graph's title
     * @param param The parameter user requested. Used for preparing the graph's title
     */
    void displayChart(const QVector<QPointF> &points, const QString& station, const QString& param);
    /**
    * @brief Checks if either of the drop-down menus were interacted with, and calls the necessary functions
    * @details To give the user more flexibility, the function gets called every time either the parameter menu or the station menu are interacted with.
    * The function then makes a decision on which functions to call.
    * CRUCIAL!!! Because chartView -> chart() uses a parent-child structure,
    * clearing memory reserved for a QChart object also clears any QSeries or QSet object used in it.
    * Because of some QCharts tomfoolery, since QT6 QChartView -> charts() cannot be nullptr
    * I have sort of solved that by adding if(oldChart) delete oldChart; after each chartView->setChart(chart); assignment.
    * In theory this should be memory safe. However as we know, this is C++.
    */
    void checkAndDrawChart();
public slots:
    ///@brief Slot get's called when the user clicks the "search" button
    void on_searchBtn_clicked();
private:
    Ui::mainwindow *ui;
    ///@brief To smoothen the interaction between classes, a pointer to the currently used Jfile is added
    Jfile* myFile_ = nullptr;
    QChartView* chartView;
    ///@brief Stores the language the user selected for use in selecting the proper translation the user sees
    std::string lang_;
};


#endif // MAINWINDOW_H
